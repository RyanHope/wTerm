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

#include "terminalstate.hpp"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

const char TerminalState::BLANK = '\x20';

TerminalState::TerminalState()
{
	m_nTermModeFlags = 0;
	m_bShiftText = false;

	m_shift = false;

	unsolicited = false;

	m_nTopBufferLine = 0;
	m_nNumBufferLines = 0;
	m_nTopMargin = 0;
	m_nBottomMargin = 0;

	memset(&m_defaultGraphicsState, 0, sizeof(m_defaultGraphicsState));
	m_defaultGraphicsState.foregroundColor = TS_COLOR_FOREGROUND;
	m_defaultGraphicsState.backgroundColor = TS_COLOR_BACKGROUND;
	m_defaultGraphicsState.g0charset = TS_CS_G0_ASCII;
	m_defaultGraphicsState.g1charset = TS_CS_G1_ASCII;

	memcpy(&m_currentGraphicsState, &m_defaultGraphicsState, sizeof(m_currentGraphicsState));
	memcpy(&m_savedGraphicsState, &m_defaultGraphicsState, sizeof(m_savedGraphicsState));

	pthread_mutexattr_init(&m_rwLockAttr);
	pthread_mutexattr_settype(&m_rwLockAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_rwLock, &m_rwLockAttr);

	setDisplayScreenSize(80, 40);
	setMargin(1, 40);
	cursorHome();
	saveCursor();
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

bool TerminalState::isPrintable(char c)
{
	return !((c >= 0 && c < 32) || c == 127);
}

/**
 * Erases the data from a specific line in the data buffer. Start index must be
 * less than end index.
 */
void TerminalState::clearBufferLine(int nLine, int nStart, int nEnd, TSCell_t & eraseTo)
{
	pthread_mutex_lock(&m_rwLock);

	if (nLine >= 0 && nLine < m_data.size())
	{
		TSLine_t &line = m_data[nLine];

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
			if (line.size() <= nEnd) line.resize(nEnd + 1);

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
	std::deque<TSLine_t> tmp;
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
			m_data.push_front(TSLine_t());
		}

		m_nTopBufferLine = 0;
	}
	else if (nLine >= m_data.size())
	{
		nLine = nLine - m_data.size() + 1;

		//Insert empty lines to the end of the buffer.
		for (int i = 0; i < nLine; i++)
		{
			m_data.push_back(TSLine_t());
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
	TSCell_t eraseTo = getEmptyCell();
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
	int nY = cursor.getY() - 1;
	int nX = cursor.getX() - 1;
	int nEnd = nX + nChars;
	if (nEnd > m_data[nY].size())
		nEnd = m_data[nY].size();
	for(int i = nX; i < nEnd; ++i)
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
				m_data.erase(m_data.begin()+m_nBottomMargin-1);
				m_data.insert(m_data.begin()+curLine-1+i, TSLine_t());
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
				m_data.erase(m_data.begin()+curLine-1+i);
				m_data.insert(m_data.begin()+m_nBottomMargin-1, TSLine_t());
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
TSLine_t * TerminalState::getBufferLine(int nLineIndex)
{
	pthread_mutex_lock(&m_rwLock);

	TSLine_t *buffer = NULL;

	if (nLineIndex >= 0 && nLineIndex < m_data.size())
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
		m_data.push_back(TSLine_t());
	}

	m_nNumBufferLines = nNumLines;

	//Removes excess old buffered lines.
	while (m_data.size() > m_nNumBufferLines && m_nTopBufferLine > 0)
	{
		m_data.pop_front();

		--m_nTopBufferLine;
	}

	//Removes excess overflow buffered lines.
	while (m_data.size() > m_nNumBufferLines && getBufferScreenHeight() > m_displayScreenSize.getY())
	{
		m_data.pop_back();
	}

	//Expand buffer if necessary.
	while (m_data.size() < m_nNumBufferLines)
	{
		m_data.push_back(TSLine_t());
	}

	assert(m_data.size() == m_nNumBufferLines);

	int nScreenWidth = m_displayScreenSize.getX();

	//Trim data.
	for (int i = 0; i < m_data.size(); i++)
	{
		// Truncate all characters beyond screen boundary
		if (m_data[i].size() > nScreenWidth)
			m_data[i].resize(nScreenWidth);
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::cursorHome() {
	if ((m_nTermModeFlags & TS_TM_ORIGIN) > 0)
		setCursorLocation(1,m_nTopMargin);
	else
		setCursorLocation(1,1);
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
	setNumBufferLines(nHeight);

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

	TSLine_t * line = getBufferLine(nY);
	TSCell_t empty = getEmptyCell(); // TODO: What graphics?
	for (int i=0; i<nBlanks; i++)
		line->insert(line->begin()+nX, empty);

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::displayScreenAlignmentPattern() {
	int nCols = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;
	int nRows = m_displayScreenSize.getY();
	for (int r=0;r<nRows;r++) {
		for (int c=0;c<nCols;c++) {
			setCursorLocation(c+1,r+1);
			insertChar(69, false);
		}
	}
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

	if ((nFlags & TS_TM_ORIGIN) != (m_nTermModeFlags & TS_TM_ORIGIN))
	{
		setCursorLocation(1, 1);
	}

	m_nTermModeFlags = nFlags;

	pthread_mutex_unlock(&m_rwLock);
}


void TerminalState::addTerminalModeFlags(int nFlags)
{
	pthread_mutex_lock(&m_rwLock);

	if ((nFlags & TS_TM_ORIGIN) > 0 && (m_nTermModeFlags & TS_TM_ORIGIN) == 0)
	{
		setCursorLocation(1, 1);
	}

	m_nTermModeFlags |= nFlags;

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::removeTerminalModeFlags(int nFlags)
{
	pthread_mutex_lock(&m_rwLock);

	if ((nFlags & TS_TM_ORIGIN) == (m_nTermModeFlags & TS_TM_ORIGIN) && (nFlags & TS_TM_ORIGIN) > 0)
	{
		setCursorLocation(1, 1);
	}

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

void TerminalState::setG0Charset(TSCharset_t charset)
{
	m_currentGraphicsState.g0charset = charset;
}

TSCharset_t TerminalState::getG0Charset()
{
	return m_currentGraphicsState.g0charset;
}

void TerminalState::setG1Charset(TSCharset_t charset)
{
	m_currentGraphicsState.g1charset = charset;
}

TSCharset_t TerminalState::getG1Charset()
{
	return m_currentGraphicsState.g1charset;
}

TSCellGraphicsState_t TerminalState::getCurrentGraphicsState()
{
	return m_currentGraphicsState;
}

TSCellGraphicsState_t TerminalState::getDefaultGraphicsState()
{
	return m_defaultGraphicsState;
}

/**
 * Invokes commands based on the given non printable char. Processing the
 * character may change its value.
 * Returns true to attempt to print the character.
 */
bool TerminalState::processNonPrintableChar(char &c)
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
 * Inserts a character at the current cursor position. Non-printable characters are ignored.
 */
void TerminalState::insertChar(char c, bool bAdvanceCursor)
{
	pthread_mutex_lock(&m_rwLock);

	insertChar(c, bAdvanceCursor, true);

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Inserts a character at the current cursor position. Any character at this location
 * will be replaced.
 */
void TerminalState::insertChar(char c, bool bAdvanceCursor, bool bIgnoreNonPrintable)
{
	pthread_mutex_lock(&m_rwLock);

	insertChar(c, bAdvanceCursor, bIgnoreNonPrintable, false);

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Inserts a character at the current cursor position. If advance cursor is specified,
 * then the cursor is moved forward a position after the character has been inserted.
 * If ignore non-printable characters is specified, then non-printable characters will not
 * be processed. If shift is specified, then all subsequent characters are shifted forward.
 * Otherwise, the current character is replaced.
 */
void TerminalState::insertChar(char c, bool bAdvanceCursor, bool bIgnoreNonPrintable, bool bShift)
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();
	int nLine;
	int nPos;
	TSLine_t *line;
	bool bPrint = true;

	int nCols = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;

	if (!bIgnoreNonPrintable && !isPrintable(c))
	{
		bPrint = processNonPrintableChar(c);
	}

	if (isPrintable(c) || bPrint)
	{
		// TODO: Should mappings be applied to parsing too, not just rendering?
		if (getShift()) c += 128;

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

		// Insert padding if our line isn't long enough already
		if (line->size() <= nPos)
		{
			int size = line->size();
			line->resize(nPos + 1);
			std::fill(line->begin()+size,line->end(),getEmptyCell());
		}

		// Populate the line with the the specified character, using cur graphics
		(*line)[nPos].graphics = m_currentGraphicsState;
		(*line)[nPos].data = c;

		if (bShift)
		{

			//Move the overflow character of each line to the
			//beginning of the next line. If no overflow, just insert
			//a blank character.
			// TODO: AFAICT the code didn't do the above preivously,
			// so I'm preserving the old code's behavior.
			// Which is correct?
			for (int i = nLine; i < m_data.size(); i++)
			{
				line = getBufferLine(i);
				if (line->size() > nCols)
					line->resize(nCols);
			}

		}

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

/**
 * Saves current cursor location, graphics mode, and character set.
 */
void TerminalState::saveCursor()
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();

	m_savedGraphicsState.nGraphicsMode = m_currentGraphicsState.nGraphicsMode;
	m_savedGraphicsState.foregroundColor = m_currentGraphicsState.foregroundColor;
	m_savedGraphicsState.backgroundColor = m_currentGraphicsState.backgroundColor;
	m_savedGraphicsState.g0charset = m_currentGraphicsState.g0charset;
	m_savedGraphicsState.g1charset = m_currentGraphicsState.g1charset;
	m_savedCursorLoc = displayLoc;

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Restores last saved cursor location, graphics mode, and character set.
 */
void TerminalState::restoreCursor()
{
	pthread_mutex_lock(&m_rwLock);

	setCursorLocation(m_savedCursorLoc.getX(), m_savedCursorLoc.getY());
	m_currentGraphicsState.nGraphicsMode = m_savedGraphicsState.nGraphicsMode;
	m_currentGraphicsState.foregroundColor = m_savedGraphicsState.foregroundColor;
	m_currentGraphicsState.backgroundColor = m_savedGraphicsState.backgroundColor;
	m_currentGraphicsState.g0charset = m_savedGraphicsState.g0charset;
	m_currentGraphicsState.g1charset = m_savedGraphicsState.g1charset;

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

void TerminalState::enableShiftText(bool bShift)
{
	pthread_mutex_lock(&m_rwLock);

	m_bShiftText = bShift;

	pthread_mutex_unlock(&m_rwLock);
}
bool TerminalState::isShiftText()
{
	return m_bShiftText;
}


void TerminalState::tabForward(int nTabs) {
	if (tabs.size()==0 || nTabs>tabs.size())
		setCursorLocation((getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80, m_cursorLoc.getY());
	else {
		int i = 0, t = 0;
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

void TerminalState::resetTerminal() {
	setTerminalModeFlags(TS_TM_AUTO_REPEAT|TS_TM_AUTO_WRAP|TS_TM_COLUMN|TS_TM_CURSOR);
	eraseScreen();
	setMargin(1,getDisplayScreenSize().getY());
	cursorHome();
	tabs.clear();
	setShift(true);
	setG0Charset(TS_CS_G0_ASCII);
	setG1Charset(TS_CS_G1_ASCII);
	unsolicited = false;
}

void TerminalState::setShift(bool shift) {
	m_shift = shift;
}

bool TerminalState::getShift() {
	return m_shift;
}
