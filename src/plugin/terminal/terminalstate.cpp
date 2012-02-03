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

	m_savedCharset = 'B';

	unsolicited = false;

	m_nTopMargin = 0;
	m_nBottomMargin = 0;

	pthread_mutexattr_init(&m_rwLockAttr);
	pthread_mutexattr_settype(&m_rwLockAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_rwLock, &m_rwLockAttr);
}

TerminalState::~TerminalState()
{
	pthread_mutexattr_destroy(&m_rwLockAttr);
	pthread_mutex_destroy(&m_rwLock);
}

bool TerminalState::isPrintable(CellCharacter c)
{
	return !((c >= 0 && c < 32) || c == 127);
}

void TerminalState::eraseCurrentLine()
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();

	m_screenBuffer.fillLines(displayLoc.getY(), displayLoc.getY(), TSCell(BLANK, m_currentGraphicsState));

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseCursorToEndOfLine()
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();

	m_screenBuffer.fillLine(displayLoc.getY(), displayLoc.getX(), m_displayScreenSize.getX(), TSCell(BLANK, m_currentGraphicsState));

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseBeginOfLineToCursor()
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();

	m_screenBuffer.fillLine(displayLoc.getY(), 1, displayLoc.getX(), TSCell(BLANK, m_currentGraphicsState));

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseCursorToEndOfScreen()
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();

	m_screenBuffer.fillLine(displayLoc.getY(), displayLoc.getX(), m_displayScreenSize.getX(), TSCell(BLANK, m_currentGraphicsState));
	m_screenBuffer.fillLines(displayLoc.getY() + 1, m_displayScreenSize.getY(), TSCell(BLANK, m_currentGraphicsState));

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseBeginOfScreenToCursor()
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();

	m_screenBuffer.fillLine(displayLoc.getY(), 1, displayLoc.getX(), TSCell(BLANK, m_currentGraphicsState));
	m_screenBuffer.fillLines(1, displayLoc.getY() - 1, TSCell(BLANK, m_currentGraphicsState));

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseScreen()
{
	pthread_mutex_lock(&m_rwLock);

	// why not scrolling things into scrollback ?
	m_screenBuffer.fillLines(1, m_displayScreenSize.getY(), TSCell(BLANK, m_currentGraphicsState));

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::eraseCharacters(int nChars)
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();

	m_screenBuffer.fillLine(displayLoc.getY(), displayLoc.getX(), displayLoc.getX() + nChars - 1, TSCell(BLANK, m_currentGraphicsState));

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::deleteCharacters(int nChars)
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();

	m_screenBuffer.deleteCharacters(displayLoc.getY(), displayLoc.getX(), nChars, TSCell(BLANK, m_currentGraphicsState));

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::insertLines(int nLines)
{
	pthread_mutex_lock(&m_rwLock);

	int curLine = getDisplayCursorLocation().getY();

	// Insert lines into the scroll buffer region:

	if (curLine >= m_nTopMargin) {
		m_screenBuffer.scrollLines(curLine, m_nBottomMargin, -nLines, TSCell(BLANK, m_currentGraphicsState));
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::deleteLines(int nLines)
{
	pthread_mutex_lock(&m_rwLock);

	int curLine = getDisplayCursorLocation().getY();

	// Delete lines from the scroll buffer region:
	if (curLine >= m_nTopMargin) {
		m_screenBuffer.scrollLines(curLine, m_nBottomMargin, nLines, TSCell(BLANK, m_currentGraphicsState));
	}

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
				m_screenBuffer.scrollLines(m_nTopMargin, m_nBottomMargin, nY - m_nTopMargin, TSCell(BLANK, m_currentGraphicsState));
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
				m_screenBuffer.scrollLines(m_nTopMargin, m_nBottomMargin, nY - m_nBottomMargin, TSCell(BLANK, m_currentGraphicsState));
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

void TerminalState::backIndex()
{
	pthread_mutex_lock(&m_rwLock);

	int maxX = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;

	if (m_cursorLoc.getX() == 1) {
		TSCell blank(BLANK, m_currentGraphicsState);
		m_screenBuffer.deleteCharacters(m_cursorLoc.getY(), maxX, 1, blank);
		m_screenBuffer.insertCharacter(m_cursorLoc.getY(), 1, maxX, blank);
	} else {
		moveCursorBackward(1);
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::forwardIndex()
{
	pthread_mutex_lock(&m_rwLock);

	int maxX = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;

	if (m_cursorLoc.getX() == maxX) {
		TSCell blank(BLANK, m_currentGraphicsState);
		m_screenBuffer.deleteCharacters(m_cursorLoc.getY(), 1, 1, blank);
		m_screenBuffer.insertCharacter(m_cursorLoc.getY(), maxX, maxX, blank);
	} else {
		moveCursorForward(1);
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::insertColumns(int value)
{
	pthread_mutex_lock(&m_rwLock);

	int maxX = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;

	TSCell blank(BLANK, m_currentGraphicsState);

	for (int r = m_nTopMargin; r <= m_nBottomMargin; ++r) {
		for (int x=0; x<value; x++) {
			m_screenBuffer.insertCharacter(r, m_cursorLoc.getX(), maxX, blank);
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::deleteColumns(int value)
{
	pthread_mutex_lock(&m_rwLock);

	TSCell blank(BLANK, m_currentGraphicsState);

	for (int r = m_nTopMargin; r <= m_nBottomMargin; ++r)
		m_screenBuffer.deleteCharacters(r, m_cursorLoc.getX(), value, blank);

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::scrollLeft(int value)
{
	pthread_mutex_lock(&m_rwLock);

	TSCell blank(BLANK, m_currentGraphicsState);

	for (int r=getDisplayScreenSize().getY(); r>0; r--)
		m_screenBuffer.deleteCharacters(r, 1, value, blank);

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::scrollRight(int value)
{
	pthread_mutex_lock(&m_rwLock);

	int maxX = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;

	TSCell blank(BLANK, m_currentGraphicsState);

	for (int r=getDisplayScreenSize().getY(); r>0; r--) {
		for (int x=0; x<value; x++) {
			m_screenBuffer.insertCharacter(r, 1, maxX, blank);
		}
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

void TerminalState::setScrollOffset(int offset)
{
	pthread_mutex_lock(&m_rwLock);

	m_screenBuffer.setScrollbackPosition(std::max(0, offset));

	pthread_mutex_unlock(&m_rwLock);
}

int TerminalState::getScrollOffset()
{
	pthread_mutex_lock(&m_rwLock);

	int offset = m_screenBuffer.scrollbackPosition();

	pthread_mutex_unlock(&m_rwLock);

	return offset;
}

void TerminalState::setScrollBufferLines(int lines)
{
	pthread_mutex_lock(&m_rwLock);

	if (lines >= 0) {
		m_screenBuffer.setScrollbackSize(lines);
	}

	pthread_mutex_unlock(&m_rwLock);
}

int TerminalState::getScrollBufferLines()
{
	pthread_mutex_lock(&m_rwLock);

	int lines = m_screenBuffer.scrollbackSize();

	pthread_mutex_unlock(&m_rwLock);

	return lines;
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
	pthread_mutex_lock(&m_rwLock);

	Point result = m_cursorLoc;

	pthread_mutex_unlock(&m_rwLock);

	return result;
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

	m_screenBuffer.setScreenSize(nHeight, nWidth, getDisplayCursorLocation().getY());

	//Reset affected attributes to fix cases where location is out of bounds after setting the display.
	setCursorLocation(m_cursorLoc.getX(), m_cursorLoc.getY());

	setMargin(m_nTopMargin, m_nBottomMargin);

	pthread_mutex_unlock(&m_rwLock);
}

Point TerminalState::getDisplayScreenSize()
{
	pthread_mutex_lock(&m_rwLock);

	Point result = m_displayScreenSize;

	pthread_mutex_unlock(&m_rwLock);

	return result;
}

void TerminalState::insertBlanks(int nBlanks)
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();

	TSCell blank(BLANK, m_currentGraphicsState);
	int nCols = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;

	for (int i = 0; i < nBlanks; i++) {
		m_screenBuffer.insertCharacter(displayLoc.getY(), displayLoc.getX(), nCols, blank);
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::displayScreenAlignmentPattern() {
	pthread_mutex_lock(&m_rwLock);

	int nCols = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;
	int nRows = m_displayScreenSize.getY();

	TSCell pattern('E', m_currentGraphicsState);

	for (int r = 1; r <= nRows; r++) {
		for (int c = 1; c <= nCols; c++) {
			m_screenBuffer.replaceCharacter(r, c, pattern);
		}
	}

	setCursorLocation(1,1);

	pthread_mutex_unlock(&m_rwLock);
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

	if (nBottom - nTop < 2)
	{
		nTop = 1;
		nBottom = m_displayScreenSize.getY();
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

void TerminalState::setForegroundColor(TSColor color)
{
	pthread_mutex_lock(&m_rwLock);

	m_currentGraphicsState.foregroundColor = color;

	pthread_mutex_unlock(&m_rwLock);
}

TSColor TerminalState::getForegroundColor()
{
	return m_currentGraphicsState.foregroundColor;
}

void TerminalState::setBackgroundColor(TSColor color)
{
	pthread_mutex_lock(&m_rwLock);

	m_currentGraphicsState.backgroundColor = color;

	pthread_mutex_unlock(&m_rwLock);
}

TSColor TerminalState::getBackgroundColor()
{
	return m_currentGraphicsState.backgroundColor;
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

	switch (m_currentCharset.charset) {
	case '0': // SPEC
	case '2': // SPEC
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

TSGraphicsState TerminalState::getCurrentGraphicsState()
{
	return m_currentGraphicsState;
}

/**
 * Inserts a printable character at the current cursor position. If advance cursor is
 * specified, then the cursor is moved forward a position after the character has been
 * inserted. If ignore non-printable characters is specified, then non-printable characters
 * will not be processed.
 */
void TerminalState::insertChar(CellCharacter c)
{
	pthread_mutex_lock(&m_rwLock);

	Point displayLoc = getDisplayCursorLocation();

	c = applyCharset(c);

	int nCols = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;

	if (isPrintable(c))
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

		if (TS_TM_INSERT & m_nTermModeFlags) {
			m_screenBuffer.insertCharacter(displayLoc.getY(), displayLoc.getX(), nCols, TSCell(c, m_currentGraphicsState));
		} else {
			m_screenBuffer.replaceCharacter(displayLoc.getY(), displayLoc.getX(), TSCell(c, m_currentGraphicsState));
		}

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

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::saveScreen()
{
	pthread_mutex_lock(&m_rwLock);

	m_screenBuffer.save(m_savedScreen.screenStore);
	m_savedScreen.savedCursorLoc = m_savedCursorLoc;

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::restoreScreen()
{
	pthread_mutex_lock(&m_rwLock);

	m_screenBuffer.restore(m_savedScreen.screenStore);
	m_savedCursorLoc = m_savedScreen.savedCursorLoc;

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
	m_savedCharset = m_currentCharset.charset;

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
	m_currentCharset.charset = m_savedCharset;

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
	if (0 == nTabs) return;

	pthread_mutex_lock(&m_rwLock);

	int nCols = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;

	if (tabs.size()==0 || nTabs > tabs.size()) {
		setCursorLocation(nCols, m_cursorLoc.getY());
	} else {
		for (unsigned int i = 0, t = 0; ; ++i) {
			if (i >= tabs.size()) {
				setCursorLocation(nCols, m_cursorLoc.getY());
				break;
			}
			if (tabs[i] > m_cursorLoc.getX())
				t++;
			if (t == nTabs) {
				setCursorLocation(tabs[i], m_cursorLoc.getY());
				break;
			}
		}
	}

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::tabBackward(int nTabs) {
	pthread_mutex_lock(&m_rwLock);

	int maxX = (getTerminalModeFlags() & TS_TM_COLUMN) ? getDisplayScreenSize().getX() : 80;
	setCursorLocation(maxX-nTabs*8, m_cursorLoc.getY());

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::resetTerminal()
{
	pthread_mutex_lock(&m_rwLock);

	setCursorStyle(TS_CURSOR_STYLE_BLOCK_STEADY);
	setTerminalModeFlags(TS_TM_AUTO_REPEAT|TS_TM_AUTO_WRAP|TS_TM_COLUMN|TS_TM_CURSOR);
	setMargin(1,getDisplayScreenSize().getY());
	eraseScreen();
	cursorHome();
	m_currentGraphicsState.reset();
	m_currentCharset.reset();
	saveCursor();
	tabs.clear();
	unsolicited = false;

	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::handle_osc(int value, const char *txt)
{
	if (PDL_IsPlugin())
	{
		char *val = 0;
		asprintf(&val, "%d", value);
		const char *params[2];
		params[0] = val;
		params[1] = txt;
		PDL_CallJS("OSCevent", params, 2);
		if (val) free(val);
	}
}

void TerminalState::setCursorStyle(TSCursorStyle style)
{
	pthread_mutex_lock(&m_rwLock);
	m_cursorStyle = style;
	pthread_mutex_unlock(&m_rwLock);
}

void TerminalState::processCursorStyle(int style)
{
	switch (style) {
	case 0:
	case 1:
		setCursorStyle(TS_CURSOR_STYLE_BLOCK_BLINK);
		break;
	case 2:
		setCursorStyle(TS_CURSOR_STYLE_BLOCK_STEADY);
		break;
	case 3:
		setCursorStyle(TS_CURSOR_STYLE_UNDERLINE_BLINK);
		break;
	case 4:
		setCursorStyle(TS_CURSOR_STYLE_UNDERLINE_STEADY);
		break;
	case 5:
		setCursorStyle(TS_CURSOR_STYLE_VERTICALLINE_BLINK);
		break;
	case 6:
		setCursorStyle(TS_CURSOR_STYLE_VERTICALLINE_STEADY);
		break;
	}
}

TSCursorStyle TerminalState::getCursorStyle() {
	return m_cursorStyle;
}
