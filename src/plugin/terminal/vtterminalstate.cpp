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

#include "vtterminalstate.hpp"
#include "seqparser.hpp"

#include <stdlib.h>
#include <string.h>
#include <syslog.h>

VTTerminalState::VTTerminalState()
{
	m_parser = new ControlSeqParser();
}

VTTerminalState::~VTTerminalState()
{
	delete m_parser;
}

bool VTTerminalState::processNonPrintableChar(char &c)
{
	bool bPrint = false;

	pthread_mutex_lock(&m_rwLock);

	switch (c)
	{
	case 8: //Backspace.
		if (getCursorLocation().getX() >= 1)
		{
			moveCursorBackward(1);
		}
		break;
	default:
		bPrint = TerminalState::processNonPrintableChar(c);
		break;
	}

	pthread_mutex_unlock(&m_rwLock);

	return bPrint;
}

void VTTerminalState::processControlSeq(int nToken, int *values, int numValues, ExtTerminal *extTerminal)
{
	int i;

	pthread_mutex_lock(&m_rwLock);

	switch (nToken)
	{
	case CS_CURSOR_POSITION: //ESC[<Line>;<Column>H or ESC[<Line>;<Column>f
		values[0] = (values[0] <= 0) ? 1 : values[0];
		values[1] = (values[1] <= 0) ? 1 : values[1];
		setCursorLocation(values[1], values[0]);
		break;
	case CS_CURSOR_UP: //ESC[<Value>A
		values[0] = (values[0] <= 0) ? 1 : values[0];
		moveCursorUp(values[0]);
		break;
	case CS_CURSOR_DOWN: //ESC[<Value>B
		values[0] = (values[0] <= 0) ? 1 : values[0];
		moveCursorDown(values[0]);
		break;
	case CS_CURSOR_FORWARD: //ESC[<Value>C
		values[0] = (values[0] <= 0) ? 1 : values[0];
		moveCursorForward(values[0]);
		break;
	case CS_CURSOR_BACKWARD: //ESC[<Value>D
		values[0] = (values[0] <= 0) ? 1 : values[0];
		moveCursorBackward(values[0]);
		break;
	case CS_CURSOR_POSITION_SAVE: //ESC[s or ESC7
		saveCursor();
		break;
	case CS_CURSOR_POSITION_RESTORE: //ESC[u or ESC8
		restoreCursor();
		break;
	case CS_ERASE_DISPLAY: //ESC[<Value>;...;<Value>J
		if (numValues == 0)
		{
			eraseCursorToEndOfScreen();
		}
		else
		{
			for (i = 0; i < numValues; i++)
			{
				values[i] = (values[i] < 0) ? 0 : values[i];

				if (values[i] == 0)
				{
					eraseCursorToEndOfScreen();
				}
				else if (values[i] == 1)
				{
					eraseBeginOfScreenToCursor();
				}
				else if (values[i] == 2)
				{
					eraseScreen();
				}
			}
		}
		break;
	case CS_ERASE_LINE: //ESC[<Value>;...;<Value>K
		if (numValues == 0)
		{
			eraseCursorToEndOfLine();
		}
		else
		{
			for (i = 0; i < numValues; i++)
			{
				values[i] = (values[i] < 0) ? 0 : values[i];

				if (values[i] == 0)
				{
					eraseCursorToEndOfLine();
				}
				else if (values[i] == 1)
				{
					eraseBeginOfLineToCursor();
				}
				else if (values[i] == 2)
				{
					eraseCurrentLine();
				}
			}
		}
		break;
	case CS_GRAPHICS_MODE_SET: //ESC[<Value>;...;<Value>m
		if (numValues == 0)
		{
			memcpy(&m_currentGraphicsState, &m_defaultGraphicsState, sizeof(m_currentGraphicsState));
		}
		else
		{
			for (i = 0; i < numValues; i++)
			{
				values[i] = (values[i] < 0) ? 0 : values[i];

				if (values[i] == 0)
				{
					memcpy(&m_currentGraphicsState, &m_defaultGraphicsState, sizeof(m_currentGraphicsState));
				}
				else if (values[i] == 1)
				{
					addGraphicsModeFlags(TS_GM_BOLD);
				}
				else if (values[i] == 4)
				{
					addGraphicsModeFlags(TS_GM_UNDERSCORE);
				}
				else if (values[i] == 5)
				{
					addGraphicsModeFlags(TS_GM_BLINK);
				}
				else if (values[i] == 7)
				{
					addGraphicsModeFlags(TS_GM_NEGATIVE);
				}
				else if (values[i] >= 30 && values[i] <= 37)
				{
					if ((m_currentGraphicsState.nGraphicsMode & TS_GM_BOLD) > 0)
					{
						setForegroundColor((TSColor_t)(values[i] - 30 + TS_COLOR_BLACK_BRIGHT));
					}
					else
					{
						setForegroundColor((TSColor_t)(values[i] - 30));
					}
				}
				else if (values[i] == 39)
				{
					setForegroundColor(m_defaultGraphicsState.foregroundColor);
				}
				else if (values[i] >= 40 && values[i] <= 47)
				{
					if ((m_currentGraphicsState.nGraphicsMode & TS_GM_BOLD) > 0)
					{
						setBackgroundColor((TSColor_t)(values[i] - 40 + TS_COLOR_BLACK_BRIGHT));
					}
					else
					{
						setBackgroundColor((TSColor_t)(values[i] - 40));
					}
				}
				else if (values[i] == 49)
				{
					setBackgroundColor(m_defaultGraphicsState.backgroundColor);
				}
			}
		}
		break;
	case CS_MODE_SET: //ESC[<?><Value>;...;<Value>h
		for (i = 0; i < numValues; i++)
		{
			if (values[i] == 1)
			{
				addTerminalModeFlags(TS_TM_CURSOR_KEYS);
			}
			else if (values[i] == 2)
			{
				addTerminalModeFlags(TS_TM_VT52);
			}
			else if (values[i] == 3)
			{
				addTerminalModeFlags(TS_TM_COLUMN);
			}
			else if (values[i] == 4)
			{
				addTerminalModeFlags(TS_TM_SCROLL);
			}
			else if (values[i] == 5)
			{
				addTerminalModeFlags(TS_TM_SCREEN);
			}
			else if (values[i] == 6)
			{
				addTerminalModeFlags(TS_TM_ORIGIN);
			}
			else if (values[i] == 7)
			{
				addTerminalModeFlags(TS_TM_AUTO_WRAP);
			}
			else if (values[i] == 8)
			{
				addTerminalModeFlags(TS_TM_AUTO_REPEAT);
			}
			else if (values[i] == 9)
			{
				//FIXME Not implemented.
				//Interlace.
			}
			else if (values[i] == 20)
			{
				addTerminalModeFlags(TS_TM_NEW_LINE);
			}
		}
		break;
	case CS_MODE_RESET: //ESC[<?><Value>;...;<Value>l
		for (i = 0; i < numValues; i++)
		{
			if (values[i] == 1)
			{
				removeTerminalModeFlags(TS_TM_CURSOR_KEYS);
			}
			else if (values[i] == 2)
			{
				removeTerminalModeFlags(TS_TM_VT52);
			}
			else if (values[i] == 3)
			{
				removeTerminalModeFlags(TS_TM_COLUMN);
			}
			else if (values[i] == 4)
			{
				removeTerminalModeFlags(TS_TM_SCROLL);
			}
			else if (values[i] == 5)
			{
				removeTerminalModeFlags(TS_TM_SCREEN);
			}
			else if (values[i] == 6)
			{
				removeTerminalModeFlags(TS_TM_ORIGIN);
			}
			else if (values[i] == 7)
			{
				removeTerminalModeFlags(TS_TM_AUTO_WRAP);
			}
			else if (values[i] == 8)
			{
				removeTerminalModeFlags(TS_TM_AUTO_REPEAT);
			}
			else if (values[i] == 9)
			{
				//FIXME Not implemented.
				//Interlace.
			}
			else if (values[i] == 20)
			{
				removeTerminalModeFlags(TS_TM_NEW_LINE);
			}
		}
		break;
	case CS_KEYPAD_APP_MODE: //ESC=
	case CS_KEYPAD_NUM_MODE: //ESC>
		//FIXME Not implemented.
		syslog(LOG_ERR, "VT100 Control Sequence: KEYPAD not implemented.");
		break;
	case CS_CHARSET_UK_G0_SET: //ESC(A
		setCharset(TS_CS_G0_UK);
		break;
	case CS_CHARSET_UK_G1_SET: //ESC)A
		setCharset(TS_CS_G1_UK);
		break;
	case CS_CHARSET_US_G0_SET: //ESC(B
		setCharset(TS_CS_G0_ASCII);
		break;
	case CS_CHARSET_US_G1_SET: //ESC)B
		setCharset(TS_CS_G1_ASCII);
		break;
	case CS_CHARSET_SPEC_G0_SET: //ESC(0
		setCharset(TS_CS_G0_SPEC);
		break;
	case CS_CHARSET_SPEC_G1_SET: //ESC)0
		setCharset(TS_CS_G1_SPEC);
		break;
	case CS_CHARSET_ALT_G0_SET: //ESC(1
		setCharset(TS_CS_G0_ALT_STD);
		break;
	case CS_CHARSET_ALT_G1_SET: //ESC)1
		setCharset(TS_CS_G1_ALT_STD);
		break;
	case CS_CHARSET_ALT_SPEC_G0_SET: //ESC(2
		setCharset(TS_CS_G0_ALT_SPEC);
		break;
	case CS_CHARSET_ALT_SPEC_G1_SET: //ESC)2
		setCharset(TS_CS_G1_ALT_SPEC);
		break;
	case CS_MARGIN_SET: //ESC[<Top>;<Bottom>r
		values[0] = (values[0] <= 0) ? 1 : values[0];
		values[1] = (values[1] <= 0) ? 1 : values[1];
		values[1] = (values[1] <= values[0]) ? (values[0] + 1) : values[1];
		setMargin(values[0], values[1]);
		break;
	case CS_MOVE_UP: //ESCD
		moveCursorUp(1, true);
		break;
	case CS_MOVE_DOWN: //ESCM
		moveCursorDown(1, true);
		break;
	case CS_MOVE_NEXT_LINE: //ESCE
		moveCursorNextLine();
		break;
	case CS_TAB: //ESCH
		insertChar(9, false, false);
		break;
	case CS_TAB_CLEAR: //ESC[<Value>;...;<Value>g
		//FIXME Not implemented.
		syslog(LOG_ERR, "VT100 Control Sequence: CLEAR TAB not implemented.", nToken);
		break;
	case CS_DOUBLE_HEIGHT_LINE_TOP: //ESC#3
	case CS_DOUBLE_HEIGHT_LINE_BOTTOM: //ESC#4
	case CS_SINGLE_WIDTH_LINE: //ESC#5
	case CS_DOUBLE_WIDTH_LINE: //ESC#6
		//FIXME Not implemented.
		syslog(LOG_ERR, "VT100 Control Sequence: DOUBLE CELL not implemented.", nToken);
		break;
	case CS_DEVICE_STATUS_REPORT: //ESC[<Value>;...;<Value>n
		if (extTerminal != NULL && extTerminal->isReady())
		{
			for (i = 0; i < numValues; i++)
			{
				if (values[i] == 5)
				{
					extTerminal->insertData("\x1B[0n", 1);
				}
				else if (values[i] == 6)
				{
					char buf[32];

					sprintf(buf, "\x1B[%d;%dR", getCursorLocation().getY(), getCursorLocation().getX());
					extTerminal->insertData(buf, 1);
				}
			}
		}
		break;
	case CS_DEVICE_ATTR_PRIMARY_REQUEST: //ESC[<Value>c
	case CS_DEVICE_ATTR_PRIMARY_RESPONSE: //ESC[?6c
		if (extTerminal != NULL && extTerminal->isReady())
			extTerminal->insertData("\x1B[?6c", 1);
		break;
	case CS_DEVICE_ATTR_SECONDARY_REQUEST: //ESC[><Value>c
	case CS_DEVICE_ATTR_SECONDARY_RESPONSE: //ESC[>0;115;0c
		if (extTerminal != NULL && extTerminal->isReady())
			extTerminal->insertData("\x1B[>0;115;0c", 1);
		break;
	case CS_TERM_IDENTIFY: //ESCZ
		//FIXME Not implemented.
		syslog(LOG_ERR, "VT100 Control Sequence: TERM IDENTIFY not implemented.", nToken);
		break;
	case CS_TERM_PARAM: //ESC[<Value>;...;<Value>x
	case CS_TERM_RESET: //ESCc
		//FIXME Not implemented
		syslog(LOG_ERR, "VT100 Control Sequence: TERM PARAM not implemented.", nToken);
		break;
	default:
		syslog(LOG_ERR, "VT100 Control Sequence: %d not implemented.", nToken);
		break;
	}

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Inserts a null terminating string to the terminal.
 * The string may contain VT100 control sequences, which will invoke
 * the underlying commands.
 */
void VTTerminalState::insertString(const char *sStr, ExtTerminal *extTerminal)
{
	pthread_mutex_lock(&m_rwLock);

	if (sStr == NULL)
	{
		return;
	}

	size_t size = ControlSeqParser::MAX_NUM_VALUES * sizeof(int);
	int nValues = 0;
	int *values = (int *)malloc(size);
	int nSeqLength = 0;
	int nToken = 0;
	int nLength = strlen(sStr);
	int nCurrentIndex = 0;

	while (nCurrentIndex < nLength)
	{
		memset(values, 0, size);
		nToken = m_parser->parse(sStr + nCurrentIndex, values, &nValues, &nSeqLength);

		if (nToken != CS_UNKNOWN)
		{
			processControlSeq(nToken, values, nValues, extTerminal);
		}
		else
		{
			//Treat as text.
			insertChar(sStr[nCurrentIndex], true, false, isShiftText());
		}

		nCurrentIndex += (nSeqLength > 0) ? nSeqLength : 1;
	}

	free(values);

	pthread_mutex_unlock(&m_rwLock);
}

void VTTerminalState::sendCursorCommand(VTTS_Cursor_t cursor, ExtTerminal *extTerminal)
{
	if (extTerminal != NULL)
	{
		bool bCursorKeys = ((getTerminalModeFlags() & TS_TM_CURSOR_KEYS) != 0);

		if (cursor == VTTS_CURSOR_UP)
		{
			if (!bCursorKeys)
			{
				extTerminal->insertData("\x1B[A", 1);
			}
			else
			{
				extTerminal->insertData("\x1BOA", 1);
			}
		}
		else if (cursor == VTTS_CURSOR_DOWN)
		{
			if (!bCursorKeys)
			{
				extTerminal->insertData("\x1B[B", 1);
			}
			else
			{
				extTerminal->insertData("\x1BOB", 1);
			}
		}
		else if (cursor == VTTS_CURSOR_RIGHT)
		{
			if (!bCursorKeys)
			{
				extTerminal->insertData("\x1B[C", 1);
			}
			else
			{
				extTerminal->insertData("\x1BOC", 1);
			}
		}
		else if (cursor == VTTS_CURSOR_LEFT)
		{
			if (!bCursorKeys)
			{
				extTerminal->insertData("\x1B[D", 1);
			}
			else
			{
				extTerminal->insertData("\x1BOD", 1);
			}
		}
	}
}
