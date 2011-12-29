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

#include "terminalstate.hpp"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

const char TerminalState::BLANK = '\x20';

int cmp_graphics_state(TSLineGraphicsState_t const *state1, TSLineGraphicsState_t const *state2)
{
	if (state1 != NULL && state2 != NULL)
	{
		if (state1->nGraphicsMode == state2->nGraphicsMode
			&& state1->foregroundColor == state2->foregroundColor
			&& state1->backgroundColor == state2->backgroundColor)
		{
			return 0;
		}
	}

	return -1;
}

TerminalState::TerminalState()
{
	m_nTermModeFlags = 0;
	m_charset = TS_CS_NONE;
	m_bShiftText = false;

	m_nTopBufferLine = 0;
	m_nNumBufferLines = 0;
	m_nTopMargin = 0;
	m_nBottomMargin = 0;

	memset(&m_defaultGraphicsState, 0, sizeof(m_defaultGraphicsState));
	m_defaultGraphicsState.nColumn = 1;
	m_defaultGraphicsState.nLine = 1;
	m_defaultGraphicsState.foregroundColor = TS_COLOR_WHITE_BRIGHT;
	m_defaultGraphicsState.backgroundColor = TS_COLOR_BLACK;

	memcpy(&m_currentGraphicsState, &m_defaultGraphicsState, sizeof(m_currentGraphicsState));
	memcpy(&m_savedGraphicsState, &m_defaultGraphicsState, sizeof(m_savedGraphicsState));

	pthread_mutexattr_init(&m_rwLockAttr);
	pthread_mutexattr_settype(&m_rwLockAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_rwLock, &m_rwLockAttr);

	setDisplayScreenSize(80, 40);
	setMargin(1, 40);
	saveCursor();
}

TerminalState::~TerminalState()
{
	pthread_mutex_lock(&m_rwLock);

	freeBuffer();
	freeGraphicsMode();

	pthread_mutex_unlock(&m_rwLock);

	pthread_mutexattr_destroy(&m_rwLockAttr);
	pthread_mutex_destroy(&m_rwLock);
}

void TerminalState::freeBuffer()
{
	pthread_mutex_lock(&m_rwLock);

	for (std::deque<DataBuffer *>::iterator itr = m_data.begin(); itr != m_data.end(); itr++)
	{
		delete (*itr);
	}

	m_data.clear();

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::freeGraphicsMode()
{
	pthread_mutex_lock(&m_rwLock);

	for (std::vector<TSLineGraphicsState_t *>::iterator itr = m_graphicsState.begin(); itr != m_graphicsState.end(); itr++)
	{
		free(*itr);
	}

	m_graphicsState.clear();

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Free memory of the elements from start to end. Includes the start, but does not remove the end element.
 */
void TerminalState::freeGraphicsMode(std::vector<TSLineGraphicsState_t *>::iterator start, std::vector<TSLineGraphicsState_t *>::iterator end)
{
	pthread_mutex_lock(&m_rwLock);

	for (std::vector<TSLineGraphicsState_t *>::iterator itr = start; itr != end; itr++)
	{
		free(*itr);
	}

	m_graphicsState.erase(start, end);

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
void TerminalState::clearBufferLine(int nLine, int nStart, int nEnd)
{
	DataBuffer *line;
	int nSize;

	pthread_mutex_lock(&m_rwLock);

	if (nLine >= 0 && nLine < m_data.size())
	{
		line = m_data[nLine];

		if (nStart < 0)
		{
			nStart = 0;
		}

		if (nEnd < 0)
		{
			nEnd = 0;
		}

		if (nEnd >= line->size())
		{
			nEnd = (line->size() < 1) ? 0 : (line->size() - 1);
		}

		if (nStart <= nEnd)
		{
			nSize = (nEnd - nStart + 1);
			line->clear(nStart, nSize, false);
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

	//Retain buffer outside of scroll region.
	std::deque<DataBuffer *> tmp;
	int nSizeTopMargin = getTopMargin() - 1;
	int nSizeBottomMargin = getDisplayScreenSize().getY() - getBottomMargin();

	for (int i = 0; i < nSizeTopMargin; i++)
	{
		tmp.push_back(m_data[m_nTopBufferLine + i]);
		m_data[m_nTopBufferLine + i] = new DataBuffer();
	}

	for (int i = 0; i < nSizeBottomMargin; i++)
	{
		tmp.push_back(m_data[m_nTopBufferLine + i + getBottomMargin()]);
		m_data[m_nTopBufferLine + i + getBottomMargin()] = new DataBuffer();
	}

	//Move buffer up
	if (nLine < 0)
	{
		nLine = (-1) * nLine;

		//Insert empty lines to the top of the buffer.
		for (int i = 0; i < nLine; i++)
		{
			m_data.push_front(new DataBuffer());
		}

		m_nTopBufferLine = 0;
		moveGraphicsState(nLine, false);
	}
	else if (nLine >= m_data.size())
	{
		nLine = nLine - m_data.size() + 1;

		//Insert empty lines to the end of the buffer.
		for (int i = 0; i < nLine; i++)
		{
			m_data.push_back(new DataBuffer());
		}

		m_nTopBufferLine = (m_data.size() - 1);
		moveGraphicsState(nLine, true);
	}
	else
	{
		if (m_nTopBufferLine > nLine)
		{
			moveGraphicsState(m_nTopBufferLine - nLine, false);
		}
		else if (m_nTopBufferLine < nLine)
		{
			moveGraphicsState(nLine - m_nTopBufferLine, true);
		}

		m_nTopBufferLine = nLine;
	}

	//Validate and fixup buffer.
	setNumBufferLines(m_nNumBufferLines);

	//Restore buffer outside scroll region.
	for (int i = 0; i < nSizeTopMargin; i++)
	{
		delete m_data[m_nTopBufferLine + i];
		m_data[m_nTopBufferLine + i] = tmp.front();
		tmp.pop_front();
	}

	for (int i = 0; i < nSizeBottomMargin; i++)
	{
		delete m_data[m_nTopBufferLine + i + getBottomMargin()];
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

	removeGraphicsState(nStartX, nStartLine, nEndX, nEndLine, &m_currentGraphicsState);

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
				clearBufferLine(i, nStartX, nEndX);
			}
			else
			{
				clearBufferLine(i, nStartX, m_data[i]->size() - 1);
			}
		}
		//Process last line.
		else if (i == nEndLine)
		{
			clearBufferLine(i, 0, nEndX);
		}
		//Clear lines in between.
		else
		{
			m_data[i]->clear();
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

void TerminalState::insertLines(int nLines)
{
	pthread_mutex_lock(&m_rwLock);

	int curLine = convertToDisplayLocation(m_cursorLoc).getY();


	if (curLine >= m_nTopMargin && curLine < m_nBottomMargin) {
		for (int i=0; i<nLines; i++) {
			if (curLine-1+i < m_nBottomMargin) {
				m_data.erase(m_data.begin()+m_nBottomMargin-1);
				m_data.insert(m_data.begin()+curLine-1+i, new DataBuffer());
			}
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::deleteLines(int nLines)
{
	pthread_mutex_lock(&m_rwLock);

	int curLine = convertToDisplayLocation(m_cursorLoc).getY();

	if (curLine >= m_nTopMargin && curLine < m_nBottomMargin) {
		for (int i=0; i<nLines; i++) {
			if (curLine-1+i < m_nBottomMargin) {
				m_data.erase(m_data.begin()+curLine-1+i);
				m_data.insert(m_data.begin()+m_nBottomMargin-1, new DataBuffer());
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

	if (nPos >= 0)
	{
		setCursorLocation(m_cursorLoc.getX() + nPos, m_cursorLoc.getY());
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::moveCursorBackward(int nPos)
{
	pthread_mutex_lock(&m_rwLock);

	if (nPos >= 0)
	{
		setCursorLocation(m_cursorLoc.getX() - nPos, m_cursorLoc.getY());
	}

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
DataBuffer *TerminalState::getBufferLine(int nLineIndex)
{
	pthread_mutex_lock(&m_rwLock);

	DataBuffer *buffer = NULL;

	if (nLineIndex >= 0 && nLineIndex < m_data.size())
	{
		buffer = m_data[nLineIndex];
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
		m_data.push_back(new DataBuffer());
	}

	m_nNumBufferLines = nNumLines;

	//Removes excess old buffered lines.
	while (m_data.size() > m_nNumBufferLines && m_nTopBufferLine > 0)
	{
		delete m_data.front();
		m_data.pop_front();

		--m_nTopBufferLine;
	}

	//Removes excess overflow buffered lines.
	while (m_data.size() > m_nNumBufferLines && getBufferScreenHeight() > m_displayScreenSize.getY())
	{
		delete m_data.back();
		m_data.pop_back();
	}

	//Expand buffer if necessary.
	while (m_data.size() < m_nNumBufferLines)
	{
		m_data.push_back(new DataBuffer());
	}

	assert(m_data.size() == m_nNumBufferLines);

	int nScreenWidth = m_displayScreenSize.getX();

	//Trim data.
	for (int i = 0; i < m_data.size(); i++)
	{
		if (m_data[i] != NULL && m_data[i]->size() > nScreenWidth)
		{
			m_data[i]->clear(nScreenWidth, m_data[i]->size() - nScreenWidth, true);
		}
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

	cursorHome();

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

void TerminalState::setCharset(TSCharset_t charset)
{
	m_charset = charset;
}

TSCharset_t TerminalState::getCharset()
{
	return m_charset;
}

TSLineGraphicsState_t TerminalState::getCurrentGraphicsState()
{
	return m_currentGraphicsState;
}

TSLineGraphicsState_t TerminalState::getDefaultGraphicsState()
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
		{
			deleteChar(true, m_bShiftText);
		}
		break;
	case 9: //Tab.
		insertChar(' ', true, true, m_bShiftText);
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
	DataBuffer *line, *nextLine;
	bool bPrint = true;

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
				setCursorLocation(getDisplayScreenSize().getX(), m_cursorLoc.getY());
			}

			displayLoc = getDisplayCursorLocation();
		}

		addGraphicsState(displayLoc.getX(), displayLoc.getY(),
			m_currentGraphicsState.foregroundColor, m_currentGraphicsState.backgroundColor,
			m_currentGraphicsState.nGraphicsMode, TS_GM_OP_SET, false);

		nPos = displayLoc.getX() - 1;
		nLine = getBufferTopLineIndex() + displayLoc.getY() - 1;
		line = getBufferLine(nLine);

		//Add padding.
		if (line->size() <= nPos)
		{
			line->fill(BLANK, nPos - line->size());
		}

		if (bShift)
		{
			int nScreenWidth = getDisplayScreenSize().getX();
			int nOverFlowSize;
			char *tmp = (char *)malloc(nScreenWidth * sizeof(char));
			char cEmpty = BLANK;

			getBufferLine(nLine)->insert(nPos, &c, 1);

			//Move the overflow character of each line to the
			//beginning of the next line. If no overflow, just insert
			//a blank character.
			for (int i = nLine; i < m_data.size(); i++)
			{
				line = getBufferLine(i);
				nOverFlowSize = line->size() - nScreenWidth;

				if ((i + 1) < m_data.size())
				{
					nextLine = getBufferLine(i + 1);

					if (nOverFlowSize > 0)
					{
						line->copy(nScreenWidth, tmp, nOverFlowSize);
						nextLine->insert(0, tmp, nOverFlowSize);
					}
					else if (nextLine->size() > 0)
					{
						//Insert an empty padding.
						nextLine->insert(0, &cEmpty, 1);
					}
				}

				line->clear(nScreenWidth, nOverFlowSize, true);
			}

			free(tmp);
		}
		else
		{
			if (line->size() <= nPos)
			{
				line->append(&c, 1);
			}
			else
			{
				line->replace(nPos, &c, 1);
			}
		}

		if (bAdvanceCursor)
		{
			if (displayLoc.getX() >= nCols)
			{
				if ((getTerminalModeFlags() & TS_TM_AUTO_WRAP) > 0)
				{
					m_cursorLoc.setX(getDisplayScreenSize().getX() + 1);
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
 * Deletes a character at current cursor position. If shift is specified, then all the characters following
 * the cursor is shifted backwards. If advance cursor is specified, then the cursor is moved back a position
 * prior to the delete.
 */
void TerminalState::deleteChar(bool bAdvanceCursor, bool bShift)
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();
	int nLine;

	if (bAdvanceCursor)
	{
		if (displayLoc.getX() > 1)
		{
			moveCursorBackward(1);
		}
		else
		{
			moveCursorUp(1, true);
			moveCursorForward(getDisplayScreenSize().getX() - 1);
		}
	}

	displayLoc = getDisplayCursorLocation();
	nLine = displayLoc.getY() + getBufferTopLineIndex() - 1;

	if (bShift)
	{
		int nLastColumnIndex = getDisplayScreenSize().getX() - 1;
		DataBuffer *line = getBufferLine(nLine);
		DataBuffer *prevLine;
		char c[1];

		line->clear(displayLoc.getX() - 1, 1, true);

		//Move the first character of each subsequent line to the
		//last position of the previous line.
		for (int i = nLine + 1; i < m_data.size(); i++)
		{
			prevLine = getBufferLine(i - 1);
			line = getBufferLine(i);

			if (line->size() > 0)
			{
				line->copy(c, 1);
			}
			else
			{
				c[0] = BLANK;
			}

			if (line->size() > 0)
			{
				//Add padding.
				if (prevLine->size() <= nLastColumnIndex)
				{
					prevLine->fill(BLANK, nLastColumnIndex - prevLine->size());
					prevLine->append(c, 1);
				}
				else
				{
					prevLine->replace(nLastColumnIndex, c, 1);
				}
			}

			line->clear(0, 1, true);
		}
	}
	else
	{
		char c = BLANK;

		DataBuffer *line = getBufferLine(nLine);
		line->replace(displayLoc.getX() - 1, &c, 1);
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

	m_savedGraphicsState.nColumn = m_cursorLoc.getX();
	m_savedGraphicsState.nLine = m_cursorLoc.getY();
	m_savedGraphicsState.nGraphicsMode = m_currentGraphicsState.nGraphicsMode;
	m_savedGraphicsState.foregroundColor = m_currentGraphicsState.foregroundColor;
	m_savedGraphicsState.backgroundColor = m_currentGraphicsState.backgroundColor;

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * Restores last saved cursor location, graphics mode, and character set.
 */
void TerminalState::restoreCursor()
{
	pthread_mutex_lock(&m_rwLock);

	setCursorLocation(m_savedGraphicsState.nColumn, m_savedGraphicsState.nLine);
	m_currentGraphicsState.nGraphicsMode = m_savedGraphicsState.nGraphicsMode;
	m_currentGraphicsState.foregroundColor = m_savedGraphicsState.foregroundColor;
	m_currentGraphicsState.backgroundColor = m_savedGraphicsState.backgroundColor;

	pthread_mutex_unlock(&m_rwLock);
}

Point TerminalState::getSavedCursorLocation()
{
	return Point(m_savedGraphicsState.nColumn, m_savedGraphicsState.nLine);
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

void TerminalState::addGraphicsState(int nColumn, int nLine, TSColor_t foregroundColor, TSColor_t backgroundColor, int nGraphicsMode, TSGraphicsModeOp_t op, bool bTrim)
{
	pthread_mutex_lock(&m_rwLock);

	int nLocator = findGraphicsState(nColumn, nLine, true);
	int nMode = 0;
	TSLineGraphicsState_t *prevState = NULL;
	TSLineGraphicsState_t *newState = NULL;

	if (nLocator >= 0 && nLocator < m_graphicsState.size())
	{
		prevState = m_graphicsState[nLocator];

		if (prevState->nColumn == nColumn && prevState->nLine == nLine)
		{
			newState = prevState;
		}
		else
		{
			nLocator++;
		}
	}

	if (bTrim)
	{
		removeGraphicsState(nColumn + 1, nLine, true, NULL);
	}

	if (foregroundColor == TS_COLOR_MAX)
	{
		foregroundColor = (prevState == NULL) ? m_defaultGraphicsState.foregroundColor : prevState->foregroundColor;
	}

	if (backgroundColor == TS_COLOR_MAX)
	{
		backgroundColor = (prevState == NULL) ? m_defaultGraphicsState.backgroundColor : prevState->backgroundColor;
	}

	nMode = (prevState == NULL) ? m_defaultGraphicsState.nGraphicsMode : prevState->nGraphicsMode;

	switch (op)
	{
	case TS_GM_OP_SET:
		nMode = nGraphicsMode;
		break;
	case TS_GM_OP_ADD:
		nMode |= nGraphicsMode;
		break;
	case TS_GM_OP_REMOVE:
		nMode &= ~nGraphicsMode;
		break;
	}

	if (prevState == NULL)
	{
		prevState = &m_defaultGraphicsState;
	}

	if (newState == NULL)
	{
		newState = (TSLineGraphicsState_t *)malloc(sizeof(TSLineGraphicsState_t));
	}

	newState->nColumn = nColumn;
	newState->nLine = nLine;
	newState->foregroundColor = foregroundColor;
	newState->backgroundColor = backgroundColor;
	newState->nGraphicsMode = nMode;

	if (prevState != newState)
	{
		//Prevent duplicate state on the same line.
		if (cmp_graphics_state(prevState, newState) == 0
			&& prevState->nLine == newState->nLine)
		{
			free(newState);
		}
		else
		{
			if (nLocator < 0)
			{
				m_graphicsState.insert(m_graphicsState.begin(), newState);
			}
			else if (nLocator >= m_graphicsState.size())
			{
				m_graphicsState.insert(m_graphicsState.end(), newState);
			}
			else
			{
				m_graphicsState.insert(m_graphicsState.begin() + nLocator, newState);
			}
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::removeGraphicsState(int nColumn, int nLine, bool bTrim, TSLineGraphicsState_t *resetState)
{
	pthread_mutex_lock(&m_rwLock);

	int nLocator;

	if (!bTrim)
	{
		nLocator = findGraphicsState(nColumn, nLine, false);

		if (nLocator >= 0 && nLocator < m_graphicsState.size())
		{
			free(m_graphicsState[nLocator]);
			m_graphicsState.erase(m_graphicsState.begin() + nLocator);
		}
	}
	else
	{
		nLocator = findGraphicsState(nColumn, nLine, true);

		if (nLocator < 0)
		{
			freeGraphicsMode();
		}
		else
		{
			nLocator++;

			if (nLocator < m_graphicsState.size())
			{
				freeGraphicsMode(m_graphicsState.begin() + nLocator, m_graphicsState.end());
			}
		}

		//Insert a graphics state at the start if the erased block.
		if (resetState != NULL)
		{
			nLocator = findGraphicsState(nColumn, nLine, true);

			if (nLocator >= 0 && nLocator < m_graphicsState.size())
			{
				TSLineGraphicsState_t *tmpState = m_graphicsState[nLocator];

				if (tmpState->foregroundColor != resetState->foregroundColor
					|| tmpState->backgroundColor != resetState->backgroundColor
					|| tmpState->nGraphicsMode != resetState->nGraphicsMode)
				{
					addGraphicsState(nColumn, nLine, resetState->foregroundColor, resetState->backgroundColor, resetState->nGraphicsMode, TS_GM_OP_SET, false);
				}
			}
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::removeGraphicsState(int nBeginColumn, int nBeginLine, int nEndColumn, int nEndLine, TSLineGraphicsState_t *resetState)
{
	pthread_mutex_lock(&m_rwLock);

	int nBeginLocator;
	int nEndLocator;
	std::vector<TSLineGraphicsState_t *>::iterator beginLocator = m_graphicsState.end();
	std::vector<TSLineGraphicsState_t *>::iterator endLocator = m_graphicsState.end();
	TSLineGraphicsState_t *tmpState;

	nBeginLocator = findGraphicsState(nBeginColumn, nBeginLine, true);
	nEndLocator = findGraphicsState(nEndColumn, nEndLine, true);

	if (nBeginLocator >= 0 && nBeginLocator < m_graphicsState.size())
	{
		tmpState = m_graphicsState[nBeginLocator];

		//Previous state was returned.
		if (tmpState->nColumn != nBeginColumn || tmpState->nLine != nBeginLine)
		{
			nBeginLocator++;
		}
	}

	for (int i = nBeginLocator; i <= nEndLocator; i++)
	{
		if (i >= 0 && i < m_graphicsState.size())
		{
			if (beginLocator == m_graphicsState.end())
			{
				beginLocator = m_graphicsState.begin() + i;
			}

			endLocator = m_graphicsState.begin() + i + 1;
		}
	}

	freeGraphicsMode(beginLocator, endLocator);

	//Insert a graphics state at the start if the erased block.
	if (resetState != NULL)
	{
		nBeginLocator = findGraphicsState(nBeginColumn, nBeginLine, true);

		if (nBeginLocator >= 0 && nBeginLocator < m_graphicsState.size())
		{
			tmpState = m_graphicsState[nBeginLocator];

			if (tmpState->foregroundColor != resetState->foregroundColor
				|| tmpState->backgroundColor != resetState->backgroundColor
				|| tmpState->nGraphicsMode != resetState->nGraphicsMode)
			{
				addGraphicsState(nBeginColumn, nBeginLine, resetState->foregroundColor, resetState->backgroundColor, resetState->nGraphicsMode, TS_GM_OP_SET, false);
			}
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

/**
 * With bGetPrev set, then if an exact match is not found, the index of the previous element
 * will be returned.
 */
int TerminalState::findGraphicsState(int nColumn, int nLine, bool bGetPrev)
{
	pthread_mutex_lock(&m_rwLock);

	std::vector<TSLineGraphicsState_t *>::iterator itr = m_graphicsState.begin();
	int nResult = -1;
	TSLineGraphicsState_t *curState;
	int nFirst, nLast, nMid;

	nFirst = 0;
	nLast = m_graphicsState.size() - 1;

	while (nFirst <= nLast)
	{
		nMid = (nFirst + nLast) / 2;
		curState = m_graphicsState[nMid];

		if (curState->nLine > nLine || (curState->nLine == nLine && curState->nColumn > nColumn))
		{
			if (bGetPrev)
			{
				nResult = nMid - 1;
			}

			nLast = nMid - 1;
		}
		else if (curState->nLine < nLine || (curState->nLine == nLine && curState->nColumn < nColumn))
		{
			if (bGetPrev)
			{
				nResult = nMid;
			}

			nFirst = nMid + 1;
		}
		else
		{
			nResult = nMid;
			break;
		}
	}

	pthread_mutex_unlock(&m_rwLock);

	return nResult;
}

void TerminalState::moveGraphicsState(int nLines, bool bUp)
{
	pthread_mutex_lock(&m_rwLock);

	TSLineGraphicsState_t *tmpState;

	for (int i = 0; i < m_graphicsState.size(); i++)
	{
		tmpState = m_graphicsState[i];

		//Do not move lines outside of scroll region.
		if (tmpState->nLine >= getTopMargin() && tmpState->nLine <= getBottomMargin())
		{
			if (bUp)
			{
				tmpState->nLine -= nLines;
			}
			else
			{
				tmpState->nLine += nLines;
			}

			if (tmpState->nLine < getTopMargin() || tmpState->nLine > getBottomMargin())
			{
				free(tmpState);
				m_graphicsState.erase(m_graphicsState.begin() + i);
				i--;
			}
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::getLineGraphicsState(int nLine, TSLineGraphicsState_t **states, int &nNumStates, int nMaxStates)
{
	pthread_mutex_lock(&m_rwLock);

	int nBeginLocator;
	int nEndLocator;

	nNumStates = 0;
	nBeginLocator = findGraphicsState(1, nLine, true);
	nEndLocator = findGraphicsState(getDisplayScreenSize().getX(), nLine, true);

	if (nBeginLocator < 0 || nBeginLocator >= m_graphicsState.size())
	{
		if (nNumStates < nMaxStates)
		{
			states[nNumStates] = &m_defaultGraphicsState;
		}

		nNumStates++;
	}

	for (int i = nBeginLocator; i <= nEndLocator; i++)
	{
		if (i >= 0 && i < m_graphicsState.size())
		{
			if (nNumStates < nMaxStates)
			{
				states[nNumStates] = m_graphicsState[i];
			}

			nNumStates++;
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::resetTerminal() {
	setTerminalModeFlags(TS_TM_AUTO_REPEAT|TS_TM_AUTO_WRAP|TS_TM_COLUMN);
	eraseScreen();
	setMargin(1,getDisplayScreenSize().getY());
}
