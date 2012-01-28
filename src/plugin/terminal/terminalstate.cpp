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

#include "terminalstate.hpp"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <PDL.h>

const CellCharacter TerminalState::BLANK = ' ';

TerminalState::TerminalState()
{
	m_nTermModeFlags = 0;

	unsolicited = false;

	m_nTopBufferLine = 0;
	m_nNumBufferLines = 0;
	m_nScrollBufferLines = 0;
	m_nTopMargin = 0;
	m_nBottomMargin = 0;
	m_nScollOffset = 0;

	memset(&m_defaultGraphicsState, 0, sizeof(m_defaultGraphicsState));
	m_defaultGraphicsState.foregroundColor = TS_COLOR_FOREGROUND;
	m_defaultGraphicsState.backgroundColor = TS_COLOR_BACKGROUND;
	m_defaultGraphicsState.charset = 'B';
	m_defaultGraphicsState.charset_ndx = 0;
	memset(m_defaultGraphicsState.charsets, 'B', sizeof(m_defaultGraphicsState.charsets));

	memcpy(&m_currentGraphicsState, &m_defaultGraphicsState, sizeof(m_currentGraphicsState));
	memcpy(&m_savedGraphicsState, &m_defaultGraphicsState, sizeof(m_savedGraphicsState));

	pthread_mutexattr_init(&m_rwLockAttr);
	pthread_mutexattr_settype(&m_rwLockAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_rwLock, &m_rwLockAttr);
}

TerminalState::~TerminalState()
{
	pthread_mutex_lock(&m_rwLock);

	freeBuffer();

	pthread_mutex_unlock(&m_rwLock);

	pthread_mutexattr_destroy(&m_rwLockAttr);
	pthread_mutex_destroy(&m_rwLock);
}

void TerminalState::freeBuffer()
{
	pthread_mutex_lock(&m_rwLock);

	m_data.clear();

	pthread_mutex_unlock(&m_rwLock);
}

bool TerminalState::isPrintable(CellCharacter c)
{
	return !((c >= 0 && c < 32) || c == 127);
}

/**
 * Erases the data from a specific line in the data buffer. Start index must be
 * less than end index.
 */
void TerminalState::clearBufferLine(int nLine, int nStart, int nEnd, TSCell & eraseTo)
{
	pthread_mutex_lock(&m_rwLock);

	if (nLine >= 0 && (unsigned int) nLine < m_data.size())
	{
		TSLine &line = m_data[nLine];

		if (nStart < 0)
		{
			nStart = 0;
		}

		if (nEnd < 0)
		{
			nEnd = 0;
		}

		if (nStart <= nEnd)
		{
			// If our line buffer isn't long enough, extend it
			// TODO: I'm not sure we don't wanna just make all lines
			// always the full size, but will revisit this later.
			if (line.size() <= (unsigned int) nEnd) line.resize(nEnd + 1);

			// Set the desired region to 'eraseTo'...
			// NOTE: Fill doesn't set the element pointed to by the 2nd iterator
			std::fill(line.begin()+nStart,line.begin()+ nEnd + 1, eraseTo);
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Relocates the buffer line index that corresponds with the first line of the display.
 * If the specified line is out of bounds (e.g. nLine < 0) then the buffer is reshaped.
 */
void TerminalState::setBufferTopLine(int nLine)
{
	pthread_mutex_lock(&m_rwLock);

	// TODO: Rewrite this to copy less

	//Retain buffer outside of scroll region.
	std::deque<TSLine> tmp;
	int nSizeTopMargin = getTopMargin() - 1;
	int nSizeBottomMargin = getDisplayScreenSize().getY() - getBottomMargin();

	for (int i = 0; i < nSizeTopMargin; i++)
	{
		tmp.push_back(m_data[m_nTopBufferLine + i]);
		m_data[m_nTopBufferLine + i].clear();
	}

	for (int i = 0; i < nSizeBottomMargin; i++)
	{
		tmp.push_back(m_data[m_nTopBufferLine + i + getBottomMargin()]);
		m_data[m_nTopBufferLine + i + getBottomMargin()].clear();
	}

	//Move buffer up
	if (nLine < 0)
	{
		nLine = (-1) * nLine;

		//Insert empty lines to the top of the buffer.
		for (int i = 0; i < nLine; i++)
		{
			m_data.push_front(TSLine());
		}

		m_nTopBufferLine = 0;
	}
	else if ((unsigned int) nLine >= m_data.size())
	{
		nLine = nLine - m_data.size() + 1;

		//Insert empty lines to the end of the buffer.
		for (int i = 0; i < nLine; i++)
		{
			m_data.push_back(TSLine());
		}

		m_nTopBufferLine = (m_data.size() - 1);
	}
	else
	{
		m_nTopBufferLine = nLine;
	}

	//Validate and fixup buffer.
	setNumBufferLines(m_nNumBufferLines);

	//Restore buffer outside scroll region.
	// TODO: Do this more directly
	for (int i = 0; i < nSizeTopMargin; i++)
	{
		m_data[m_nTopBufferLine + i] = tmp.front();
		tmp.pop_front();
	}

	for (int i = 0; i < nSizeBottomMargin; i++)
	{
		m_data[m_nTopBufferLine + i + getBottomMargin()] = tmp.front();
		tmp.pop_front();
	}

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Erases characters from start location to end location of the display.
 * Does not affect cursor location.
 * If origin mode is set, the location is relative to the origin.
 * Coordinates are bound within the display screen.
 */
void TerminalState::erase(const Point &start, const Point &end)
{
	pthread_mutex_lock(&m_rwLock);

	Point displayStart = boundLocation(start);
	Point displayEnd = boundLocation(end);

	//True if going forward
	bool bDirection = (displayEnd.getY() == displayStart.getY() && displayEnd.getX() >= displayStart.getX())
		|| (displayEnd.getY() > displayStart.getY());

	int nStartX = bDirection ? displayStart.getX() : displayEnd.getX();
	int nEndX = bDirection ? displayEnd.getX() : displayStart.getX();
	int nStartLine = bDirection ? displayStart.getY() : displayEnd.getY();
	int nEndLine = bDirection ? displayEnd.getY() : displayStart.getY();

	// When erasing, set the erase chars to use our fg/bg, but
	// reset the character attributes.
	TSCell eraseTo = getEmptyCell();
	eraseTo.graphics.foregroundColor = m_currentGraphicsState.foregroundColor;
	eraseTo.graphics.backgroundColor = m_currentGraphicsState.backgroundColor;

	//Change index variables to be relative to the buffer.
	nStartX -= 1;
	nEndX -= 1;
	nStartLine += (m_nTopBufferLine - 1);
	nEndLine += (m_nTopBufferLine - 1);

	for (int i = nStartLine; i <= nEndLine; i++)
	{
		//Process first line.
		if (i == nStartLine)
		{
			if (nStartLine == nEndLine)
			{
				clearBufferLine(i, nStartX, nEndX, eraseTo);
			}
			else
			{
				clearBufferLine(i, nStartX, m_displayScreenSize.getX() - 1, eraseTo);
			}
		}
		//Process last line.
		else if (i == nEndLine)
		{
			clearBufferLine(i, 0, nEndX, eraseTo);
		}
		//Clear lines in between.
		else
		{
			clearBufferLine(i, 0, m_displayScreenSize.getX() - 1, eraseTo);
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseCurrentLine()
{
	pthread_mutex_lock(&m_rwLock);

	erase(Point(1, m_cursorLoc.getY()), Point(m_displayScreenSize.getX(), m_cursorLoc.getY()));

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseCursorToEndOfLine()
{
	pthread_mutex_lock(&m_rwLock);

	erase(m_cursorLoc, Point(m_displayScreenSize.getX(), m_cursorLoc.getY()));

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseBeginOfLineToCursor()
{
	pthread_mutex_lock(&m_rwLock);

	erase(Point(1, m_cursorLoc.getY()), m_cursorLoc);

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseCursorToEndOfScreen()
{
	pthread_mutex_lock(&m_rwLock);

	erase(m_cursorLoc, m_displayScreenSize);

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseBeginOfScreenToCursor()
{
	pthread_mutex_lock(&m_rwLock);

	erase(Point(1, 1), m_cursorLoc);

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseScreen()
{
	pthread_mutex_lock(&m_rwLock);

	erase(Point(1, 1), m_displayScreenSize);

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::deleteCharacters(int nChars)
{
	pthread_mutex_lock(&m_rwLock);

	Point cursor = convertToDisplayLocation(m_cursorLoc);
	// TODO: Is this right?
	unsigned int nY = cursor.getY() - 1;
	unsigned int nX = cursor.getX() - 1;
	unsigned int nEnd = nX + nChars;
	if (nEnd > m_data[nY].size())
		nEnd = m_data[nY].size();
	for(unsigned int i = nX; i < nEnd; ++i)
		m_data[nY].erase(m_data[nY].begin() + nX);

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::insertLines(int nLines)
{
	pthread_mutex_lock(&m_rwLock);

	int curLine = convertToDisplayLocation(m_cursorLoc).getY();

	// Insert lines into the scroll buffer region:

	if (curLine >= m_nTopMargin && curLine < m_nBottomMargin) {
		for (int i=0; i<nLines; i++) {
			if (curLine-1+i < m_nBottomMargin) {
				m_data.erase(m_data.begin()+getBufferTopLineIndex() + m_nBottomMargin-1);
				m_data.insert(m_data.begin()+getBufferTopLineIndex() + curLine-1+i, TSLine());
			}
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::deleteLines(int nLines)
{
	pthread_mutex_lock(&m_rwLock);

	int curLine = convertToDisplayLocation(m_cursorLoc).getY();

	// Delete lines from the scroll buffer region:
	if (curLine >= m_nTopMargin && curLine < m_nBottomMargin) {
		for (int i=0; i<nLines; i++) {
			if (curLine-1+i < m_nBottomMargin) {
				m_data.erase(m_data.begin()+getBufferTopLineIndex() + curLine-1+i);
				m_data.insert(m_data.begin()+getBufferTopLineIndex() + m_nBottomMargin-1, TSLine());
			}
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Moves cursor up the number of specified position.
 * Does not scroll display.
 */
void TerminalState::moveCursorUp(int nPos)
{
	pthread_mutex_lock(&m_rwLock);

	moveCursorUp(nPos, false);

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Moves cursor down the number of specified position.
 * Does not scroll display.
 */
void TerminalState::moveCursorDown(int nPos)
{
	pthread_mutex_lock(&m_rwLock);

	moveCursorDown(nPos, false);

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::moveCursorUp(int nPos, bool bScroll)
{
	pthread_mutex_lock(&m_rwLock);

	int nY = getDisplayCursorLocation().getY();

	if (nPos >= 0)
	{
		nY -= nPos;

		if (nY < m_nTopMargin)
		{
			if (bScroll)
			{
				setBufferTopLine(m_nTopBufferLine - m_nTopMargin + nY);
			}

			nY = m_nTopMargin;
		}

		if ((m_nTermModeFlags & TS_TM_ORIGIN) > 0)
		{
			nY -= (m_nTopMargin - 1);

			if (nY < 1)
			{
				nY = 1;
			}
		}

		setCursorLocation(m_cursorLoc.getX(), nY);
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::moveCursorDown(int nPos, bool bScroll)
{
	pthread_mutex_lock(&m_rwLock);

	int nY = getDisplayCursorLocation().getY();

	if (nPos >= 0)
	{
		nY += nPos;

		if (nY > m_nBottomMargin)
		{
			if (bScroll)
			{
				setBufferTopLine(m_nTopBufferLine + nY - m_nBottomMargin);
			}

			nY = m_nBottomMargin;
		}

		if ((m_nTermModeFlags & TS_TM_ORIGIN) > 0)
		{
			nY -= (m_nTopMargin - 1);

			if (nY < 1)
			{
				nY = 1;
			}
		}

		setCursorLocation(m_cursorLoc.getX(), nY);
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::moveCursorForward(int nPos)
{
	pthread_mutex_lock(&m_rwLock);

	int newX = m_cursorLoc.getX() + nPos;
	int maxX = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;
	newX = (newX>maxX) ? maxX : newX;

	if (nPos >= 0)
	{
		setCursorLocation(newX, m_cursorLoc.getY());
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::moveCursorBackward(int nPos)
{
	pthread_mutex_lock(&m_rwLock);

	int maxX = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;

	if (nPos >= 0)
	{
		if (m_cursorLoc.getX()>maxX)
			setCursorLocation(maxX - nPos, m_cursorLoc.getY());
		else
			setCursorLocation(m_cursorLoc.getX() - nPos, m_cursorLoc.getY());
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::moveCursorPreviousLine()
{
	pthread_mutex_lock(&m_rwLock);

	moveCursorUp(1, true);
	moveCursorBackward(m_cursorLoc.getX() - 1);

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::moveCursorNextLine()
{
	pthread_mutex_lock(&m_rwLock);

	moveCursorDown(1, true);
	moveCursorBackward(m_cursorLoc.getX() - 1);

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Gets the virtual screen height that the buffer can store.
 */
int TerminalState::getBufferScreenHeight()
{
	pthread_mutex_lock(&m_rwLock);

	int nResult = (m_data.size() - m_nTopBufferLine);

	pthread_mutex_unlock(&m_rwLock);

	return nResult;
}

/**
 * Gets a line of the data buffer that represents a line of the terminal.
 * Caller to this method should never modify the contents.
 * Returns NULL if the specified line is out of bounds.
 */
TSLine * TerminalState::getBufferLine(int nLineIndex)
{
	pthread_mutex_lock(&m_rwLock);

	TSLine *buffer = NULL;

	if (nLineIndex >= 0 && (unsigned int) nLineIndex < m_data.size())
	{
		buffer = &m_data[nLineIndex];
	}

	pthread_mutex_unlock(&m_rwLock);

	return buffer;
}

/**
 * Gets the index of the line in the buffer that represents the first line of the display.
 */
int TerminalState::getBufferTopLineIndex()
{
	return m_nTopBufferLine;
}

int TerminalState::getNumBufferLines()
{
	return m_nNumBufferLines;
}


/**
 * Bound the location within the display window.
 * If the origin mode is set, the location is converted to be relative to the display first.
 * The returned location will be within (1, 1) and (<display window width>, <display window height>).
 */
Point TerminalState::boundLocation(const Point &loc)
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = convertToDisplayLocation(loc);

	if (displayLoc.getX() < 1)
	{
		displayLoc.setX(1);
	}

	if (displayLoc.getY() < 1)
	{
		displayLoc.setY(1);
	}

	if (displayLoc.getX() > m_displayScreenSize.getX())
	{
		displayLoc.setX(m_displayScreenSize.getX());
	}

	if (displayLoc.getY() > m_displayScreenSize.getY())
	{
		displayLoc.setY(m_displayScreenSize.getY());
	}

	pthread_mutex_unlock(&m_rwLock);

	return displayLoc;
}

/**
 * Converts the location to be relative to the display screen rather than the origin.
 * Does nothing if origin mode is not set.
 */
Point TerminalState::convertToDisplayLocation(const Point &loc)
{
	pthread_mutex_lock(&m_rwLock);

	Point result = loc;

	if ((m_nTermModeFlags & TS_TM_ORIGIN) > 0)
	{
		result = Point(loc.getX(), loc.getY() + m_nTopMargin - 1);
	}

	pthread_mutex_unlock(&m_rwLock);

	return result;
}

void TerminalState::setScollOffset(int offset)
{
	pthread_mutex_lock(&m_rwLock);

	if (getBufferTopLineIndex() > 0)
	{
		if (offset < 0)
			m_nScollOffset = 0;
		else if (offset > getBufferTopLineIndex())
			m_nScollOffset = getBufferTopLineIndex();
		else
			m_nScollOffset = offset;
	}

	pthread_mutex_unlock(&m_rwLock);
}

int TerminalState::getScollOffset()
{
	return m_nScollOffset;
}

void TerminalState::setScrollBufferLines(int lines)
{
	pthread_mutex_lock(&m_rwLock);

	if (lines < 0) return;

	m_nScrollBufferLines = lines;
	setNumBufferLines(m_displayScreenSize.getY()+m_nScrollBufferLines);

	setScollOffset(0);

	pthread_mutex_unlock(&m_rwLock);
}

int TerminalState::getScrollBufferLines()
{
	return m_nScrollBufferLines;
}

/**
 * Sets the number of lines the buffer can hold.
 * Minimum number of lines is the display screen height.
 */
void TerminalState::setNumBufferLines(int nNumLines)
{
	pthread_mutex_lock(&m_rwLock);

	//Minimum number of lines is the display screen height.
	if (nNumLines < m_displayScreenSize.getY() || nNumLines < 1)
	{
		nNumLines = m_displayScreenSize.getY();
	}

	//Makes sure the virtual buffer screen can at least hold the display screen size.
	while (getBufferScreenHeight() < m_displayScreenSize.getY())
	{
		m_data.push_back(TSLine());
	}

	m_nNumBufferLines = nNumLines;

	//Removes excess old buffered lines.
	while (m_nTopBufferLine > 0 && (unsigned int) m_nNumBufferLines < m_data.size())
	{
		m_data.pop_front();

		--m_nTopBufferLine;
	}

	//Removes excess overflow buffered lines.
	while ((int) m_data.size() > m_nNumBufferLines && getBufferScreenHeight() > m_displayScreenSize.getY())
	{
		m_data.pop_back();
	}

	//Expand buffer if necessary.
	while ((int) m_data.size() < m_nNumBufferLines)
	{
		m_data.push_back(TSLine());
	}

	assert((int) m_data.size() == m_nNumBufferLines);

	int nScreenWidth = m_displayScreenSize.getX();

	//Trim data.
	for (unsigned int i = 0; i < m_data.size(); i++)
	{
		// Truncate all characters beyond screen boundary
		if (m_data[i].size() > (unsigned int) nScreenWidth)
			m_data[i].resize(nScreenWidth);
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::cursorHome()
{
	pthread_mutex_lock(&m_rwLock);

	setCursorLocation(1,1);

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Minimum cursor position is (1, 1).
 * Maximum cursor position will always be limited by the visible screen size.
 * If origin mode is set, the coordinates are relative to the origin.
 */
void TerminalState::setCursorLocation(int nX, int nY)
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = convertToDisplayLocation(Point(nX, nY));

	nX = displayLoc.getX();
	nY = displayLoc.getY();

	if (nX < 1)
	{
		nX = 1;
	}

	if (nY < 1)
	{
		nY = 1;
	}

	if (displayLoc.getX() > m_displayScreenSize.getX())
	{
		nX = m_displayScreenSize.getX();
	}

	if (displayLoc.getY() > m_displayScreenSize.getY())
	{
		nY = m_displayScreenSize.getY();
	}

	if ((m_nTermModeFlags & TS_TM_ORIGIN) > 0)
	{
		if (nY > m_nBottomMargin)
		{
			nY = m_nBottomMargin;
		}

		nY = nY - m_nTopMargin + 1;

		if (nY < 1)
		{
			nY = 1;
		}
	}

	m_cursorLoc.setLocation(nX, nY);

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Gets the cursor location relative to the margin if origin mode is set.
 * Otherwise, the cursor location is relative to the upper left corner of the display.
 */
Point TerminalState::getCursorLocation()
{
	return m_cursorLoc;
}

/**
 * Gets the cursor location relative to the upper left corner of the visible screen.
 * If origin mode is set, then offset is added to the returned cursor location.
 */
Point TerminalState::getDisplayCursorLocation()
{
	pthread_mutex_lock(&m_rwLock);

	Point result = convertToDisplayLocation(m_cursorLoc);

	pthread_mutex_unlock(&m_rwLock);

	return result;
}

/**
 * Sets the display size. Minimum is 1x1.
 */
void TerminalState::setDisplayScreenSize(int nWidth, int nHeight)
{
	pthread_mutex_lock(&m_rwLock);

	if (nWidth < 1)
	{
		nWidth = 1;
	}

	if (nHeight < 1)
	{
		nHeight = 1;
	}

	m_displayScreenSize.setLocation(nWidth, nHeight);

	//Reset affected attributes to fix cases where location is out of bounds after setting the display.
	setCursorLocation(m_cursorLoc.getX(), m_cursorLoc.getY());

	setNumBufferLines(nHeight+m_nScrollBufferLines);

	setMargin(m_nTopMargin, m_nBottomMargin);

	pthread_mutex_unlock(&m_rwLock);
}

Point TerminalState::getDisplayScreenSize()
{
	return m_displayScreenSize;
}

void TerminalState::insertBlanks(int nBlanks)
{
	pthread_mutex_lock(&m_rwLock);

	Point cursor = convertToDisplayLocation(m_cursorLoc);

	int nX = cursor.getX() - 1;
	int nY = cursor.getY() - 1;

	TSLine * line = getBufferLine(nY);
	TSCell empty = getEmptyCell(); // TODO: What graphics?
	for (int i=0; i<nBlanks; i++)
		line->insert(line->begin()+nX, empty);

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::displayScreenAlignmentPattern() {
	int nCols = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;
	int nRows = m_displayScreenSize.getY();
	int modeFlags = m_nTermModeFlags;
	m_nTermModeFlags &= ~TS_TM_INSERT;
	for (int r=0;r<nRows;r++) {
		for (int c=0;c<nCols;c++) {
			setCursorLocation(c+1,r+1);
			insertChar(69, false, true); // 'E'
		}
	}
	m_nTermModeFlags = modeFlags;
	setCursorLocation(1,1);
}

/**
 * Sets the margin to be used in origin mode.
 * Setting the margin will reset the cursor to its home position
 * regardless if origin mode is enabled.
 */
void TerminalState::setMargin(int nTop, int nBottom)
{
	pthread_mutex_lock(&m_rwLock);

	if (nTop < 1)
	{
		nTop = 1;
	}

	if (nBottom < 1)
	{
		nBottom = 1;
	}

	if (nTop >= m_displayScreenSize.getY())
	{
		nTop = m_displayScreenSize.getY() - 1;
	}

	if (nBottom > m_displayScreenSize.getY())
	{
		nBottom = m_displayScreenSize.getY();
	}

	if (nBottom <= nTop)
	{
		nBottom = nTop + 1;
	}

	m_nTopMargin = nTop;
	m_nBottomMargin = nBottom;

	pthread_mutex_unlock(&m_rwLock);
}

int TerminalState::getTopMargin()
{
	return m_nTopMargin;
}

int TerminalState::getBottomMargin()
{
	return m_nBottomMargin;
}

void TerminalState::setTerminalModeFlags(int nFlags)
{
	pthread_mutex_lock(&m_rwLock);

	m_nTermModeFlags = nFlags;

	pthread_mutex_unlock(&m_rwLock);
}


void TerminalState::addTerminalModeFlags(int nFlags)
{
	pthread_mutex_lock(&m_rwLock);

	m_nTermModeFlags |= nFlags;

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::removeTerminalModeFlags(int nFlags)
{
	pthread_mutex_lock(&m_rwLock);

	m_nTermModeFlags &= ~nFlags;

	pthread_mutex_unlock(&m_rwLock);
}

int TerminalState::getTerminalModeFlags()
{
	return m_nTermModeFlags;
}

void TerminalState::setGraphicsModeFlags(int nFlags)
{
	pthread_mutex_lock(&m_rwLock);

	m_currentGraphicsState.nGraphicsMode = nFlags;

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::addGraphicsModeFlags(int nFlags)
{
	pthread_mutex_lock(&m_rwLock);

	m_currentGraphicsState.nGraphicsMode |= nFlags;

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::removeGraphicsModeFlags(int nFlags)
{
	pthread_mutex_lock(&m_rwLock);

	m_currentGraphicsState.nGraphicsMode &= ~nFlags;

	pthread_mutex_unlock(&m_rwLock);
}

int TerminalState::getGraphicsModeFlags()
{
	return m_currentGraphicsState.nGraphicsMode;
}

void TerminalState::setForegroundColor(TSColor_t color)
{
	pthread_mutex_lock(&m_rwLock);

	m_currentGraphicsState.foregroundColor = color;

	pthread_mutex_unlock(&m_rwLock);
}

TSColor_t TerminalState::getForegroundColor()
{
	return m_currentGraphicsState.foregroundColor;
}

void TerminalState::setBackgroundColor(TSColor_t color)
{
	pthread_mutex_lock(&m_rwLock);

	m_currentGraphicsState.backgroundColor = color;

	pthread_mutex_unlock(&m_rwLock);
}

TSColor_t TerminalState::getBackgroundColor()
{
	return m_currentGraphicsState.backgroundColor;
}

void TerminalState::setCharset(unsigned int ndx, unsigned char charset)
{
	pthread_mutex_lock(&m_rwLock);

	m_currentGraphicsState.charsets[ndx & 0x3] = charset;
	m_currentGraphicsState.charset = m_currentGraphicsState.charsets[m_currentGraphicsState.charset_ndx];

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::useCharset(unsigned int ndx)
{
	pthread_mutex_lock(&m_rwLock);

	m_currentGraphicsState.charset_ndx = ndx & 0x3;
	m_currentGraphicsState.charset = m_currentGraphicsState.charsets[ndx & 0x3];

	pthread_mutex_unlock(&m_rwLock);
}

CellCharacter TerminalState::applyCharset(CellCharacter cChar) {
	/* map offset 0x5F (to 0x7E) */
	static const CellCharacter vt100_mapping[32] = {
		// 0/8     1/9    2/10    3/11    4/12    5/13    6/14    7/15
/*
 * from kde konsole
		// needs (128-char) slots: 0x0000, 0x0080, 0x0380, 0x2200, 0x2400, 0x2500, 0x2580, 0xF800
		0x0020, 0x25C6, 0x2592, 0x2409, 0x240c, 0x240d, 0x240a, 0x00b0,
		0x00b1, 0x2424, 0x240b, 0x2518, 0x2510, 0x250c, 0x2514, 0x253c,
		0xF800, 0xF801, 0x2500, 0xF803, 0xF804, 0x251c, 0x2524, 0x2534,
		0x252c, 0x2502, 0x2264, 0x2265, 0x03C0, 0x2260, 0x00A3, 0x00b7
*/

		// needs (128-char) slots: 0x0000, 0x0080, 0x0380, 0x2200, 0x2500, 0x2580, 0x2600
		0x0020, 0x2666, 0x2592, 0x2595, 0x2595, 0x2595, 0x2595, 0x00B0,
		0x00B1, 0x2595, 0x2595, 0x2518, 0x2510, 0x250C, 0x2514, 0x253C,
		0x2595, 0x2595, 0x2500, 0x2595, 0x2595, 0x251C, 0x2524, 0x2534,
		0x252C, 0x2502, 0x2264, 0x2265, 0x03C0, 0x2260, 0x00A3, 0x00B7
	};

	pthread_mutex_lock(&m_rwLock);

	switch (m_currentGraphicsState.charset) {
	case '0': // SPEC
		if (0x5F <= cChar && cChar <= 0x7E) cChar = vt100_mapping[cChar - 0x5F];
		break;
	case 'A':
		if ('#' == cChar) cChar = 0x00A3;
		break;
	default:
		break;
	}

	pthread_mutex_unlock(&m_rwLock);

	return cChar;
}

unsigned char TerminalState::charset()
{
	return m_currentGraphicsState.charset;
}

TSGraphicsState TerminalState::getCurrentGraphicsState()
{
	return m_currentGraphicsState;
}

TSGraphicsState TerminalState::getDefaultGraphicsState()
{
	return m_defaultGraphicsState;
}

/**
 * Invokes commands based on the given non printable char. Processing the
 * character may change its value.
 * Returns true to attempt to print the character.
 */
bool TerminalState::processNonPrintableChar(CellCharacter &c)
{
	if (isPrintable(c))
	{
		return true;
	}

	pthread_mutex_lock(&m_rwLock);

	switch (c)
	{
	case 8: //Backspace.
		if (getCursorLocation().getX() >= 1)
			moveCursorBackward(1);
		break;
	case 10: //Line feed.
		if ((getTerminalModeFlags() & TS_TM_NEW_LINE) > 0)
		{
			moveCursorNextLine();
		}
		else
		{
			moveCursorDown(1, true);
		}
		break;
	case 11: //Vertical tab
		moveCursorDown(1, true);
		break;
	case 13: //Return.
		setCursorLocation(1, getCursorLocation().getY());
		break;
	}

	pthread_mutex_unlock(&m_rwLock);

	return false;
}

/**
 * Inserts a character at the current cursor position. If advance cursor is specified,
 * then the cursor is moved forward a position after the character has been inserted.
 * If ignore non-printable characters is specified, then non-printable characters will not
 * be processed.
 */
void TerminalState::insertChar(CellCharacter c, bool bAdvanceCursor, bool bIgnoreNonPrintable)
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();
	int nLine;
	unsigned int nPos;
	TSLine *line;
	bool bPrint = true;

	c = applyCharset(c);

	int nCols = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;

	if (!bIgnoreNonPrintable && !isPrintable(c))
	{
		bPrint = processNonPrintableChar(c);
	}

	if (isPrintable(c) || bPrint)
	{
		if (displayLoc.getX() > nCols)
		{
			if ((getTerminalModeFlags() & TS_TM_AUTO_WRAP) > 0)
			{
				moveCursorNextLine();
			}
			else
			{
				setCursorLocation(nCols, m_cursorLoc.getY());
			}

			displayLoc = getDisplayCursorLocation();
		}

		// Get the 'line' for these coordinates
		nPos = displayLoc.getX() - 1;
		nLine = getBufferTopLineIndex() + displayLoc.getY() - 1;
		line = getBufferLine(nLine);
		if (!line) {
			syslog(LOG_ERR, "TerminalState::insertChar fatal error: couldn't access line %i, %i", nLine, displayLoc.getY());
			abort();
		}

		// Insert padding if our line isn't long enough already
		if (line->size() <= nPos)
		{
			int size = line->size();
			line->resize(nPos + 1);
			std::fill(line->begin()+size,line->end(),getEmptyCell());
		} else if (TS_TM_INSERT & m_nTermModeFlags) {
			line->insert(line->begin() + nPos, TSCell());
			if ((int) line->size() > nCols)
				line->resize(nCols);
		}



		// Populate the line with the the specified character, using cur graphics
		(*line)[nPos].graphics = m_currentGraphicsState;
		(*line)[nPos].data = c;

		if (bAdvanceCursor)
		{
			if (displayLoc.getX() >= nCols)
			{
				if ((getTerminalModeFlags() & TS_TM_AUTO_WRAP) > 0)
				{
					m_cursorLoc.setX(nCols + 1);
				}
			}
			else
			{
				moveCursorForward(1);
			}
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::saveScreen()
{
	pthread_mutex_lock(&m_rwLock);

	m_savedScreen.m_data = m_data;
	m_savedScreen.m_savedCursorLoc = m_savedCursorLoc;

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::restoreScreen()
{
	pthread_mutex_lock(&m_rwLock);

	m_data = m_savedScreen.m_data;
	m_savedCursorLoc = m_savedScreen.m_savedCursorLoc;
	setNumBufferLines(m_nNumBufferLines);

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Saves current cursor location, graphics mode, and character set.
 */
void TerminalState::saveCursor()
{
	pthread_mutex_lock(&m_rwLock);

	m_savedGraphicsState = m_currentGraphicsState;
	m_savedCursorLoc = m_cursorLoc;

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Restores last saved cursor location, graphics mode, and character set.
 */
void TerminalState::restoreCursor()
{
	pthread_mutex_lock(&m_rwLock);

	m_currentGraphicsState = m_savedGraphicsState;
	m_cursorLoc = m_savedCursorLoc;

	pthread_mutex_unlock(&m_rwLock);
}

Point TerminalState::getSavedCursorLocation()
{
	return m_savedCursorLoc;
}

int TerminalState::getSavedGraphicsModeFlags()
{
	return m_savedGraphicsState.nGraphicsMode;
}

void TerminalState::lock()
{
	pthread_mutex_lock(&m_rwLock);
}

void TerminalState::unlock()
{
	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::tabForward(unsigned int nTabs) {
	if (tabs.size()==0 || nTabs > tabs.size())
		setCursorLocation((getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80, m_cursorLoc.getY());
	else {
		unsigned int i = 0, t = 0;
		while (t!=nTabs) {
			if (tabs[i]>m_cursorLoc.getX())
				t++;
			i++;
		}
		setCursorLocation(tabs[i-1],m_cursorLoc.getY());
	}
}

void TerminalState::tabBackward(int nTabs) {
	int maxX = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;
	setCursorLocation(maxX-nTabs*8, m_cursorLoc.getY());
}

void TerminalState::resetTerminal()
{
	setTerminalModeFlags(TS_TM_AUTO_REPEAT|TS_TM_AUTO_WRAP|TS_TM_COLUMN|TS_TM_CURSOR);
	eraseScreen();
	setMargin(1,getDisplayScreenSize().getY());
	cursorHome();
	saveCursor();
	tabs.clear();
	memset(m_currentGraphicsState.charsets, 'B', sizeof(m_currentGraphicsState.charsets));
	m_currentGraphicsState.charset = 'B';
	m_defaultGraphicsState.charset_ndx = 0;
	unsolicited = false;
}

void TerminalState::handle_osc(int value, const char *txt)
{
	char *val = 0;
	asprintf(&val, "%d", value);
	const char *params[2];
	params[0] = val;
	params[1] = txt;
	PDL_CallJS("OSCevent", params, 2);
	if (val) free(val);
}
