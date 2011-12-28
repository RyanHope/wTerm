/**
 * This file is part of SDLTerminal.
 * Copyright (C) 2011 Vincent Ho <www.whimsicalvee.com>
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

#include "seqparser.hpp"

const char ControlSeqParser::ESC_CHAR = 27;
const char ControlSeqParser::DELIMITER_CHAR = ';';
const int ControlSeqParser::MAX_NUM_VALUES = 20;

ControlSeqParser::ControlSeqParser()
{
	m_seq = NULL;
	m_savedPos = 0;
	m_values = (int *)malloc(sizeof(int) * MAX_NUM_VALUES);
	reset();
	buildLookup();
}

ControlSeqParser::~ControlSeqParser()
{
	if (m_values != NULL)
	{
		free(m_values);
	}

	freeLookup();
}

void ControlSeqParser::freeLookup()
{
	for (std::map<const char *, std::list<CSEntry_t *> *, cmp_str>::iterator itr = m_csLookup.begin(); itr != m_csLookup.end(); itr++)
	{
		std::list<CSEntry_t *> *list = itr->second;

		for (std::list<CSEntry_t *>::iterator itr2 = list->begin(); itr2 != list->end(); itr2++)
		{
			free(*itr2);
		}

		delete list;
	}
}

void ControlSeqParser::addLookupEntry(const char *sCSI, CSToken_t token, int nMinParams, int nMaxParams, int nDefaultVal, char cFinal)
{
	CSEntry_t *entry = (CSEntry_t *)malloc(sizeof(CSEntry_t));
	std::list<CSEntry_t *> *list;

	entry->m_sCSI = sCSI;
	entry->m_token = token;
	entry->m_nMinParams = nMinParams;
	entry->m_nMaxParams = nMaxParams;
	entry->m_nDefaultVal = nDefaultVal;
	entry->m_cFinal = cFinal;

	if (m_csLookup.find(sCSI) == m_csLookup.end())
	{
		m_csLookup[sCSI] = new std::list<CSEntry_t *>();
	}

	list = m_csLookup[sCSI];
	list->push_back(entry);
}

/**
 * Builds the lookup table to facilitate parsing.
 */
void ControlSeqParser::buildLookup()
{
	//Should only build once.
	if (!m_csLookup.empty())
	{
		return;
	}

	addLookupEntry("[", CS_CURSOR_POSITION_REPORT, 0, 2, 1, 'R');
	addLookupEntry("[", CS_CURSOR_POSITION, 0, 2, 1, 'H');
	addLookupEntry("[", CS_CURSOR_POSITION, 0, 2, 1, 'f');
	addLookupEntry("[", CS_CURSOR_UP, 0, 1, 1, 'A');
	addLookupEntry("[", CS_CURSOR_DOWN, 0, 1, 1, 'B');
	addLookupEntry("[", CS_CURSOR_FORWARD, 0, 1, 1, 'C');
	addLookupEntry("[", CS_CURSOR_BACKWARD, 0, 1, 1, 'D');
	addLookupEntry("[", CS_CURSOR_POSITION_SAVE, 0, 0, 1, 's');
	addLookupEntry("", CS_CURSOR_POSITION_SAVE, 0, 0, 1, '7');
	addLookupEntry("[", CS_CURSOR_POSITION_RESTORE, 0, 0, 1, 'u');
	addLookupEntry("", CS_CURSOR_POSITION_RESTORE, 0, 0, 1, '8');
	addLookupEntry("[", CS_ERASE_DISPLAY, 0, -1, 1, 'J');
	addLookupEntry("[", CS_ERASE_LINE, 0, -1, 1, 'K');
	addLookupEntry("[", CS_GRAPHICS_MODE_SET, 0, -1, 1, 'm');
	addLookupEntry("[", CS_MODE_SET, 0, -1, 1, 'h');
	addLookupEntry("[?", CS_MODE_SET, 0, -1, 1, 'h');
	addLookupEntry("[", CS_MODE_RESET, 0, -1, 1, 'l');
	addLookupEntry("[?", CS_MODE_RESET, 0, -1, 1, 'l');

	addLookupEntry("", CS_KEYPAD_APP_MODE, 0, 0, 1, '=');
	addLookupEntry("", CS_KEYPAD_NUM_MODE, 0, 0, 1, '>');

	addLookupEntry("", CS_INDEX, 0, 0, 0, 'D');
	addLookupEntry("", CS_REVERSE_INDEX, 0, 0, 0, 'M');

	addLookupEntry("(", CS_USER_MAPPING, 0, 0, 0, 'K');

	addLookupEntry("(", CS_CHARSET_UK_G0_SET, 0, 0, 1, 'A');
	addLookupEntry("(", CS_CHARSET_US_G0_SET, 0, 0, 1, 'B');
	addLookupEntry("(", CS_CHARSET_SPEC_G0_SET, 0, 0, 1, '0');
	addLookupEntry("(", CS_CHARSET_ALT_G0_SET, 0, 0, 1, '1');
	addLookupEntry("(", CS_CHARSET_ALT_SPEC_G0_SET, 0, 0, 1, '2');
	addLookupEntry(")", CS_CHARSET_UK_G1_SET, 0, 0, 1, 'A');
	addLookupEntry(")", CS_CHARSET_US_G1_SET, 0, 0, 1, 'B');
	addLookupEntry(")", CS_CHARSET_SPEC_G1_SET, 0, 0, 1, '0');
	addLookupEntry(")", CS_CHARSET_ALT_G1_SET, 0, 0, 1, '1');
	addLookupEntry(")", CS_CHARSET_ALT_SPEC_G1_SET, 0, 0, 1, '2');

	addLookupEntry("[", CS_MARGIN_SET, 0, 2, 1, 'r');
	addLookupEntry("", CS_MOVE_UP, 0, 0, 1, 'D');
	addLookupEntry("", CS_MOVE_DOWN, 0, 0, 1, 'M');
	addLookupEntry("", CS_MOVE_NEXT_LINE, 0, 0, 1, 'E');

	addLookupEntry("", CS_TAB, 0, 0, 1, 'H');
	addLookupEntry("[", CS_TAB_CLEAR, 0, -1, 1, 'g');

	addLookupEntry("#", CS_DOUBLE_HEIGHT_LINE_TOP, 0, 0, 1, '3');
	addLookupEntry("#", CS_DOUBLE_HEIGHT_LINE_BOTTOM, 0, 0, 1, '4');
	addLookupEntry("#", CS_SINGLE_WIDTH_LINE, 0, 0, 1, '5');
	addLookupEntry("#", CS_DOUBLE_WIDTH_LINE, 0, 0, 1, '6');
	addLookupEntry("#", CS_SCREEN_ALIGNMENT_DISPLAY, 0, 0, 0, '8');

	addLookupEntry("[", CS_DEVICE_STATUS_REPORT, 0, -1, 1, 'n');

	addLookupEntry("[", CS_DEVICE_ATTR_PRIMARY_REQUEST, 0, 1, 0, 'c');
	addLookupEntry("[?", CS_DEVICE_ATTR_PRIMARY_RESPONSE, 0, -1, 0, 'c');

	addLookupEntry("[>", CS_DEVICE_ATTR_SECONDARY_REQUEST, 0, 1, 0, 'c');
	addLookupEntry("[>", CS_DEVICE_ATTR_SECONDARY_RESPONSE, 0, -1, 0, 'c');

	addLookupEntry("", CS_TERM_IDENTIFY, 0, 0, 1, 'Z');
	addLookupEntry("[", CS_TERM_PARAM, 0, -1, 1, 'x');
	addLookupEntry("", CS_TERM_RESET, 0, 0, 1, 'c');
}

/**
 * Advance to the next character of the string.
 */
void ControlSeqParser::nextChar()
{
	if (m_seq != NULL)
	{
		m_currentChar = m_seq[++m_currentPos];
	}
}

void ControlSeqParser::reset()
{
	m_currentPos = 0;
	m_numValues = 0;

	if (m_seq != NULL)
	{
		m_currentChar = m_seq[m_currentPos];
	}
	else
	{
		m_currentChar = 0;
	}

	for (int i = 0; i < MAX_NUM_VALUES; i++)
	{
		m_values[i] = -1;
	}
}

/**
 * Returns the number of values read.
 */
int ControlSeqParser::parsePositiveInt(int *values, int maxNumValues)
{
	bool bDone = false;
	bool bParseLast = false;
	int value = -1;
	int numValues = 0;

	while(!bDone)
	{
		switch(m_currentChar)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			value = (value < 0) ? (m_currentChar - '0') : ((value * 10) + (m_currentChar - '0'));
			nextChar();
			break;
		case DELIMITER_CHAR:
			if (numValues < maxNumValues && values != NULL)
			{
				values[numValues] = value;
			}

			value = -1; //Reset value.
			++numValues;

			//If there's a delimiter, then it must be followed by another number.
			bParseLast = true;
			nextChar();
			break;
		default:
			if (value >= 0 || bParseLast)
			{
				if (numValues < maxNumValues && values != NULL)
				{
					values[numValues] = value;
				}

				++numValues;
			}

			bDone = true;
			break;
		}
	}

	return numValues;
}

int ControlSeqParser::match(char *prefix, char suffix, int *values, int numValues)
{
	std::map<const char *, std::list<CSEntry_t *> *, cmp_str>::iterator locator = m_csLookup.find(prefix);
	std::list<CSEntry_t *> *list;

	if (locator == m_csLookup.end())
	{
		return CS_UNKNOWN;
	}

	list = locator->second;

	for (std::list<CSEntry_t *>::iterator itr = list->begin(); itr != list->end(); itr++)
	{
		CSEntry_t *entry = *itr;

		if (numValues >= entry->m_nMinParams && (entry->m_nMaxParams == -1 || numValues <= entry->m_nMaxParams))
		{
			if (entry->m_cFinal == suffix)
			{
				return entry->m_token;
			}
		}
	}

	return CS_UNKNOWN;
}

/**
 * Check if the prefix partially matches any known control sequences.
 */
bool ControlSeqParser::isPartialMatch(char *prefix)
{
	if (strcmp(prefix, "") == 0)
	{
		return true;
	}

	for (std::map<const char *, std::list<CSEntry_t *> *, cmp_str>::iterator itr = m_csLookup.begin(); itr != m_csLookup.end(); itr++)
	{
		const char *sCSI = itr->first;

		if (strstr(sCSI, prefix) == sCSI)
		{
			return true;
		}
	}

	return false;
}

/**
 * Check if the prefix and the parameters matches any known control sequences entirely.
 */
bool ControlSeqParser::isPartialMatch(char *prefix, int *values, int numValues)
{
	std::map<const char *, std::list<CSEntry_t *> *, cmp_str>::iterator locator = m_csLookup.find(prefix);
	std::list<CSEntry_t *> *list;

	if (locator == m_csLookup.end())
	{
		return false;
	}

	list = locator->second;

	for (std::list<CSEntry_t *>::iterator itr = list->begin(); itr != list->end(); itr++)
	{
		CSEntry_t *entry = *itr;

		if (numValues >= entry->m_nMinParams && (entry->m_nMaxParams == -1 || numValues <= entry->m_nMaxParams))
		{
			return true;
		}
	}

	return false;
}

/**
 * Parse a possible sequence extract it into prefix, suffix, and parameters.
 */
int ControlSeqParser::parseSeq()
{
	bool bDone = false;
	char prefix[4] = { 0, 0, 0, 0 };
	char suffix = 0;
	int nResult;
	int nPrefixIndex = 0;

	reset();

	while(!bDone)
	{
		switch(m_currentPos)
		{
		case 0:
			if (m_currentChar == ESC_CHAR)
			{
				nextChar();
			}
			else
			{
				bDone = true;
			}
			break;
		default:
			if (m_currentChar == '\0')
			{
				//End of string.
				bDone = true;
				break;
			}

			//First test if the current state matches any control sequence in full.
			if ((nResult = match(prefix, m_currentChar, m_values, m_numValues)) != CS_UNKNOWN)
			{
				nextChar();
				return nResult;
			}

			prefix[nPrefixIndex++] = m_currentChar;

			//Check if the prefix might lead to a valid sequence.
			if (!isPartialMatch(prefix))
			{
				//Match not found, maybe number parameters are located here instead.
				prefix[--nPrefixIndex] = 0;
				m_numValues = parsePositiveInt(m_values, MAX_NUM_VALUES);

				if (!isPartialMatch(prefix, m_values, m_numValues))
				{
					//Unknown sequence.
					bDone = true;
				}
				else
				{
					//Must end after read parameters.
					nResult = match(prefix, m_currentChar, m_values, m_numValues);
					nextChar();

					return nResult;
				}
			}
			else
			{
				nextChar();
			}
			break;
		}
	}

	return CS_UNKNOWN;
}

/**
 * Parses a string and extract VT100 control sequence information.
 * @param seq The string to parse.
 * @param values A pointer to an array that will hold the resultant values. The memory must already be allocated.
 * @param numValues A pointer to an integer that will hold the number of resultant values.
 * @param seqLength A pointer to an integer that will hold the length of the control sequence.
 * @return The token number that identifies the type of the control sequence.
 */
int ControlSeqParser::parse(const char *seq, int *values, int *numValues, int *seqLength)
{
	int result = CS_UNKNOWN;

	if (seq != NULL)
	{
		m_seq = seq;
		result = parseSeq();
	}

	if (seqLength != NULL)
	{
		*seqLength = 0;
	}

	if (result != CS_UNKNOWN)
	{
		if (seqLength != NULL)
		{
			*seqLength = m_currentPos;
		}

		if (numValues != NULL)
		{
			*numValues = m_numValues;
		}

		if (values != NULL)
		{
			for (int i=0; i < MAX_NUM_VALUES && i < m_numValues; i++)
			{
				values[i] = m_values[i];
			}
		}
	}

	return result;
}
