/**
 * This file is part of wTerm.
 * Copyright (C) 2011 Vincent Ho <www.whimsicalvee.com>
 * Copyright (C) 2011-2012 Ryan Hope <rmh3093@gmail.com>
 *
 * wTerm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * wTerm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with wTerm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <limits>

#include "seqparser.hpp"
#include "util/utf8.hpp"

const unsigned char ControlSeqParser::ESC_CHAR = 27;
const unsigned char ControlSeqParser::DELIMITER_CHAR = ';';

ControlSeqParser::ControlSeqParser()
: m_state(ST_START), m_seq(NULL), m_mode(MODE_UTF8), m_utf8_remlen(0), m_vt52(false)
{
	buildLookup();
}

ControlSeqParser::~ControlSeqParser()
{
}

void ControlSeqParser::addFixedLookup(const char *str, CSToken token)
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

	for (std::list<CS_Fixed_Entry>::const_iterator li = it->second.begin(), le = it->second.end(); li != le; ++li) {
		unsigned int i = 0;
		while (0 != e.fixed[i] && e.fixed[i] == li->fixed[i]) ++i;
		if (0 == e.fixed[i] || 0 == li->fixed[i]) {
			syslog(LOG_DEBUG, "conflicting ESC sequences: ESC%s and ESC%s", e.fixed, li->fixed);
		}
	}

	it->second.push_back(e);
}

void ControlSeqParser::addVT52FixedLookup(const char *str, CSToken token) {
	CS_Fixed_Entry e;
	unsigned int slen = strlen(str);
	if (slen == 0) return; /* invalid */
	if (slen + 1 > sizeof(e.fixed)) return; /* too long */

	memset(&e, 0, sizeof(e));
	memcpy(e.fixed, str, slen);
	e.token = token;

	CS_Fixed_Lookup::iterator it;
	it = m_csVT52Lookup.find(e.fixed[0]);
	if (it == m_csVT52Lookup.end()) {
		it = m_csVT52Lookup.insert(std::make_pair(e.fixed[0], std::list<CS_Fixed_Entry>())).first;
	}

	for (std::list<CS_Fixed_Entry>::const_iterator li = it->second.begin(), le = it->second.end(); li != le; ++li) {
		unsigned int i = 0;
		while (0 != e.fixed[i] && e.fixed[i] == li->fixed[i]) ++i;
		if (0 == e.fixed[i] || 0 == li->fixed[i]) {
			syslog(LOG_DEBUG, "conflicting ESC sequences: ESC%s and ESC%s", e.fixed, li->fixed);
		}
	}

	it->second.push_back(e);
}

void ControlSeqParser::addCSILookup(const char parameter, CSToken token, int nMinParam, int nMaxParam, int nDefaultVal, char cFinal)
{
	CSI_Entry e;
	e.token = token;
	e.parameter = parameter;
	e.function = cFinal;
	e.suffix = 0;
	e.minParams = nMinParam;
	e.maxParams = nMaxParam;
	e.defaultVal = nDefaultVal;

	CSI_Lookup::iterator it;
	it = m_csiLookup.find(e.function);
	if (it == m_csiLookup.end()) {
		it = m_csiLookup.insert(std::make_pair(e.function, std::list<CSI_Entry>())).first;
	}

	for (std::list<CSI_Entry>::const_iterator li = it->second.begin(), le = it->second.end(); li != le; ++li) {
		if (li->parameter == e.parameter && li->function == e.function && li->suffix == e.suffix) {
			const char buf[2] = { e.parameter, 0 };
			if (e.suffix)
				syslog(LOG_DEBUG, "conflicting CSI functions: ESC[%s...%c%c", buf, e.suffix, e.function);
			else
				syslog(LOG_DEBUG, "conflicting CSI functions: ESC[%s...%c", buf, e.function);
		}
	}

	it->second.push_back(e);
}

void ControlSeqParser::addCSI2Lookup(const char parameter, CSToken token, int nMinParam, int nMaxParam, int nDefaultVal, char cSuffix, char cFinal)
{
	CSI_Entry e;
	e.token = token;
	e.parameter = parameter;
	e.suffix = cSuffix;
	e.function = cFinal;
	e.minParams = nMinParam;
	e.maxParams = nMaxParam;
	e.defaultVal = nDefaultVal;

	CSI_Lookup::iterator it;
	it = m_csiLookup.find(e.function);
	if (it == m_csiLookup.end()) {
		it = m_csiLookup.insert(std::make_pair(e.function, std::list<CSI_Entry>())).first;
	}

	for (std::list<CSI_Entry>::const_iterator li = it->second.begin(), le = it->second.end(); li != le; ++li) {
		if (li->parameter == e.parameter && li->function == e.function && li->suffix == e.suffix) {
			const char buf[2] = { e.parameter, 0 };
			if (e.suffix)
				syslog(LOG_DEBUG, "conflicting CSI functions: ESC[%s...%c%c", buf, e.suffix, e.function);
			else
				syslog(LOG_DEBUG, "conflicting CSI functions: ESC[%s...%c", buf, e.function);
		}
	}

	it->second.push_back(e);
}

/**
 * Builds the lookup table to facilitate parsing.
 */
void ControlSeqParser::buildLookup()
{
	addCSI2Lookup(0, CS_CURSOR_STYLE, 0, 1, 0, ' ', 'q');

	addCSI2Lookup(0, CS_INSERT_COLUMN, 0, -1, 1, '\'', '}'); // what about SP } ? (http://invisible-island.net/xterm/ctlseqs/ctlseqs.html)
	addCSI2Lookup(0, CS_DELETE_COLUMN, 0, -1, 1, '\'', '~'); // what about SP } ? (http://invisible-island.net/xterm/ctlseqs/ctlseqs.html)

	addCSI2Lookup(0, CS_SCROLL_RIGHT, 0, -1, 1, ' ', 'A');
	addCSI2Lookup(0, CS_SCROLL_LEFT, 0, -1, 1, ' ', '@');

	addCSILookup(0, CS_ICH, 1, 1, 1, '@');
	addCSILookup(0, CS_CURSOR_UP, 1, 1, 1, 'A');
	addCSILookup(0, CS_CURSOR_DOWN, 1, 1, 1, 'B');
	addCSILookup(0, CS_CURSOR_FORWARD, 1, 1, 1, 'C');
	addCSILookup(0, CS_CURSOR_BACKWARD, 1, 1, 1, 'D');
	addCSILookup(0, CS_CNL, 1, 1, 1, 'E');
	addCSILookup(0, CS_CPL, 1, 1, 1, 'F');
	addCSILookup(0, CS_CHA, 1, 1, 1, 'G');
	addCSILookup(0, CS_CURSOR_POSITION, 1, 2, 1, 'H');
	addCSILookup(0, CS_TAB_FORWARD, 1, 1, 1, 'I');
	addCSILookup(0, CS_ERASE_DISPLAY, 1, 1, 0, 'J');
	// addCSILookup('?', CS_SELECTIVE_ERASE_DISPLAY, 1, 1, 0, 'J'); // DECSED
	addCSILookup(0, CS_ERASE_LINE, 1, -1, 0, 'K');
	// addCSILookup('?', CS_SELECTIVE_ERASE_LINE, 1, 1, 0, 'J'); // DECSEL
	addCSILookup(0, CS_IL, 1, 1, 1, 'L');
	addCSILookup(0, CS_DL, 1, 1, 1, 'M');
	addCSILookup(0, CS_DCH, 1, 1, 1, 'P');
	// send only: addCSILookup(0, CS_CURSOR_POSITION_REPORT, 1, 2, 1, 'R');
	addCSILookup(0, CS_SU, 1, 1, 1, 'S');
	addCSILookup(0, CS_SD, 1, 1, 1, 'T');
	// send only?: ESC[...T - Mouse Tracking
	// addCSILookup('>' CS_TITLE_MODE, 0, -1, -1, 'T');
	addCSILookup(0, CS_ECH, 1, 1, 1, 'X');
	addCSILookup(0, CS_CBT, 1, 1, 1, 'Z');
	addCSILookup(0, CS_HPA, 1, 1, 1, '`');
	// TODO?: addCSILookup(0, CS_REP, 1, 1, 0, 'b');
	addCSILookup(0, CS_DEVICE_ATTR_PRIMARY_REQUEST, 1, 1, 0, 'c');
	// send only: addCSILookup('?', CS_DEVICE_ATTR_PRIMARY_RESPONSE, 1, -1, 0, 'c');
	addCSILookup('>', CS_DEVICE_ATTR_SECONDARY_REQUEST, 1, 1, 0, 'c');
	// send only: addCSILookup('>', CS_DEVICE_ATTR_SECONDARY_RESPONSE, 1, -1, 0, 'c');
	addCSILookup(0, CS_VPA, 1, 1, 1, 'd');
	addCSILookup(0, CS_CURSOR_POSITION, 1, 2, 1, 'f');
	addCSILookup(0, CS_TAB_CLEAR, 1, 1, 0, 'g');
	addCSILookup(0, CS_MODE_SET, 1, -1, 0, 'h');
	addCSILookup('?', CS_MODE_SET_PRIV, 1, -1, 0, 'h');
	addCSILookup(0, CS_MODE_RESET, 1, -1, 0, 'l');
	addCSILookup('?', CS_MODE_RESET_PRIV, 1, -1, 0, 'l');
	addCSILookup(0, CS_GRAPHICS_MODE_SET, 1, -1, 0, 'm');
	addCSILookup(0, CS_DEVICE_STATUS_REPORT, 1, -1, 0, 'n');
	addCSILookup(0, CS_MARGIN_SET, 0, 2, 0, 'r'); // default params are the window size
	addCSILookup(0, CS_CURSOR_POSITION_SAVE, 0, 0, 0, 's');
	addCSILookup(0, CS_CURSOR_POSITION_RESTORE, 0, 0, 0, 'u');
	addCSILookup(0, CS_TERM_PARAM, 1, 1, 0, 'x');

	addFixedLookup("=", CS_KEYPAD_APP_MODE);
	addFixedLookup(">", CS_KEYPAD_NUM_MODE);

	addFixedLookup("D", CS_INDEX);
	addFixedLookup("M", CS_REVERSE_INDEX);
	addFixedLookup("H", CS_TAB_SET);

	addFixedLookup("6", CS_BACK_INDEX);
	addFixedLookup("7", CS_CURSOR_POSITION_SAVE);
	addFixedLookup("8", CS_CURSOR_POSITION_RESTORE);
	addFixedLookup("9", CS_FORWARD_INDEX);
	addFixedLookup("(K", CS_USER_MAPPING);

	/* G0 charset */
	addFixedLookup("(A", CS_CHARSET_UK_G0_SET);
	addFixedLookup("(B", CS_CHARSET_ASCII_G0_SET);
	addFixedLookup("(0", CS_CHARSET_SPEC_G0_SET);
	addFixedLookup("(1", CS_CHARSET_ALT_G0_SET);
	addFixedLookup("(2", CS_CHARSET_ALT_SPEC_G0_SET);

	/* G1 charset */
	addFixedLookup(")A", CS_CHARSET_UK_G1_SET);
	addFixedLookup(")B", CS_CHARSET_ASCII_G1_SET);
	addFixedLookup(")0", CS_CHARSET_SPEC_G1_SET);
	addFixedLookup(")1", CS_CHARSET_ALT_G1_SET);
	addFixedLookup(")2", CS_CHARSET_ALT_SPEC_G1_SET);

	/* G2 charset */
	addFixedLookup("*A", CS_CHARSET_UK_G2_SET);
	addFixedLookup("*B", CS_CHARSET_ASCII_G2_SET);
	addFixedLookup("*0", CS_CHARSET_SPEC_G2_SET);
	addFixedLookup("*1", CS_CHARSET_ALT_G2_SET);
	addFixedLookup("*2", CS_CHARSET_ALT_SPEC_G2_SET);
	addFixedLookup("n", CS_CHARSET_USE_G2); /* use g2 */

	/* G3 charset */
	addFixedLookup("+A", CS_CHARSET_UK_G3_SET);
	addFixedLookup("+B", CS_CHARSET_ASCII_G3_SET);
	addFixedLookup("+0", CS_CHARSET_SPEC_G3_SET);
	addFixedLookup("+1", CS_CHARSET_ALT_G3_SET);
	addFixedLookup("+2", CS_CHARSET_ALT_SPEC_G3_SET);
	addFixedLookup("o", CS_CHARSET_USE_G3); /* use g3 */

	addFixedLookup("E", CS_MOVE_NEXT_LINE);

	addFixedLookup("#3", CS_DOUBLE_HEIGHT_LINE_TOP);
	addFixedLookup("#4", CS_DOUBLE_HEIGHT_LINE_BOTTOM);
	addFixedLookup("#5", CS_SINGLE_WIDTH_LINE);
	addFixedLookup("#6", CS_DOUBLE_WIDTH_LINE);
	addFixedLookup("#8", CS_SCREEN_ALIGNMENT_DISPLAY);

	addFixedLookup("c", CS_TERM_RESET);

	/* VT52 Compat */
	addVT52FixedLookup("<", CS_VT52_ANSI_MODE);
	addVT52FixedLookup("=", CS_VT52_KEYPAD_ALT_MODE);
	addVT52FixedLookup(">", CS_VT52_KEYPAD_NORMAL_MODE);
	addVT52FixedLookup("A", CS_VT52_CURSOR_UP);
	addVT52FixedLookup("B", CS_VT52_CURSOR_DOWN);
	addVT52FixedLookup("C", CS_VT52_CURSOR_RIGHT);
	addVT52FixedLookup("D", CS_VT52_CURSOR_LEFT);
	addVT52FixedLookup("F", CS_VT52_SPEC_CHARSET);
	addVT52FixedLookup("G", CS_VT52_ASCII_CHARSET);
	addVT52FixedLookup("J", CS_VT52_ERASE_SCREEN);
	addVT52FixedLookup("K", CS_VT52_ERASE_LINE);
	addVT52FixedLookup("H", CS_VT52_CURSOR_HOME);
	addVT52FixedLookup("I", CS_VT52_REVERSE_LINE_FEED);
	/* ESC Y -> CS_VT52_CURSOR_POSITION is done manually in the parser */
	addVT52FixedLookup("Z", CS_VT52_IDENTIFY);
}

bool ControlSeqParser::tryFixedEscape() {
	CS_Fixed_Lookup &table(m_vt52 ? m_csVT52Lookup : m_csFixedLookup);
	CS_Fixed_Lookup::iterator it;
	std::list<CS_Fixed_Entry>::const_iterator i, end;

	m_prefix[m_prefixlen] = 0;

	it = table.find(m_prefix[0]);
	if (it == table.end()) {
		/* invalid sequence (prefix) */

		syslog(LOG_DEBUG, "unknown esc sequence prefix: ESC '%s'", m_prefix);

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
				return (m_token != CS_UNKNOWN); /* skip CS_UNKOWN tokens */
			} else {
				/* need more */
				return false;
			}
		}
		++i;
	}

	/* invalid sequence (prefix) */
	syslog(LOG_DEBUG, "unknown esc sequence prefix: ESC '%s'", m_prefix);

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

	m_prefix[m_prefixlen] = 0;

	if (m_prefixlen > 1) {
		/* prefix too long, we don't use those */
		syslog(LOG_DEBUG, "unknown ESC[%s ... %c", m_prefix, m_currentChar);

		/* unknown csi function, skip */
		m_state = ST_START;
		return false;
	}

	it = m_csiLookup.find(m_currentChar);
	if (it == m_csiLookup.end()) {
		syslog(LOG_DEBUG, "unknown ESC[%s ... %c", m_prefix, m_currentChar);

		/* unknown csi function, skip */
		m_state = ST_START;
		return false;
	}

	i = it->second.begin();
	end = it->second.end();

	for (; i != end; ++i) {
		if (i->parameter == (m_prefixlen ? m_prefix[0] : 0)) {
			/* found function */

			/* invalid suffix */
			if (i->suffix != m_suffix) continue;

			/* empty param string should represent a default value. only add it if required. */
			if (m_numValues == 0 && 1 == i->minParams) { m_values[m_numValues++] = i->defaultVal; }

			/* invalid numberof parameters */
			if (m_numValues < i->minParams || (-1 != i->maxParams && m_numValues > i->maxParams)) {
				continue;
			}

			int k;
			for (k = 0; k < m_numValues; k++) {
				if (-1 == m_values[k]) {
					m_values[k] = i->defaultVal;
				}
			}
			for (; k < MAX_NUM_VALUES; k++) {
				/* reset all other values */
				m_values[k] = -1;
			}

#if 0
			syslog(LOG_DEBUG, "CSI seq(%i): ESC[%s%i;%i;%i;%i<...>%c",
				m_numValues, m_prefix,
				m_values[0], m_values[1], m_values[2], m_values[3],
				m_currentChar);
#endif

			m_token = i->token;
			m_state = ST_START;
			return true;
		}
	}

	syslog(LOG_DEBUG, "unknown ESC[%s ... %c", m_prefix, m_currentChar);

	/* unknown csi function, skip */
	m_state = ST_START;
	return false;
}

bool ControlSeqParser::parseChar() {
	switch (m_currentChar) {
	case 0x00: // ^@ NUL
	case 0x01: // ^A SOH
	case 0x02: // ^B STX
	case 0x03: // ^C ETX
	case 0x04: // ^D EOT
		return false; /* ignore */
	case 0x05: // ^E ENQ
		return true; // return character
	case 0x06: // ^F ACK
		return false; /* ignore */
	case 0x07: // ^G BEL
		if (m_state != ST_OSC && m_state != ST_OSC_ESC)
		{
			m_token = CS_ASCII_BEL;
			return true;
		}
		break; // terminator for OSC
	case 0x08: // ^H BS \b backspace
		m_token = CS_ASCII_BS;
		return true;
	case 0x09: // ^I HT \t
		m_token = CS_ASCII_TAB;
		return true;
	case 0x0A: // ^J LF \n
		m_token = CS_ASCII_LF;
		return true;
	case 0x0B: // ^K VT \v
		m_token = CS_ASCII_VT;
		return true;
	case 0x0C: // ^L FF \f
		m_token = CS_ASCII_FF;
		return true;
	case 0x0D: // ^M CR \r
		m_token = CS_ASCII_CR;
		return true; // return character
	case 0x0E: // ^N SO (shift out)
		m_token = CS_CHARSET_USE_G1;
		return true;
	case 0x0F: // ^O SI (shift in)
		m_token = CS_CHARSET_USE_G0;
		return true;
	case 0x10: // ^P DLE
	case 0x11: // ^Q DC1
	case 0x12: // ^R DC2
	case 0x13: // ^S DC3
	case 0x14: // ^T DC4
	case 0x15: // ^U NAK
	case 0x16: // ^B SYN
	case 0x17: // ^W ETB
		return false;  /* ignore */
	case 0x18: // ^X CAN
		m_state = ST_START;
		return false;
	case 0x19: // ^Y EM
		return false;  /* ignore */
	case 0x1A: // ^Z SUB
		m_state = ST_START;
		m_currentChar = 0x2592;
		return true;
	case 0x1B: // ^[ ESC
		m_state = ST_START;
		break;
	case 0x1C: // ^\\ FS
	case 0x1D: // ^] GS
	case 0x1E: // ^^ RS
	case 0x1F: // ^_ US
	case 0x7F: // DEL
		return false;  /* ignore */
	}
	switch (m_state) {
	case ST_START:
		if (ESC_CHAR == m_currentChar) {
			m_state = ST_ESCAPE;
			m_prefixlen = 0;
			m_numValues = 0;
			m_suffix = 0;
			return false;
		}
		if (MODE_7BIT != m_mode && m_currentChar >= 0x80 && m_currentChar < 0xA0) {
			m_state = ST_ESCAPE;
			m_prefixlen = 0;
			m_numValues = 0;
			m_suffix = 0;
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
		} else if (m_vt52 && 'Y' == m_currentChar) {
			m_state = ST_VT52_ESCY;
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
			m_suffix = m_currentChar;
			m_state = ST_CSI_SUFFIX;
		} else if (m_currentChar >= 0x40 && m_currentChar <= 0x7E) {
			/* final char */
			return matchCSI();
		} else {
			m_state = ST_CSI_INVALID;
		}
		return false;
	case ST_CSI_SUFFIX:
		if (m_currentChar >= 0x40 && m_currentChar <= 0x7E) {
			/* final char */
			return matchCSI();
		}
		return false;
	case ST_CSI_INVALID:
		if (m_currentChar >= 0x40 && m_currentChar <= 0x7E) {
			/* final char */
			m_state = ST_START;
		}
		return false;
	case ST_OSC:
		if (m_numValues < 2) m_oscString.clear();
		if (m_currentChar == 0x98 || m_currentChar == 0x9C || m_currentChar == 0x07) {
			m_state = ST_START;
			if (2 == m_numValues) {
				m_token = CS_OSC;
				m_numValues = 1;
				return true;
			}
		} else if (ESC_CHAR == m_currentChar) {
			m_state = ST_OSC_ESC;
		} else {
			if (m_numValues < 2) {
				/* no semicolon yet */
				if ((m_currentChar >= 0x30 && m_currentChar <= 0x39) || 0x3B == m_currentChar) {
					/* digit */
					parseCSIValue();
				} else {
					m_numValues = 3; /* "invalid" marker */
				}
			} else {
				appendUtf8Char(m_oscString, m_currentChar);
			}
		}
		return false;
	case ST_OSC_ESC:
		if (m_currentChar == 0x98 || m_currentChar == 0x9C || m_currentChar == 0x07 || m_currentChar == 'X' || m_currentChar == '\\') {
			m_state = ST_START;
			if (2 == m_numValues) {
				m_token = CS_OSC;
				m_numValues = 1;
				return true;
			}
		} else {
			/* throw away old state, start with new escape seq */
			m_state = ST_ESCAPE;
			m_prefixlen = 0;
			m_numValues = 0;
			return parseChar();
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
	case ST_VT52_ESCY:
		if (m_currentChar < 0x20) {
			/* cancel sequence, ignore previous part */
			m_state = ST_START;
			return true;
		}
		m_values[m_numValues++] = m_currentChar - 0x1F;
		if (2 == m_numValues) {
			m_state = ST_START;
			m_token = CS_VT52_CURSOR_POSITION;
			return true;
		}
		return false;
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
				m_utf8_seqlen = 1;
				/* new char */
				if (0 == (c & 0x80)) { m_currentChar = c; }
				else if (0xC0 == (c & 0xFE)) { continue; /* 0xCO / 0xC1 overlong */ }
				else if (0xC0 == (c & 0xE0)) { m_utf8_seqlen = 2; m_currentChar = c & 0x1f; }
				else if (0xE0 == (c & 0xF0)) { m_utf8_seqlen = 3; m_currentChar = c & 0x0f; }
				/* only 16-bit, seqlen 4 not supported */
				/* else if (0xF0 == (c & 0xF8)) { m_utf8_seqlen = 4; code = c & 0x07; } */
				else continue; /* skip invalid byte */
				m_utf8_remlen = m_utf8_seqlen - 1;
			} else {
				if (0x80 != (c & 0xC0)) {
					m_utf8_remlen = 0; /* skip invalid bytes */
					continue;
				}
				m_currentChar = (m_currentChar << 6) | (c & 0x3f);
				--m_utf8_remlen;
			}
			if (0 == m_utf8_remlen) {
				/* char complete */
				if ((3 == m_utf8_seqlen) && (m_currentChar < 0x800)) return false; /* overlong */
				return true;
			}
			break;
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

void ControlSeqParser::enableVT52() {
	m_vt52 = true;
}

void ControlSeqParser::disableVT52() {
	m_vt52 = false;
}

void ControlSeqParser::reset() {
	m_state = ST_START;
	m_mode = MODE_UTF8;
	m_vt52 = false;
}
