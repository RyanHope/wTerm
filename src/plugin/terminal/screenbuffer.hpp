/**
 * This file is part of wTerm.
 * Copyright (C) 2012 Stefan Bühler <stbuehler@web.de>
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

#ifndef SCREENBUFFER_HPP__
#define SCREENBUFFER_HPP__

#include <vector>
#include <deque>

#include <stdint.h>

typedef enum
{
	TS_GM_NONE = 0,
	TS_GM_BOLD = 1,
	TS_GM_UNDERSCORE = 2,
	TS_GM_BLINK = 4,
	TS_GM_NEGATIVE = 8,
	TS_GM_ITALIC = 16,
	TS_GM_MAX
} TSGraphicMode;

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
} TSColor;

struct TSCellGraphicsState
{
	TSColor foregroundColor;
	TSColor backgroundColor;
	int nGraphicsMode;

	bool bold() const { return nGraphicsMode & TS_GM_BOLD; }
	bool underline() const { return nGraphicsMode & TS_GM_UNDERSCORE; }
	bool blink() const { return nGraphicsMode & TS_GM_BLINK; }
	bool negative() const { return nGraphicsMode & TS_GM_NEGATIVE; }
	bool italic() const { return nGraphicsMode & TS_GM_ITALIC; }

	TSCellGraphicsState() : foregroundColor(TS_COLOR_FOREGROUND), backgroundColor(TS_COLOR_BACKGROUND), nGraphicsMode(0) { }
};

typedef uint16_t CellCharacter;

// For each cell on the screen, track its graphics and textual contents:
struct TSCell {
	CellCharacter data;
	TSCellGraphicsState graphics;

	TSCell() : data(0) { }
	TSCell(CellCharacter data) : data(data) { }
	TSCell(TSCellGraphicsState graphics) : data(0), graphics(graphics) { }
	TSCell(CellCharacter data, TSCellGraphicsState graphics) : data(data), graphics(graphics) { }
};

class ScreenBuffer {
public:
	typedef std::vector<TSCell> Line;
	typedef std::deque<Line> Lines;

	typedef Lines::const_iterator LinesIterator;

	typedef Lines Store;

	ScreenBuffer();

	void setScrollbackSize(unsigned int scrollbackSize); /** scrollbackSize + screen height = max # lines */
	void setScrollbackPosition(unsigned int scrollbackPos); /** 0: no scrolling */
	void modifyScrollPosition(int diff);
	void setScreenSize(unsigned int rows, unsigned int columns, unsigned int cursorRow);

	unsigned int scrollbackSize() const { return m_sbSize; }
	unsigned int scrollbackPosition() const { return m_sbPos; }
	unsigned int screenRows() const { return m_rows; }
	unsigned int screenColumns() const { return m_cols; }

	LinesIterator screen_start();
	LinesIterator screen_end();

	void save(Store& store);
	void restore(Store& store);

	// all offset are relative to (1,1) as the top left corner
	void replaceCharacter(unsigned int row, unsigned int col, TSCell c);
	void insertCharacter(unsigned int row, unsigned int col, unsigned int lastCol, TSCell c);
	void deleteCharacters(unsigned int row, unsigned int col, unsigned int count, TSCell fill);

	// inclusive ranges

	void fillLine(unsigned int row, unsigned int colStart, unsigned int colEnd, TSCell fill);
	void fillLines(unsigned int rowStart, unsigned int rowEnd, TSCell fill);

	// scroll lines within a window; count > 0 means empty lines are inserted at the bottom, < 0 at the top
	// if rowStart == 1 and count > 0, the top lines will move up in the Scrollback
	// if scrollbackPos > 0 it will stay relatively on the same line (until it hits the upper limit)
	void scrollLines(unsigned int rowStart, unsigned int rowEnd, int count, TSCell fill);

private:
	Lines::iterator getLine(unsigned int row); // row > 0 && row <= (m_rows+1)! (m_rows+1 returns m_lines.end())

	Lines m_lines;
	unsigned int m_sbSize, m_sbPos; // scrollback
	unsigned int m_rows, m_cols;
};

#endif
