/**
 * This file is part of SDLTerminal.
 * Copyright (C) 2011 Vincent Ho <www.whimsicalvee.com>
 * Copyright (C) 2011-2012 Ryan Hope <rmh3093@gmail.com>
 *
 * SDLTerminal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SDLTerminal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with SDLTerminal.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits>

#include "seqparser.hpp"

const unsigned char ControlSeqParser::ESC_CHAR = 27;
const unsigned char ControlSeqParser::DELIMITER_CHAR = ';';

ControlSeqParser::ControlSeqParser()
: m_state(ST_START), m_seq(NULL), m_mode(MODE_8BIT), m_utf8_remlen(0)
{
	buildLookup();
}

ControlSeqParser::~ControlSeqParser()
{
}

void ControlSeqParser::addFixedLookup(const char *str, CSToken_t token)
{
	CS_Fixed_Entry e;
	unsigned int slen = strlen(str);
	if (slen == 0) return; /* invalid */
	if (slen + 1 > sizeof(e.fixed)) return; /* too long */

	memset(&e, 0, sizeof(e));
	memcpy(e.fixed, str, slen);
	e.token = token;

	CS_Fixed_Lookup::iterator it;
	it = m_csFixedLookup.find(e.fixed[0]);
	if (it == m_csFixedLookup.end()) {
		it = m_csFixedLookup.insert(std::make_pair(e.fixed[0], std::list<CS_Fixed_Entry>())).first;
	}
	it->second.push_back(e);
}

void ControlSeqParser::addCSILookup(const char parameter, CSToken_t token, int nMinParam, int nMaxParam, int nDefaultVal, char cFinal)
{
	CSI_Entry e;
	e.token = token;
	e.parameter = parameter;
	e.function = cFinal;
	e.minParams = nMinParam;
	e.maxParams = nMaxParam;
	e.defaultVal = nDefaultVal;

	CSI_Lookup::iterator it;
	it = m_csiLookup.find(e.function);
	if (it == m_csiLookup.end()) {
		it = m_csiLookup.insert(std::make_pair(e.function, std::list<CSI_Entry>())).first;
	}
	it->second.push_back(e);
}

/**
 * Builds the lookup table to facilitate parsing.
 */
void ControlSeqParser::buildLookup()
{
	addCSILookup(0, CS_CURSOR_POSITION_REPORT, 0, 2, 1, 'R');
	addCSILookup(0, CS_CURSOR_POSITION, 0, 2, 1, 'H');
	addCSILookup(0, CS_CURSOR_POSITION, 0, 2, 1, 'f');
	addCSILookup(0, CS_CURSOR_UP, 0, 1, 1, 'A');
	addCSILookup(0, CS_CURSOR_DOWN, 0, 1, 1, 'B');
	addCSILookup(0, CS_CURSOR_FORWARD, 0, 1, 1, 'C');
	addCSILookup(0, CS_CURSOR_BACKWARD, 0, 1, 1, 'D');
	addCSILookup(0, CS_CURSOR_POSITION_SAVE, 0, 0, 1, 's');
	addFixedLookup("7", CS_CURSOR_POSITION_SAVE);
	addCSILookup(0, CS_CURSOR_POSITION_RESTORE, 0, 0, 1, 'u');
	addFixedLookup("8", CS_CURSOR_POSITION_RESTORE);
	addCSILookup(0, CS_ERASE_DISPLAY, 0, -1, 1, 'J');
	addCSILookup(0, CS_ERASE_LINE, 0, -1, 1, 'K');
	addCSILookup(0, CS_GRAPHICS_MODE_SET, 0, -1, 1, 'm');

	addCSILookup(0, CS_MODE_SET, 0, -1, 1, 'h');
	addCSILookup(0, CS_MODE_RESET, 0, -1, 1, 'l');

	addCSILookup('?', CS_MODE_SET_PRIV, 0, -1, 1, 'h');
	addCSILookup('?', CS_MODE_RESET_PRIV, 0, -1, 1, 'l');

	addFixedLookup("=", CS_KEYPAD_APP_MODE);
	addFixedLookup(">", CS_KEYPAD_NUM_MODE);

	addFixedLookup("D", CS_INDEX);
	addFixedLookup("M", CS_REVERSE_INDEX);
	addFixedLookup("H", CS_TAB_SET);
	addCSILookup(0, CS_TAB_CLEAR, 0, 1, 0, 'g');
	addCSILookup(0, CS_TAB_FORWARD, 0, 1, 1, 'I');

	addFixedLookup("(K", CS_USER_MAPPING);

	addFixedLookup("(A", CS_CHARSET_UK_G0_SET);
	addFixedLookup("(B", CS_CHARSET_US_G0_SET);
	addFixedLookup("(0", CS_CHARSET_SPEC_G0_SET);
	addFixedLookup("(1", CS_CHARSET_ALT_G0_SET);
	addFixedLookup("(2", CS_CHARSET_ALT_SPEC_G0_SET);
	addFixedLookup(")A", CS_CHARSET_UK_G1_SET);
	addFixedLookup(")B", CS_CHARSET_US_G1_SET);
	addFixedLookup(")0", CS_CHARSET_SPEC_G1_SET);
	addFixedLookup(")1", CS_CHARSET_ALT_G1_SET);
	addFixedLookup(")2", CS_CHARSET_ALT_SPEC_G1_SET);

	addCSILookup(0, CS_VPA, 0, 1, 1, 'd');
	addCSILookup(0, CS_CHA, 0, 1, 1, 'G');
	addCSILookup(0, CS_ECH, 0, 1, 1, 'X');
	addCSILookup(0, CS_IL, 0, 1, 1, 'L');
	addCSILookup(0, CS_DL, 0, 1, 1, 'M');
	addCSILookup(0, CS_DCH, 0, 1, 1, 'P');
	addCSILookup(0, CS_ICH, 0, 1, 1, '@');
	addCSILookup(0, CS_HPA, 0, 1, 1, '`');
	addCSILookup(0, CS_CBT, 0, 1, 1, 'Z');
	addCSILookup(0, CS_CNL, 0, 1, 1, 'E');
	addCSILookup(0, CS_CPL, 0, 1, 1, 'F');

	addCSILookup(0, CS_MARGIN_SET, 0, 2, 1, 'r');
	addFixedLookup("D", CS_MOVE_UP);
	addFixedLookup("M", CS_MOVE_DOWN);
	addFixedLookup("E", CS_MOVE_NEXT_LINE);

	addFixedLookup("#3", CS_DOUBLE_HEIGHT_LINE_TOP);
	addFixedLookup("#4", CS_DOUBLE_HEIGHT_LINE_BOTTOM);
	addFixedLookup("#5", CS_SINGLE_WIDTH_LINE);
	addFixedLookup("#6", CS_DOUBLE_WIDTH_LINE);
	addFixedLookup("#8", CS_SCREEN_ALIGNMENT_DISPLAY);

	addCSILookup(0, CS_DEVICE_STATUS_REPORT, 0, -1, 1, 'n');

	addCSILookup(0, CS_DEVICE_ATTR_PRIMARY_REQUEST, 0, 1, 0, 'c');
	addCSILookup('?', CS_DEVICE_ATTR_PRIMARY_RESPONSE, 0, -1, 0, 'c');

	addCSILookup('>', CS_DEVICE_ATTR_SECONDARY_REQUEST, 0, 1, 0, 'c');
	addCSILookup('>', CS_DEVICE_ATTR_SECONDARY_RESPONSE, 0, -1, 0, 'c');

	addCSILookup(0, CS_TERM_PARAM, 0, 1, 0, 'x');
	addFixedLookup("c", CS_TERM_RESET);

	/* VT52 Compat */
	addFixedLookup("<", CS_VT52_ANSI_MODE);
	addFixedLookup("A", CS_VT52_CURSOR_UP);
	addFixedLookup("B", CS_VT52_CURSOR_DOWN);
	addFixedLookup("C", CS_VT52_CURSOR_RIGHT);
	addFixedLookup("D", CS_VT52_CURSOR_LEFT);
	addFixedLookup("J", CS_VT52_ERASE_SCREEN);
	addFixedLookup("K", CS_VT52_ERASE_LINE);
	addFixedLookup("H", CS_VT52_CURSOR_HOME);
	addFixedLookup("I", CS_VT52_REVERSE_LINE_FEED);
	addFixedLookup("Z", CS_VT52_IDENTIFY);
	/* ESC Y -> CS_VT52_CURSOR_POSITION is done manually in the parser */
}

bool ControlSeqParser::tryFixedEscape() {
	CS_Fixed_Lookup::iterator it;
	std::list<CS_Fixed_Entry>::const_iterator i, end;

	it = m_csFixedLookup.find(m_prefix[0]);
	if (it == m_csFixedLookup.end()) {
		/* invalid sequence (prefix) */
		m_state = ST_START;
		return true;
	}

	i = it->second.begin();
	end = it->second.end();

	while (i != end) {
		if (0 == memcmp(i->fixed, m_prefix, m_prefixlen)) {
			/* prefix is good */
			if (i->fixed[m_prefixlen] == '\0') {
				/* sequence complete */
				m_token = i->token;
				m_state = ST_START;
				return true;
			} else {
				/* need more */
				return false;
			}
		}
		++i;
	}

	/* invalid sequence (prefix) */
	m_state = ST_START;
	return true;
}

void ControlSeqParser::parseCSIValue() {
	if (0 == m_numValues) {
		m_values[m_numValues++] = -1;
	}

	if (m_currentChar == 0x3B) { /* ';' */
		if (m_numValues >= MAX_NUM_VALUES) {
			m_state = ST_CSI_INVALID;
			return;
		}
		m_values[m_numValues++] = -1;
	} else {
		/* we have a digit */
		if (m_values[m_numValues-1] == -1) {
			/* not a default value as we have a digit */
			m_values[m_numValues-1] = 0;
		}
		int digit = m_currentChar - 0x30;
		int val = m_values[m_numValues-1];
		if ((std::numeric_limits<int>::max() - digit)/10 < val) {
			/* overflow */
			m_state = ST_CSI_INVALID;
			return;
		}
		m_values[m_numValues-1] = 10*val + digit;
	}
}

bool ControlSeqParser::matchCSI() {
	CSI_Lookup::iterator it;
	std::list<CSI_Entry>::const_iterator i, end;

	it = m_csiLookup.find(m_currentChar);
	if (it == m_csiLookup.end()) {
		/* invalid csi function, skip */
		m_state = ST_START;
		return false;
	}

	i = it->second.begin();
	end = it->second.end();

	/* empty param string should represent a default value. we ignore this here. */
	/* if (m_numValues == 0) { m_values[m_numValues++] = -1; } */

	for (; i != end; ++i) {
		if (i->parameter == (m_prefixlen ? m_prefix[0] : 0)) {
			/* found function */

			/* invalid numberof parameters */
			if (m_numValues < i->minParams || (-1 != i->maxParams && m_numValues > i->maxParams)) continue;

			unsigned int k;
			for (k = 0; k < m_numValues; k++) {
				if (-1 == m_values[k]) {
					if (-1 != i->defaultVal) {
						m_values[k] = i->defaultVal;
					} else {
						/* no value specified for a param, but neither is a default value available */
						m_state = ST_START;
						return false;
					}
				}
			}
			for (; k < MAX_NUM_VALUES; k++) {
				/* reset all other values */
				m_values[k] = -1;
			}

			m_token = i->token;
			m_state = ST_START;
			return true;
		}
	}

	/* invalid csi function, skip */
	m_state = ST_START;
	return false;
}

bool ControlSeqParser::parseChar() {
	switch (m_currentChar) {
	case 0x08: // backspace
	case 0x09: // \t
	case 0x0A: // \n
	case 0x0B: // vertical tab
	case 0x0D: // \r
		/* return control character and resume parsing afterwards */
		return true;
	}
	switch (m_state) {
	case ST_START:
		if (ESC_CHAR == m_currentChar) {
			m_state = ST_ESCAPE;
			m_prefixlen = 0;
			m_numValues = 0;
			return false;
		}
		if (MODE_7BIT != m_mode && m_currentChar >= 0x80 && m_currentChar < 0xA0) {
			m_state = ST_ESCAPE;
			m_prefixlen = 0;
			m_numValues = 0;
			m_currentChar = m_currentChar - 0x40;
			return parseChar();
		}
		return true;
	case ST_ESCAPE:
		if (m_currentChar >= 0x100) {
			/* cancel sequence, ignore previous part */
			m_state = ST_START;
			return true;
		} else if ('[' == m_currentChar) {
			m_state = ST_CSI;
			return false;
		} else if (']' == m_currentChar) {
			m_state = ST_OSC;
			return false;
		} else if ('Y' == m_currentChar) {
			m_state = ST_ESCY;
			return false;
		} else {
			m_state = ST_ESCAPE_TRIE;
			return parseChar();
		}
		break;
	case ST_CSI:
		if (m_currentChar >= 0x3C && m_currentChar <= 0x3F) {
			/* "private" extension */
			if (m_prefixlen >= 1) {
				m_state = ST_CSI_INVALID;
			} else {
				m_prefix[m_prefixlen++] = m_currentChar;
			}
		} else if ((m_currentChar >= 0x30 && m_currentChar <= 0x39) || 0x3B == m_currentChar) {
			/* digit */
			m_state = ST_CSI_VALUES;
			parseCSIValue();
			return false;
		} else if (m_currentChar >= 0x20 && m_currentChar <= 0x2F) {
			/* intermediate bytes */
			m_state = ST_CSI_INVALID;
		} else if (m_currentChar >= 0x40 && m_currentChar <= 0x7E) {
			/* final char */
			return matchCSI();
		} else {
			m_state = ST_CSI_INVALID;
		}
		return false;
	case ST_CSI_VALUES:
		if ((m_currentChar >= 0x30 && m_currentChar <= 0x39) || 0x3B == m_currentChar) {
			/* digit */
			parseCSIValue();
		} else if (m_currentChar >= 0x20 && m_currentChar <= 0x2F) {
			/* intermediate bytes */
			m_state = ST_CSI_INVALID;
		} else if (m_currentChar >= 0x40 && m_currentChar <= 0x7E) {
			/* final char */
			return matchCSI();
		} else {
			m_state = ST_CSI_INVALID;
		}
		return false;
	case ST_CSI_INVALID:
		if (m_currentChar >= 0x40 && m_currentChar <= 0x7E) {
			/* final char */
			m_state = ST_START;
		}
		return false;
	case ST_OSC:
		if (m_currentChar == 0x98 || m_currentChar == 0x9C || m_currentChar == 0x07) {
			m_state = ST_START;
		} else if (ESC_CHAR == m_currentChar) {
			m_state = ST_OSC_ESC;
		}
		return false;
	case ST_OSC_ESC:
		if (m_currentChar == 0x98 || m_currentChar == 0x9C || m_currentChar == 0x07 || m_currentChar == 'X' || m_currentChar == '\\') {
			m_state = ST_START;
		} else {
			m_state = ST_OSC;
		}
		return false;
	case ST_ESCAPE_TRIE:
		if (m_currentChar >= 0x100) {
			/* cancel sequence, ignore previous part */
			m_state = ST_START;
			return true;
		}
		m_prefix[m_prefixlen++] = m_currentChar;
		return tryFixedEscape();
	case ST_ESCY:
		if (m_currentChar < 0x20) {
			/* cancel sequence, ignore previous part */
			m_state = ST_START;
			return true;
		}
		m_values[m_numValues++] = m_currentChar - 0x19;
		if (2 == m_numValues) {
			m_state = ST_START;
			m_token = CS_VT52_CURSOR_POSITION;
			return true;
		}
	default:
		m_state = ST_START;
		return true;
	}
}

void ControlSeqParser::addInput(const char *seq, int len) {
	/* assert(NULL == m_seq); */
	m_seq = (const unsigned char*) seq;
	m_len = len;
}

unsigned char ControlSeqParser::nextByte() {
	unsigned char c;
	if (!m_seq) return 0;

	do {
		if (0 == m_len) {
			/* end of input for now */
			m_seq = NULL;
			return 0;
		}
		c = *m_seq++;
		--m_len;
	} while (!c);

	return c;
}

bool ControlSeqParser::nextChar() {
	unsigned char c;
	while (0 != (c = nextByte())) {
		switch (m_mode) {
		case MODE_UTF8:
			if (0 == m_utf8_remlen) {
				m_currentChar = 0;
				/* new char */
				if (0 == (c & 0x80)) { m_utf8_seqlen = 1; m_currentChar = c & 0x7f; }
				else if (0xC0 == (c & 0xFE)) { return false; /* 0xCO / 0xC1 overlong */ }
				else if (0xC0 == (c & 0xE0)) { m_utf8_seqlen = 2; m_currentChar = c & 0x1f; }
				else if (0xE0 == (c & 0xF0)) { m_utf8_seqlen = 3; m_currentChar = c & 0x0f; }
				/* only 16-bit, seqlen 4 not supported */
				/* else if (0xF0 == (c & 0xF8)) { m_utf8_seqlen = 4; code = c & 0x07; } */
				else return false; /* skip invalid byte */
				m_utf8_remlen = m_utf8_seqlen - 1;
			} else {
				if (0x80 != (c & 0xC0)) {
					m_utf8_remlen = 0; /* skip invalid bytes */
					return false;
				}
				m_currentChar = (m_currentChar << 6) | (c & 0x3f);
				--m_utf8_remlen;
			}
			if (0 == m_utf8_remlen) {
				/* char complete */
				if ((3 == m_utf8_seqlen) && (m_currentChar < 0x800)) return false; /* overlong */
				return true;
			}
			return false;
		case MODE_7BIT:
		case MODE_8BIT:
		default:
			m_currentChar = c;
			return true;
		}
	}
	return false;
}

bool ControlSeqParser::next() {
	if (!m_seq) return false;
	m_token = CS_UNKNOWN;

	while (nextChar()) {
		if (parseChar()) return true;
	}
	return false;
}

void ControlSeqParser::setMode(Mode mode) {
	if (mode != m_mode) {
		m_mode = mode;
		m_utf8_remlen = 0; /* reset utf-8 state */
	}
}
