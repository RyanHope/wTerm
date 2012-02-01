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

#include "screenbuffer.hpp"
#include "util/utils.hpp"

#include <algorithm>
#include <limits>

ScreenBuffer::ScreenBuffer()
 : m_sbSize(0), m_sbPos(0), m_rows(0), m_cols(0) {
}

void ScreenBuffer::setScrollbackSize(unsigned int scrollbackSize) {
	if (m_sbSize == scrollbackSize) return;

	if (m_sbSize > scrollbackSize) {
		m_sbSize = scrollbackSize;
		return;
	}

	m_sbSize = scrollbackSize;
	unsigned int maxLines = m_sbSize + m_rows;
	unsigned int haveLines = m_lines.size();
	if (haveLines > maxLines) {
		m_lines.erase(m_lines.begin(), m_lines.begin() + (haveLines - maxLines));
		haveLines = maxLines;
	}

	if (m_sbPos > haveLines - m_rows) {
		m_sbPos = haveLines - m_rows;
		// Notification: new scroll pos
	}
}

void ScreenBuffer::setScrollbackPosition(unsigned int scrollbackPos) {
	scrollbackPos = std::min(scrollbackPos, m_sbSize);

	if (m_sbPos == scrollbackPos) return;

	unsigned int haveLines = m_lines.size();

	scrollbackPos = std::min(scrollbackPos, haveLines - m_rows);

	if (m_sbPos == scrollbackPos) return;

	m_sbPos = scrollbackPos;
	// Notification: new scroll pos
}

void ScreenBuffer::modifyScrollPosition(int diff) {
	if (diff < 0) {
		if (-(int) m_sbPos >= diff) {
			setScrollbackPosition(0);
		} else {
			setScrollbackPosition(m_sbPos + diff);
		}
	} else {
		setScrollbackPosition(m_sbPos + diff);
	}
}


void ScreenBuffer::setScreenSize(unsigned int rows, unsigned int columns, unsigned int cursorRow) {
	if (columns != m_cols) {
		for (Lines::iterator l = m_lines.begin(), e = m_lines.end(); l != e; ++l) {
			l->resize(columns);
		}
	}
	m_cols = columns;

	if (rows > m_rows) {
		m_lines.insert(m_lines.end(), rows - m_rows, Line(m_cols));
		m_rows = rows;
	} else if (rows > m_rows) {
		/* delete lines below cursor */
		unsigned int del_lines = m_rows - rows;
		if (cursorRow <= m_rows) {
			/* cursorRow should always be <= m_rows */
			del_lines = std::min(m_rows - cursorRow, del_lines);
		}
		m_lines.erase(m_lines.end() - del_lines, m_lines.end());

		/* the lines we should have deleted but moved up instead because of the cursor */
		unsigned int scrolled_up = (m_rows - rows) - del_lines;

		m_rows = rows;

		/* perhaps we still have too much lines, remove them */
		unsigned int maxLines = m_sbSize + m_rows;
		unsigned int haveLines = m_lines.size();
		if (haveLines > maxLines) {
			m_lines.erase(m_lines.begin(), m_lines.begin() + (haveLines - maxLines));
			haveLines = maxLines;
		}

		modifyScrollPosition(scrolled_up);
	}

	// Notification: total refresh
}

ScreenBuffer::LinesIterator ScreenBuffer::screen_start() {
	unsigned int haveLines = m_lines.size();

	if (m_sbPos > haveLines - m_rows) {
		syslog(LOG_ERR, "scroll position out of bounds!");
		m_sbPos = haveLines - m_rows;
		// Notification: new scroll pos
	}

	return m_lines.end() - m_rows - m_sbPos;
}

ScreenBuffer::LinesIterator ScreenBuffer::screen_end() {
	return m_lines.end() - m_sbPos;
}

void ScreenBuffer::save(Store& store) {
	store = m_lines;
}

void ScreenBuffer::restore(Store& store) {
	m_lines = store;

	// make sure dimensions match

	unsigned int maxLines = m_rows + m_sbSize;
	unsigned int haveLines = m_lines.size();

	if (haveLines > maxLines) {
		// scrollback overflow
		m_lines.erase(m_lines.begin(), m_lines.begin() + (haveLines - maxLines));
		haveLines = maxLines;
	}

	if (haveLines < m_rows) {
		// not enough lines for complete screen
		m_lines.insert(m_lines.end(), m_rows - haveLines, Line(m_cols));
		haveLines = m_rows;
	}

	if (m_sbPos > haveLines - m_rows) {
		m_sbPos = haveLines - m_rows;
		// Notification: new scroll pos
	}

	if (m_lines.front().size() != m_cols) {
		for (Lines::iterator l = m_lines.begin(), e = m_lines.end(); l != e; ++l) {
			l->resize(m_cols);
		}
	}
}

void ScreenBuffer::replaceCharacter(unsigned int row, unsigned int col, TSCell c) {
	if (0 == row || row > m_rows || 0 == col || col > m_cols) return;

	Line &l(*getLine(row));
	l[col-1] = c;
}

void ScreenBuffer::insertCharacter(unsigned int row, unsigned int col, unsigned int lastCol, TSCell c) {
	if (0 == row || row > m_rows || 0 == col || col > m_cols || col > lastCol) return;

	Line &l(*getLine(row));
	l.insert(l.begin() + col - 1, 1, c);
	l.erase(l.begin() + lastCol);
}

void ScreenBuffer::deleteCharacters(unsigned int row, unsigned int col, unsigned int count, TSCell fill2) {
	if (0 == row || row > m_rows || 0 == col || col > m_cols) return;

	count = std::min(count, m_cols - col + 1);

	Line &l(*getLine(row));
	TSCell fill(l[m_cols-1]);
	fill.data = ' ';

	Line::iterator c = l.begin() + (col-1);
	l.erase(c, c + count);
	l.insert(l.end(), count, fill);
}

void ScreenBuffer::fillLine(unsigned int row, unsigned int colStart, unsigned int colEnd, TSCell fill) {
	if (0 == row || row > m_rows || colStart > m_cols || colEnd < 1) return;

	colStart = std::max(1u, colStart);
	colEnd = std::min(m_cols, colEnd);
	if (colStart > colEnd) return;

	Line &l(*getLine(row));
	std::fill(l.begin() + colStart - 1, l.begin() + colEnd, fill);
}

void ScreenBuffer::fillLines(unsigned int rowStart, unsigned int rowEnd, TSCell fill) {
	rowStart = std::max(1u, rowStart);
	rowEnd = std::min(m_rows, rowEnd);
	if (rowStart > rowEnd) return;

	for (Lines::iterator l = getLine(rowStart), e = getLine(rowEnd + 1); l != e; ++l) {
		std::fill(l->begin(), l->end(), fill);
	}
}

void ScreenBuffer::scrollLines(unsigned int rowStart, unsigned int rowEnd, int count, TSCell fill) {
	if (0 == count || std::numeric_limits<int>::min() == count) return;
	rowStart = std::max(1u, rowStart);
	rowEnd = std::min(m_rows, rowEnd);
	if (rowStart > rowEnd) return;

	if (count > 0) {
		count = std::min<int>(rowEnd - rowStart + 1, count);
		if (rowStart > 1) {
			m_lines.insert(getLine(rowEnd+1), count, Line(m_cols, fill));
			Lines::iterator top(getLine(rowStart));
			m_lines.erase(top - count, top);
		} else {
			// with scrollback
			m_lines.insert(getLine(rowEnd+1), count, Line(m_cols, fill));
			unsigned int haveLines = m_lines.size(), maxLines = m_sbSize + m_rows;
			if (haveLines > maxLines) {
				m_lines.erase(m_lines.begin(), m_lines.begin() + (haveLines - maxLines));
				haveLines = maxLines;
			}
			if (m_sbPos > 0) m_sbPos = std::max(haveLines - m_rows, m_sbPos + count);
		}
	} else {
		count = std::min<int>(rowEnd - rowStart + 1, -count);

		m_lines.insert(getLine(rowStart), count, Line(m_cols, fill));
		Lines::iterator bottom(getLine(rowEnd + 1));
		m_lines.erase(bottom - count, bottom);
	}

}

ScreenBuffer::Lines::iterator ScreenBuffer::getLine(unsigned int row) {
	assert(row > 0 && row <= m_rows + 1);
	return (m_lines.end() - (m_rows - row + 1));
}
