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

#ifndef TERMINALSTATE_HPP__
#define TERMINALSTATE_HPP__

#include "util/databuffer.hpp"
#include "util/point.hpp"

#include <pthread.h>

#include <deque>
#include <vector>

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

	TS_TM_MAX
} TSTermMode_t;

typedef enum
{
	TS_GM_NONE = 0,
	TS_GM_BOLD = 1,
	TS_GM_UNDERSCORE = 2,
	TS_GM_BLINK = 4,
	TS_GM_NEGATIVE = 8,
	TS_GM_ITALIC = 16,
	TS_GM_MAX
} TSGraphicMode_t;

typedef enum
{
	TS_COLOR_BLACK = 0,
	TS_COLOR_RED,
	TS_COLOR_GREEN,
	TS_COLOR_YELLOW,
	TS_COLOR_BLUE,
	TS_COLOR_MAGENTA,
	TS_COLOR_CYAN,
	TS_COLOR_WHITE,
	TS_COLOR_BLACK_BRIGHT,
	TS_COLOR_RED_BRIGHT,
	TS_COLOR_GREEN_BRIGHT,
	TS_COLOR_YELLOW_BRIGHT,
	TS_COLOR_BLUE_BRIGHT,
	TS_COLOR_MAGENTA_BRIGHT,
	TS_COLOR_CYAN_BRIGHT,
	TS_COLOR_WHITE_BRIGHT,
	TS_COLOR_FOREGROUND,
	TS_COLOR_BACKGROUND,
	TS_COLOR_FOREGROUND_BRIGHT,
	TS_COLOR_BACKGROUND_BRIGHT,
	TS_COLOR_MAX
} TSColor_t;

typedef enum
{
	TS_CS_NONE = 0,
	TS_CS_G0_UK,
	TS_CS_G0_ASCII,
	TS_CS_G0_SPEC,
	TS_CS_G0_ALT_STD,
	TS_CS_G0_ALT_SPEC,
	TS_CS_G1_UK,
	TS_CS_G1_ASCII,
	TS_CS_G1_SPEC,
	TS_CS_G1_ALT_STD,
	TS_CS_G1_ALT_SPEC,
	TS_CS_MAX
} TSCharset_t;

typedef struct
{
	TSColor_t foregroundColor;
	TSColor_t backgroundColor;
	int nColumn;
	int nLine;
	int nGraphicsMode;
	TSCharset_t g0charset;
	TSCharset_t g1charset;
} TSLineGraphicsState_t;

typedef enum
{
	TS_GM_OP_SET,
	TS_GM_OP_ADD,
	TS_GM_OP_REMOVE,
	TS_GM_OP_MAX
} TSGraphicsModeOp_t;

/**
 * Terminal state information catered.
 */
class TerminalState
{
protected:
	int m_nTermModeFlags;

	TSLineGraphicsState_t m_defaultGraphicsState;
	TSLineGraphicsState_t m_currentGraphicsState;
	TSLineGraphicsState_t m_savedGraphicsState;

	bool m_bShiftText;

	bool m_shift;

	Point m_cursorLoc; //Bound by the display screen size. Home location is (1, 1).
	Point m_displayScreenSize; //The actual terminal screen size.

	std::deque<DataBuffer *> m_data; //Each list entry represents a line in the console. Holds only printable characters.
	std::vector<TSLineGraphicsState_t *> m_graphicsState;

	pthread_mutexattr_t m_rwLockAttr;
	pthread_mutex_t m_rwLock;

	bool unsolicited;

	int m_nNumBufferLines; //Must at least be the height of the display screen size.
	int m_nTopBufferLine; //The index number in the buffer that corresponds to the first line of the display. Starts at 0.
	int m_nTopMargin;
	int m_nBottomMargin;

	void freeBuffer();
	void freeGraphicsMode();
	void freeGraphicsMode(std::vector<TSLineGraphicsState_t *>::iterator start, std::vector<TSLineGraphicsState_t *>::iterator end);

	void removeGraphicsState(int nColumn, int nLine, bool bTrim, TSLineGraphicsState_t *resetState);
	void removeGraphicsState(int nBeginColumn, int nBeginLine, int nEndColumn, int nEndLine, TSLineGraphicsState_t *resetState);
	int findGraphicsState(int nColumn, int nLine, bool bGetPrev);
	void moveGraphicsState(int nLines, bool bUp);

	void clearBufferLine(int nLine, int nStartX, int nEndX);
	void setBufferTopLine(int nLine);
	void erase(const Point &start, const Point &end);

	Point convertToDisplayLocation(const Point &loc);
	Point boundLocation(const Point &loc);

	virtual bool processNonPrintableChar(char &c);

	std::vector<int> tabs;

public:
	static const char BLANK;

	TerminalState();
	virtual ~TerminalState();

	void setCursorLocation(int nX, int nY);
	void cursorHome();
	Point getCursorLocation();
	Point getDisplayCursorLocation();

	void saveCursor();
	void restoreCursor();
	Point getSavedCursorLocation();
	int getSavedGraphicsModeFlags();

	bool isPrintable(char c);

	void moveCursorUp(int nPos);
	void moveCursorDown(int nPos);
	void moveCursorUp(int nPos, bool bScroll);
	void moveCursorDown(int nPos, bool bScroll);
	void moveCursorForward(int nPos);
	void moveCursorBackward(int nPos);
	void moveCursorNextLine();

	void eraseCurrentLine();
	void eraseCursorToEndOfLine();
	void eraseBeginOfLineToCursor();
	void eraseCursorToEndOfScreen();
	void eraseBeginOfScreenToCursor();
	void eraseScreen();

	void insertChar(char c, bool bAdvanceCursor);
	void insertChar(char c, bool bAdvanceCursor, bool bIgnoreNonPrintable);
	void insertChar(char c, bool bAdvanceCursor, bool bIgnoreNonPrintable, bool bShift);
	void deleteChar(bool bAdvanceCursor, bool bShift);

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
	void setForegroundColor(TSColor_t color);
	TSColor_t getForegroundColor();
	void setBackgroundColor(TSColor_t color);
	TSColor_t getBackgroundColor();
	TSLineGraphicsState_t getCurrentGraphicsState();
	TSLineGraphicsState_t getDefaultGraphicsState();

	void setG0Charset(TSCharset_t charset);
	TSCharset_t getG0Charset();
	void setG1Charset(TSCharset_t charset);
	TSCharset_t getG1Charset();

	int getBufferScreenHeight();
	DataBuffer *getBufferLine(int nLineIndex);
	int getBufferTopLineIndex();
	void setNumBufferLines(int nNumLines);

	void enableShiftText(bool bShift);
	bool isShiftText();

	void lock();
	void unlock();

	void getLineGraphicsState(int nLine, TSLineGraphicsState_t **states, int &nNumStates, int nMaxStates);
	void addGraphicsState(int nColumn, int nLine, TSLineGraphicsState_t& state, TSGraphicsModeOp_t op, bool bTrim);
	void addGraphicsState(int nColumn, int nLine, TSColor_t foregroundColor, TSColor_t backgroundColor, int nGraphicsMode, TSCharset_t g0charset, TSCharset_t g1charset, TSGraphicsModeOp_t op, bool bTrim);

	void resetTerminal();
	void insertLines(int nLines);
	void deleteLines(int nLines);
	void deleteCharacters(int nChars);
	void insertBlanks(int nBlanks);
	void tabForward(int nTabs);
	void tabBackward(int nTabs);
	void setShift(bool shift);
	bool getShift();
};

#endif
