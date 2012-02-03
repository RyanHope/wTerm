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

#ifndef TERMINALSTATE_HPP__
#define TERMINALSTATE_HPP__

#include "screenbuffer.hpp"
#include "util/point.hpp"

#include <pthread.h>

typedef enum
{
	TS_TM_NONE = 0,

	/**
	 * Set to parse only VT52 control sequences. Otherwise, parse only ANSI control sequences.
	 */
	TS_TM_VT52 = 1,

	/**
	 * Set to allow certain keyboard keys to be auto repeated.
	 */
	TS_TM_AUTO_REPEAT = 2,

	/**
	 * Set to print characters to advance to the start of the next line, doing a scroll up if required and permitted.
	 * Otherwise, cause the last character of the line to be replaced.
	 */
	TS_TM_AUTO_WRAP = 4,

	/**
	 * Set to have cursor keys send application functions.
	 * Otherwise, the cursor keys send ANSI cursor control commands.
	 */
	TS_TM_CURSOR_KEYS = 8,

	/**
	 * Set to have the max number of columns (should really be 132).
	 * Otherwise, the max number of columns is 80.
	 */
	TS_TM_COLUMN = 16,


	/**
	 * Set to have cursor position relative to the margins.
	 * Otherwise, the cursor position is always relative to the upper-left corner of the screen.
	 */
	TS_TM_ORIGIN = 32,

	/**
	 * Set to scroll text 6 lines per second. Otherwise, scroll instantly.
	 */
	TS_TM_SCROLL = 64,

	/**
	 * Set to have the line feed return the the first position of the next line.
	 * Otherwise, the cursor will only move to the down to the next line, while retaining in the same column.
	 */
	TS_TM_NEW_LINE = 128,

	/**
	 * Set to make the background white with black characters.
	 * Otherwise, the background is black with white characters.
	 */
	TS_TM_SCREEN = 256,

	/**
	 * Set to show the cursor, otherwise, hide the cursor.
	 */
	TS_TM_CURSOR = 512,

	/**
	 * Set to enable insert mode, otherwise, overwrite.
	 */
	TS_TM_INSERT = 1024,

	/**Set to make backspace key work as backspace, otherwise, delete.
	 *
	 */
	TS_TM_BACKSPACE = 2048,

	TS_TM_MAX
} TSTermMode;

typedef enum
{
	TS_INPUT_F1 = 0,
	TS_INPUT_F2,
	TS_INPUT_F3,
	TS_INPUT_F4,
	TS_INPUT_F5,
	TS_INPUT_F6,
	TS_INPUT_F7,
	TS_INPUT_F8,
	TS_INPUT_F9,
	TS_INPUT_F10,
	TS_INPUT_F11,
	TS_INPUT_F12,
	TS_INPUT_MAX
} TSInput;

struct CharsetState {
	/* charset values:
	 * 'A': UK
	 * 'B': ASCII
	 * '0': SPEC
	 * '1': ALT
	 * '2': ALT SPEC
	 */

	/* we only handle "GL" charset (0x20-0x7F), not "GR" (0xA0-0xFF) */
	unsigned char charsets[4], charset;
	unsigned int charset_ndx;

	CharsetState() : charset('B'), charset_ndx(0) {
		charsets[0] = charsets[1] = charsets[2] = charsets[3] = 'B';
	}

	void reset() {
		charset_ndx = 0;
		charset = charsets[0] = charsets[1] = charsets[2] = charsets[3] = 'B';
	}

	void select(unsigned int ndx) {
		charset_ndx = ndx & 0x3;
		charset = charsets[ndx];
	}

	void set(unsigned int ndx, unsigned char charset) {
		charsets[ndx & 0x3] = charset;
		this->charset = charsets[charset_ndx];
	}
};

typedef enum
{
	TS_CURSOR_STYLE_BLOCK_BLINK = 0,
	TS_CURSOR_STYLE_BLOCK_STEADY = 1,
	TS_CURSOR_STYLE_UNDERLINE_BLINK = 2,
	TS_CURSOR_STYLE_UNDERLINE_STEADY = 3,
	TS_CURSOR_STYLE_VERTICALLINE_BLINK = 4,
	TS_CURSOR_STYLE_VERTICALLINE_STEADY = 5
} TSCursorStyle;

typedef enum
{
	TS_GM_OP_SET,
	TS_GM_OP_ADD,
	TS_GM_OP_REMOVE,
	TS_GM_OP_MAX
} TSGraphicsModeOp;

// Say that a line is a vector of cells.
// For now, they don't have to be the same size as the screen,
// in which case the unrepresented cells are empty using our bg color.
typedef std::vector<TSCell> TSLine;

/**
 * Terminal state information catered.
 */
class TerminalState
{
protected:
	struct TSScreen {
		ScreenBuffer::Store screenStore;
		Point savedCursorLoc;
	};


	int m_nTermModeFlags;

	TSCursorStyle m_cursorStyle;

	TSGraphicsState m_currentGraphicsState;
	TSGraphicsState m_savedGraphicsState;
	Point m_savedCursorLoc;

	CharsetState m_currentCharset;
	unsigned char m_savedCharset; // only save value of selected charset, not slot/index

	Point m_cursorLoc; //Bound by the display screen size. Home location is (1, 1).
	Point m_displayScreenSize; //The actual terminal screen size.

	ScreenBuffer m_screenBuffer;

	pthread_mutexattr_t m_rwLockAttr;
	pthread_mutex_t m_rwLock;

	TSScreen m_savedScreen;

	bool unsolicited;

	int m_nTopMargin;
	int m_nBottomMargin;

	Point convertToDisplayLocation(const Point &loc);

	std::vector<int> tabs;

public:
	static const CellCharacter BLANK;

	TerminalState();
	virtual ~TerminalState();

	ScreenBuffer::LinesIterator screen_start() { return m_screenBuffer.screen_start(); }
	ScreenBuffer::LinesIterator screen_end() { return m_screenBuffer.screen_end(); }

	void setCursorLocation(int nX, int nY);
	void cursorHome();
	Point getCursorLocation();
	Point getDisplayCursorLocation();

	void saveScreen();
	void restoreScreen();

	void saveCursor();
	void restoreCursor();
	Point getSavedCursorLocation();
	int getSavedGraphicsModeFlags();

	static bool isPrintable(CellCharacter c);

	void moveCursorUp(int nPos, bool bScroll = false);
	void moveCursorDown(int nPos, bool bScroll = false);
	void moveCursorForward(int nPos);
	void moveCursorBackward(int nPos);
	void moveCursorNextLine();
	void moveCursorPreviousLine();

	void eraseCurrentLine();
	void eraseCursorToEndOfLine();
	void eraseBeginOfLineToCursor();
	void eraseCursorToEndOfScreen();
	void eraseBeginOfScreenToCursor();
	void eraseScreen();
	void eraseCharacters(int nChars);

	void insertChar(CellCharacter c);

	void setDisplayScreenSize(int nWidth, int nHeight);
	Point getDisplayScreenSize();

	void displayScreenAlignmentPattern();

	void setMargin(int nTop, int nBottom);
	int getTopMargin();
	int getBottomMargin();

	void setTerminalModeFlags(int nFlags);
	void addTerminalModeFlags(int nFlags);
	void removeTerminalModeFlags(int nFlags);
	int getTerminalModeFlags();

	void setGraphicsModeFlags(int nFlags);
	void addGraphicsModeFlags(int nFlags);
	void removeGraphicsModeFlags(int nFlags);
	int getGraphicsModeFlags();
	void setForegroundColor(TSColor color);
	TSColor getForegroundColor();
	void setBackgroundColor(TSColor color);
	TSColor getBackgroundColor();
	TSGraphicsState getCurrentGraphicsState();

	CellCharacter applyCharset(CellCharacter cChar);

	void lock();
	void unlock();

	TSCell getEmptyCell() {
		return TSCell(BLANK);
	}

	void resetTerminal();
	void insertLines(int nLines);
	void deleteLines(int nLines);
	void deleteCharacters(int nChars);
	void insertBlanks(int nBlanks);
	void tabForward(unsigned int nTabs);
	void tabBackward(int nTabs);

	void setScrollBufferLines(int lines);
	int getScrollBufferLines();
	void setScrollOffset(int offset);
	int getScrollOffset();

	void forwardIndex();
	void backIndex();

	void handle_osc(int value, const char *txt);

	void processCursorStyle(int style);
	void setCursorStyle(TSCursorStyle style);
	TSCursorStyle getCursorStyle();

	void insertColumns(int value);
	void deleteColumns(int value);

	void scrollLeft(int value);
	void scrollRight(int value);
};

#endif
