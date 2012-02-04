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

#include "vtterminalstate.hpp"
#include "seqparser.hpp"

#include <PDL.h>

#include <algorithm>

#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>

VTTerminalState::VTTerminalState()
{
	m_parser = new ControlSeqParser();
}

VTTerminalState::~VTTerminalState()
{
	delete m_parser;
}

void VTTerminalState::processControlSeq(int nToken, int *values, int numValues, ExtTerminal *extTerminal)
{
	int i;

	pthread_mutex_lock(&m_rwLock);

	switch (nToken)
	{
	case CS_ASCII_BEL:
		PDL_CallJS("bell", NULL, 0);
		break;
	case CS_ASCII_BS: //Backspace
		moveCursorBackward(1);
		break;
	case CS_ASCII_TAB: //Tab
		tabForward(1);
		break;
	case CS_ASCII_LF: //Linefeed
		((getTerminalModeFlags() & TS_TM_NEW_LINE) > 0) ? moveCursorNextLine() : moveCursorDown(1, true);
		break;
	case CS_ASCII_VT: //Vertical tab
	case CS_ASCII_FF: //Form feed
		moveCursorDown(1, true);
		break;
	case CS_ASCII_CR: //Carriage return
		setCursorLocation(1, getCursorLocation().getY());
		break;
	case CS_CBT: //ESC[<Tabs>Z
		tabBackward(values[0]);
		break;
	case CS_HPA: //ESC[<Column>`
	case CS_CHA: //ESC[<Column>G
		setCursorLocation(values[0], m_cursorLoc.getY());
		break;
	case CS_VPA: //ESC[<Line>d
		setCursorLocation(m_cursorLoc.getX(), values[0]);
		break;
	case CS_ECH: //ESC[<Chars>X
		eraseCharacters(values[0] ? values[0] : 1);
		break;
	case CS_IL: //ESC[<Lines>L
		insertLines(values[0] ? values[0] : 1);
		break;
	case CS_DL: //ESC[<Lines>M
		deleteLines(values[0] ? values[0] : 1);
		break;
	case CS_DCH: //ESC[<Chars>P
		deleteCharacters(values[0] ? values[0] : 1);
		break;
	case CS_ICH: //ESC[<Blanks>@
		insertBlanks(values[0] ? values[0] : 1);
		break;
	case CS_SU: //ESC[<Value>S Scroll Up Lines
		m_screenBuffer.scrollLines(m_nTopMargin, m_nBottomMargin, values[0] ? values[0] : 1, TSCell(BLANK, m_currentGraphicsState));
		break;
	case CS_SD: //ESC[<Value>T Scroll Down Lines
		m_screenBuffer.scrollLines(m_nTopMargin, m_nBottomMargin, -(values[0] ? values[0] : 1), TSCell(BLANK, m_currentGraphicsState));
		break;
	case CS_TAB_SET: //ESCH TAB SET
		{
			bool added = false;
			int x = m_cursorLoc.getX();
			for (unsigned int i=0; i < tabs.size(); i++) {
				if (x < tabs[i]) {
					tabs.insert(tabs.begin() + i, x);
					added = true;
					break;
				} else if (x == tabs[i]) {
					/* don't add duplicates */
					added = true;
					break;
				}
			}
			if (!added) tabs.push_back(x);
		}
		break;
	case CS_TAB_CLEAR: //ESC[<Value>g TAB CLEAR
		switch (values[0]) {
		case 0:
			{
				int x = m_cursorLoc.getX();
				for (unsigned int i=0; i < tabs.size(); ++i) {
					if (x == tabs[i]) {
						tabs.erase(tabs.begin()+i);
						break;
					} else if (x < tabs[i]) {
						break;
					}
				}
			}
			break;
		case 3:
			tabs.clear();
			break;
		}
		break;
	case CS_BACK_INDEX: // ESC6
		backIndex();
		break;
	case CS_FORWARD_INDEX: // ESC9
		forwardIndex();
		break;
	case CS_TAB_FORWARD: //ESC[<Tabs>I
		tabForward(values[0]);
		break;
	case CS_INDEX: //ESCD
		moveCursorDown(1, true);
		break;
	case CS_VT52_REVERSE_LINE_FEED: // ESCI
	case CS_REVERSE_INDEX: //ESCM
		moveCursorUp(1, true);
		break;
	case CS_CURSOR_POSITION: //ESC[<Line>;<Column>H or ESC[<Line>;<Column>f
		values[1] = (numValues >= 2) ? values[1] : 1;
		setCursorLocation(values[1], values[0]);
		break;
	case CS_VT52_CURSOR_UP: // ESCA
		values[0] = 1;
	case CS_CURSOR_UP: //ESC[<Value>A
		moveCursorUp(values[0] ? values[0] : 1);
		break;
	case CS_VT52_CURSOR_DOWN: // ESCB
		values[0] = 1;
	case CS_CURSOR_DOWN: //ESC[<Value>B
		moveCursorDown(values[0] ? values[0] : 1);
		break;
	case CS_VT52_CURSOR_RIGHT: // ESCC
		values[0] = 1;
	case CS_CURSOR_FORWARD: //ESC[<Value>C
		moveCursorForward(values[0] ? values[0] : 1);
		break;
	case CS_VT52_CURSOR_LEFT: // ESCD
		values[0] = 1;
	case CS_CURSOR_BACKWARD: //ESC[<Value>D
		moveCursorBackward(values[0] ? values[0] : 1);
		break;
	case CS_VT52_CURSOR_HOME: // ESCH
		cursorHome();
		break;
	case CS_CURSOR_POSITION_SAVE: //ESC[s or ESC7
		saveCursor();
		break;
	case CS_CURSOR_POSITION_RESTORE: //ESC[u or ESC8
		restoreCursor();
		break;
	case CS_VT52_ERASE_SCREEN: //ESCJ
		eraseCursorToEndOfScreen();
		break;
	case CS_ERASE_DISPLAY: //ESC[<Value>J
		switch (values[0]) {
		case 0:
			eraseCursorToEndOfScreen();
			break;
		case 1:
			eraseBeginOfScreenToCursor();
			break;
		case 2:
			eraseScreen();
			break;
		default: break;
		}
		break;
	case CS_VT52_ERASE_LINE: //ESCK
		eraseCursorToEndOfLine();
		break;
	case CS_ERASE_LINE: //ESC[<Value>K
		switch (values[0]) {
		case 0:
			eraseCursorToEndOfLine();
			break;
		case 1:
			eraseBeginOfLineToCursor();
			break;
		case 2:
			eraseCurrentLine();
			break;
		}
		break;
	case CS_VT52_CURSOR_POSITION://ESCY<char(row+0x19)><char(col+0x19)>
		setCursorLocation(values[1], values[0]);
		break;
	case CS_GRAPHICS_MODE_SET: //ESC[<Value>;...;<Value>m
		if (numValues == 0)
		{
			m_currentGraphicsState.reset();;
		}
		else
		{
			for (i = 0; i < numValues; i++)
			{
				if (values[i] == 0)
				{
					m_currentGraphicsState.reset();
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
					setForegroundColor((TSColor)(values[i] - 30));
				}
				else if (values[i] == 39)
				{
					setForegroundColor(TSGraphicsState::DEFAULT_FOREGROUND_COLOR);
				}
				else if (values[i] >= 40 && values[i] <= 47)
				{
					setBackgroundColor((TSColor)(values[i] - 40));
				}
				else if (values[i] == 49)
				{
					setBackgroundColor(TSGraphicsState::DEFAULT_BACKGROUND_COLOR);
				}
				else if (values[i] == 22)
				{
					removeGraphicsModeFlags(TS_GM_BOLD);
				}
				else if (values[i] == 24)
				{
					removeGraphicsModeFlags(TS_GM_UNDERSCORE);
				}
				else if (values[i] == 25)
				{
					removeGraphicsModeFlags(TS_GM_BLINK);
				}
				else if (values[i] == 27)
				{
					removeGraphicsModeFlags(TS_GM_NEGATIVE);
				}
			}
		}
		break;
	case CS_MODE_SET: //ESC[<Value>;...;<Value>h
		for (i = 0; i < numValues; i++)
		{
			if (values[i] == 4)
			{
				addTerminalModeFlags(TS_TM_INSERT);
			}
			else if (values[i] == 20)
			{
				addTerminalModeFlags(TS_TM_NEW_LINE);
			}
		}
		break;
	case CS_VT52_ANSI_MODE: //ESC<
		removeTerminalModeFlags(TS_TM_VT52);
		m_parser->disableVT52();
		break;
	case CS_MODE_SET_PRIV: //ESC[<?><Value>;...;<Value>h
		for (i = 0; i < numValues; i++)
		{
			if (values[i] == 1)
			{
				addTerminalModeFlags(TS_TM_CURSOR_KEYS);
			}
			else if (values[i] == 3)
			{
				addTerminalModeFlags(TS_TM_COLUMN);
				eraseScreen();
				cursorHome();
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
				cursorHome();
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
			else if (values[i] == 25)
			{
				addTerminalModeFlags(TS_TM_CURSOR);
			}
			else if (values[i] == 67)
			{
				addTerminalModeFlags(TS_TM_BACKSPACE);
			}
			else if (values[i] == 47)
			{
				saveScreen();
				eraseScreen();
			}
			else if (values[i] == 1047)
			{
				saveScreen();
				eraseScreen();
			}
			else if (values[i] == 1048)
			{
				saveCursor();
			}
			else if (values[i] == 1049)
			{
				saveCursor();
				saveScreen();
				eraseScreen();
			}
		}
		break;
	case CS_MODE_RESET: //ESC[<Value>;...;<Value>l
		for (i = 0; i < numValues; i++)
		{
			if (values[i] == 4)
			{
				removeTerminalModeFlags(TS_TM_INSERT);
			}
			else if (values[i] == 20)
			{
				removeTerminalModeFlags(TS_TM_NEW_LINE);
			}
		}
		break;
	case CS_MODE_RESET_PRIV: //ESC[<?><Value>;...;<Value>l
		for (i = 0; i < numValues; i++)
		{
			if (values[i] == 1)
			{
				removeTerminalModeFlags(TS_TM_CURSOR_KEYS);
			}
			else if (values[i] == 2)
			{
				addTerminalModeFlags(TS_TM_VT52);
				m_parser->enableVT52();
			}
			else if (values[i] == 3)
			{
				removeTerminalModeFlags(TS_TM_COLUMN);
				eraseScreen();
				cursorHome();
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
				cursorHome();
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
			else if (values[i] == 25)
			{
				removeTerminalModeFlags(TS_TM_CURSOR);
			}
			else if (values[i] == 67)
			{
				removeTerminalModeFlags(TS_TM_BACKSPACE);
			}
			else if (values[i] == 47)
			{
				restoreScreen();
			}
			else if (values[i] == 1047)
			{
				restoreScreen();
			}
			else if (values[i] == 1048)
			{
				restoreCursor();
			}
			else if (values[i] == 1049)
			{
				restoreScreen();
			}
		}
		break;
	case CS_KEYPAD_APP_MODE: //ESC=
	case CS_KEYPAD_NUM_MODE: //ESC>
		//FIXME Not implemented.
		syslog(LOG_INFO, "VT100 Control Sequence: KEYPAD not implemented.");
		break;
	case CS_VT52_KEYPAD_ALT_MODE: //ESC=
	case CS_VT52_KEYPAD_NORMAL_MODE: //ESC>
		//FIXME Not implemented.
		syslog(LOG_INFO, "VT52 Control Sequence: KEYPAD not implemented.");
		break;
	case CS_CHARSET_UK_G0_SET: //ESC(A
		m_currentCharset.set(0, 'A');
		break;
	case CS_CHARSET_ASCII_G0_SET: //ESC(B
		m_currentCharset.set(0, 'B');
		break;
	case CS_CHARSET_SPEC_G0_SET: //ESC(0
		m_currentCharset.set(0, '0');
		break;
	case CS_CHARSET_ALT_G0_SET: //ESC(1
		m_currentCharset.set(0, '1');
		break;
	case CS_CHARSET_ALT_SPEC_G0_SET: //ESC(2
		m_currentCharset.set(0, '2');
		break;
	case CS_CHARSET_USE_G0: //ESCO
		m_currentCharset.select(0);
		break;
	case CS_CHARSET_UK_G1_SET: //ESC)A
		m_currentCharset.set(1, 'A');
		break;
	case CS_CHARSET_ASCII_G1_SET: //ESC)B
		m_currentCharset.set(1, 'B');
		break;
	case CS_CHARSET_SPEC_G1_SET: //ESC)0
		m_currentCharset.set(1, '0');
		break;
	case CS_CHARSET_ALT_G1_SET: //ESC)1
		m_currentCharset.set(1, '1');
		break;
	case CS_CHARSET_ALT_SPEC_G1_SET: //ESC)2
		m_currentCharset.set(1, '2');
		break;
	case CS_CHARSET_USE_G1: //ESCN
		m_currentCharset.select(1);
		break;
	case CS_CHARSET_UK_G2_SET: //ESC*A
		m_currentCharset.set(2, 'A');
		break;
	case CS_CHARSET_ASCII_G2_SET: //ESC*B
		m_currentCharset.set(2, 'B');
		break;
	case CS_CHARSET_SPEC_G2_SET: //ESC*0
		m_currentCharset.set(2, '0');
		break;
	case CS_CHARSET_ALT_G2_SET: //ESC*1
		m_currentCharset.set(2, '1');
		break;
	case CS_CHARSET_ALT_SPEC_G2_SET: //ESC*2
		m_currentCharset.set(2, '2');
		break;
	case CS_CHARSET_USE_G2: //ESCn
		m_currentCharset.select(2);
		break;
	case CS_CHARSET_UK_G3_SET: //ESC+A
		m_currentCharset.set(3, 'A');
		break;
	case CS_CHARSET_ASCII_G3_SET: //ESC+B
		m_currentCharset.set(3, 'B');
		break;
	case CS_CHARSET_SPEC_G3_SET: //ESC+0
		m_currentCharset.set(3, '0');
		break;
	case CS_CHARSET_ALT_G3_SET: //ESC+1
		m_currentCharset.set(3, '1');
		break;
	case CS_CHARSET_ALT_SPEC_G3_SET: //ESC+2
		m_currentCharset.set(3, '2');
		break;
	case CS_CHARSET_USE_G3: //ESCo
		m_currentCharset.select(3);
		break;
	case CS_MARGIN_SET: //ESC[<Top>;<Bottom>r
		values[0] = (values[0] <= 0) ? 1 : values[0];
		values[1] = (values[1] <= 0) ? getDisplayScreenSize().getY() : values[1]; // This likely needs to get adjusted. ~PTM
		values[1] = (values[1] <= values[0]) ? (values[0] + 1) : values[1];
		setMargin(values[0], values[1]);
		cursorHome();
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
	case CS_CNL: //ESC[<Value>E
		for (i=0; i<values[0]; i++)
			moveCursorNextLine();
		break;
	case CS_CPL: //ESC[<Value>F
		for (i=0; i<values[0]; i++)
			moveCursorPreviousLine();
		break;
	case CS_DOUBLE_HEIGHT_LINE_TOP: //ESC#3
	case CS_DOUBLE_HEIGHT_LINE_BOTTOM: //ESC#4
	case CS_SINGLE_WIDTH_LINE: //ESC#5
	case CS_DOUBLE_WIDTH_LINE: //ESC#6
		//FIXME Not implemented. not really needed otoh.
		syslog(LOG_INFO, "VT100 Control Sequence: DOUBLE CELL not implemented. (%i)", nToken);
		break;
	case CS_OSC: // Operating System Controls
		handle_osc(values[0], m_parser->getOSCParameter().c_str());
		break;
	case CS_SCREEN_ALIGNMENT_DISPLAY: //ESC#8
		displayScreenAlignmentPattern();
		break;
	case CS_DEVICE_STATUS_REPORT: //ESC[<Value>;...;<Value>n
		if (extTerminal != NULL && extTerminal->isReady())
		{
			for (i = 0; i < numValues; i++)
			{
				if (values[i] == 5)
				{
					extTerminal->insertData("\x1B[0n");
				}
				else if (values[i] == 6)
				{
					char buf[32];

					sprintf(buf, "\x1B[%d;%dR", getCursorLocation().getY(), getCursorLocation().getX());
					extTerminal->insertData(buf);
				}
			}
		}
		break;
	case CS_DEVICE_ATTR_PRIMARY_REQUEST: //ESC[<Value>c
		if (extTerminal != NULL && extTerminal->isReady())
			extTerminal->insertData("\x1B[?6c");
		break;
	case CS_DEVICE_ATTR_SECONDARY_REQUEST: //ESC[><Value>c
		if (extTerminal != NULL && extTerminal->isReady())
			extTerminal->insertData("\x1B[>0;115;0c");
		break;
	case CS_VT52_IDENTIFY: //ESCZ
		extTerminal->insertData("\x1B/Z");
		break;
	case CS_VT52_SPEC_CHARSET: // ESCF
		syslog(LOG_INFO, "VT52 Control Sequence: ESC F not implemented.");
		break;
	case CS_VT52_ASCII_CHARSET: // ESCG
		// no need to implement until we have ESC F
		// syslog(LOG_INFO, "VT52 Control Sequence: ESG G not implemented.");
		break;
	case CS_TERM_PARAM: //ESC[<Value>;...;<Value>x
		if (values[0]) {
			unsolicited = false;
			extTerminal->insertData("\x1B[3;1;1;112,112;1;0x");
		} else {
			unsolicited = true;
			extTerminal->insertData("\x1B[2;1;1;112,112;1;0x");
		}
		break;
	case CS_TERM_RESET: //ESCc
		syslog(LOG_DEBUG, "VT100 Control Sequence: TERM RESET partially implemented.");
		resetTerminal();
		break;
	case CS_CURSOR_STYLE: // ESC[<Value> SPACE g
		processCursorStyle(values[0]);
		break;
	case CS_DELETE_COLUMN: // ESC[<Value>'~
		deleteColumns(values[0]);
		break;
	case CS_INSERT_COLUMN: // ESC[<Value>'}
		insertColumns(values[0]);
		break;
	case CS_SCROLL_RIGHT: // ESC[<Value>'A
		scrollRight(values[0]);
		break;
	case CS_SCROLL_LEFT: // ESC[<Value>'@
		scrollLeft(values[0]);
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
void VTTerminalState::insertString(const char *sStr, int len, ExtTerminal *extTerminal)
{
	if (sStr == NULL || !len) {
		return;
	}

	pthread_mutex_lock(&m_rwLock);

	m_parser->addInput(sStr, len);

	while (m_parser->next()) {
		if (m_parser->token() != CS_UNKNOWN)
			processControlSeq(m_parser->token(), m_parser->values(), m_parser->numValues(), extTerminal);
		else
			insertChar(m_parser->character());
	}

	pthread_mutex_unlock(&m_rwLock);
}

void VTTerminalState::sendCursorCommand(VTTS_Cursor cursor, ExtTerminal *extTerminal)
{
	if (extTerminal != NULL)
	{
		bool bVT52 = ((getTerminalModeFlags() & TS_TM_VT52) != 0);
		bool bCursorKeys = ((getTerminalModeFlags() & TS_TM_CURSOR_KEYS) != 0);

		if (cursor == VTTS_CURSOR_UP)
		{
			if (bVT52)
			{
				extTerminal->insertData("\033A");
			}
			else if (!bCursorKeys)
			{
				extTerminal->insertData("\x1B[A");
			}
			else
			{
				extTerminal->insertData("\x1BOA");
			}
		}
		else if (cursor == VTTS_CURSOR_DOWN)
		{
			if (bVT52)
			{
				extTerminal->insertData("\033B");
			}
			else if (!bCursorKeys)
			{
				extTerminal->insertData("\x1B[B");
			}
			else
			{
				extTerminal->insertData("\x1BOB");
			}
		}
		else if (cursor == VTTS_CURSOR_RIGHT)
		{
			if (bVT52)
			{
				extTerminal->insertData("\033C");
			}
			else if (!bCursorKeys)
			{
				extTerminal->insertData("\x1B[C");
			}
			else
			{
				extTerminal->insertData("\x1BOC");
			}
		}
		else if (cursor == VTTS_CURSOR_LEFT)
		{
			if (bVT52)
			{
				extTerminal->insertData("\033D");
			}
			else if (!bCursorKeys)
			{
				extTerminal->insertData("\x1B[D");
			}
			else
			{
				extTerminal->insertData("\x1BOD");
			}
		}
	}
}
