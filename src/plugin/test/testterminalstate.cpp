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

#include "util/logger.hpp"
#include "test/unittest.hpp"

#include "terminal/terminalstate.hpp"
#include "terminal/vtterminalstate.hpp"

#include <stdlib.h>
#include <string.h>

char terminalBuffer[40][80];

class TerminalStateTest : public VTTerminalState
{
public:
	TerminalStateTest()
	{
	}

	virtual ~TerminalStateTest()
	{
	}

	bool testState(int nIndex, int nColumn, int nLine, int foregroundColor, int backgroundColor, int nMode)
	{
		TSLineGraphicsState_t *tmpState = m_graphicsState[nIndex];

		assertEquals(nColumn, tmpState->nColumn, "Test graphics state column");
		assertEquals(nLine, tmpState->nLine, "Test graphics state line");
		assertEquals(foregroundColor, tmpState->foregroundColor, "Test graphics state foreground color");
		assertEquals(backgroundColor, tmpState->backgroundColor, "Test graphics state background color");
		assertEquals(nMode, tmpState->nGraphicsMode, "Test graphics state mode");

		return true;
	}

	void testGraphicsState()
	{
		TSLineGraphicsState_t *tmpState;

		assertEquals(0, m_graphicsState.size(), "Test initial graphics state size");

		addGraphicsState(4, 10, TS_COLOR_MAX, TS_COLOR_MAX, 1, TS_GM_OP_SET, false);
		addGraphicsState(8, 12, TS_COLOR_YELLOW, TS_COLOR_MAX, 1, TS_GM_OP_ADD, false);
		addGraphicsState(30, 10, TS_COLOR_MAX, TS_COLOR_CYAN, 4, TS_GM_OP_ADD, false);
		addGraphicsState(1, 14, TS_COLOR_GREEN, TS_COLOR_BLUE_BRIGHT, 1, TS_GM_OP_REMOVE, false);
		addGraphicsState(40, 13, TS_COLOR_MAX, TS_COLOR_MAX, 6, TS_GM_OP_SET, false);

		assertEquals(5, m_graphicsState.size(), "Test graphics state size");

		Logger::getInstance()->error("Testing state 0");
		testState(0, 4, 10, TS_COLOR_WHITE_BRIGHT, TS_COLOR_BLACK, 1);

		Logger::getInstance()->error("Testing state 1");
		testState(1, 30, 10, TS_COLOR_WHITE_BRIGHT, TS_COLOR_CYAN, 5);

		Logger::getInstance()->error("Testing state 2");
		testState(2, 8, 12, TS_COLOR_YELLOW, TS_COLOR_BLACK, 1);

		Logger::getInstance()->error("Testing state 3");
		testState(3, 40, 13, TS_COLOR_YELLOW, TS_COLOR_BLACK, 6);

		Logger::getInstance()->error("Testing state 4");
		testState(4, 1, 14, TS_COLOR_GREEN, TS_COLOR_BLUE_BRIGHT, 0);

		addGraphicsState(40, 13, TS_COLOR_CYAN, TS_COLOR_MAX, 1, TS_GM_OP_ADD, false);
		assertEquals(5, m_graphicsState.size(), "Test graphics state size (1)");

		Logger::getInstance()->error("Testing state 3 (1)");
		testState(3, 40, 13, TS_COLOR_CYAN, TS_COLOR_BLACK, 7);

		addGraphicsState(50, 12, TS_COLOR_GREEN, TS_COLOR_RED_BRIGHT, 8, TS_GM_OP_SET, true);
		assertEquals(4, m_graphicsState.size(), "Test graphics state size (2)");

		Logger::getInstance()->error("Testing state 0 (1)");
		testState(0, 4, 10, TS_COLOR_WHITE_BRIGHT, TS_COLOR_BLACK, 1);

		Logger::getInstance()->error("Testing state 1 (1)");
		testState(1, 30, 10, TS_COLOR_WHITE_BRIGHT, TS_COLOR_CYAN, 5);

		Logger::getInstance()->error("Testing state 2 (1)");
		testState(2, 8, 12, TS_COLOR_YELLOW, TS_COLOR_BLACK, 1);

		Logger::getInstance()->error("Testing state 3 (1)");
		testState(3, 50, 12, TS_COLOR_GREEN, TS_COLOR_RED_BRIGHT, 8);

		removeGraphicsState(20, 10, 10, 12, NULL);
		assertEquals(2, m_graphicsState.size(), "Test graphics state size (3)");

		Logger::getInstance()->error("Testing state 0 (2)");
		testState(0, 4, 10, TS_COLOR_WHITE_BRIGHT, TS_COLOR_BLACK, 1);

		Logger::getInstance()->error("Testing state 1 (2)");
		testState(1, 50, 12, TS_COLOR_GREEN, TS_COLOR_RED_BRIGHT, 8);

		removeGraphicsState(20, 9, 1, 10, NULL);
		assertEquals(2, m_graphicsState.size(), "Test graphics state size (4)");
		removeGraphicsState(21, 13, 1, 14, NULL);
		assertEquals(2, m_graphicsState.size(), "Test graphics state size (5)");

		removeGraphicsState(20, 9, 1, 11, NULL);
		assertEquals(1, m_graphicsState.size(), "Test graphics state size (6)");

		Logger::getInstance()->error("Testing state 0 (3)");
		testState(1, 50, 12, TS_COLOR_GREEN, TS_COLOR_RED_BRIGHT, 8);

		removeGraphicsState(1, 8, 12, 13, NULL);
		assertEquals(0, m_graphicsState.size(), "Test graphics state size (7)");
	}
};

void testInit(TerminalState *state)
{
	assertEquals(80, state->getDisplayScreenSize().getX(), "Test initial screen width");
	assertEquals(40, state->getDisplayScreenSize().getY(), "Test initial screen height");

	assertEquals(1, state->getCursorLocation().getX(), "Test initial cursor X");
	assertEquals(1, state->getCursorLocation().getY(), "Test initial cursor Y");

	assertEquals(1, state->getTopMargin(), "Test initial margin top");
	assertEquals(40, state->getBottomMargin(), "Test initial margin bottom");

	assertEquals(0, state->getTerminalModeFlags(), "Test initial terminal flags");
	assertEquals(0, state->getGraphicsModeFlags(), "Test initial graphics flags");
	assertEquals(0, (int)state->getCharset(), "Test initial charset");

	assertEquals(40, state->getBufferScreenHeight(), "Test initial buffer height");
	assertEquals(0, state->getBufferTopLineIndex(), "Test initial buffer top line index");

	for (int i = 0; i < 40; i++)
	{
		assertEquals(0, state->getBufferLine(i)->size(), "Test initial lines");
	}
}

void testCursor(TerminalState *state)
{
	state->setCursorLocation(5, 20);
	assertEquals(5, state->getCursorLocation().getX(), "Test cursor X");
	assertEquals(20, state->getCursorLocation().getY(), "Test cursor Y");
	assertEquals(5, state->getDisplayCursorLocation().getX(), "Test cursor display X");
	assertEquals(20, state->getDisplayCursorLocation().getY(), "Test cursor display Y");

	state->setCursorLocation(0, 40);
	assertEquals(1, state->getCursorLocation().getX(), "Test cursor X (2)");
	assertEquals(40, state->getCursorLocation().getY(), "Test cursor Y (2)");

	state->setCursorLocation(80, 40);
	assertEquals(80, state->getCursorLocation().getX(), "Test cursor X (3)");
	assertEquals(40, state->getCursorLocation().getY(), "Test cursor Y (3)");

	state->setCursorLocation(81, 41);
	assertEquals(80, state->getCursorLocation().getX(), "Test cursor X (4)");
	assertEquals(40, state->getCursorLocation().getY(), "Test cursor Y (4)");

	assertEquals(80, state->getDisplayCursorLocation().getX(), "Test cursor display X (2)");
	assertEquals(40, state->getDisplayCursorLocation().getY(), "Test cursor display Y (2)");
}

void testOrigin(TerminalState *state)
{
	state->setMargin(3, 30);
	assertEquals(1, state->getCursorLocation().getX(), "Test cursor with origin X");
	assertEquals(1, state->getCursorLocation().getX(), "Test cursor with origin Y");
	state->setCursorLocation(10, 21);
	assertEquals(10, state->getCursorLocation().getX(), "Test cursor with origin X (2)");
	assertEquals(21, state->getCursorLocation().getY(), "Test cursor with origin Y (2)");
	assertEquals(10, state->getDisplayCursorLocation().getX(), "Test display cursor with origin X");
	assertEquals(21, state->getDisplayCursorLocation().getY(), "Test display cursor with origin Y");

	state->addTerminalModeFlags(TS_TM_ORIGIN);
	assertEquals(1, state->getCursorLocation().getX(), "Test cursor with origin X (3)");
	assertEquals(1, state->getCursorLocation().getY(), "Test cursor with origin Y (3)");

	state->setCursorLocation(5, 17);
	assertEquals(5, state->getCursorLocation().getX(), "Test cursor with origin X (4)");
	assertEquals(17, state->getCursorLocation().getY(), "Test cursor with origin Y (4)");
	state->addTerminalModeFlags(TS_TM_ORIGIN);
	assertEquals(5, state->getCursorLocation().getX(), "Test cursor with origin X (5)");
	assertEquals(17, state->getCursorLocation().getY(), "Test cursor with origin Y (5)");
	assertEquals(5, state->getDisplayCursorLocation().getX(), "Test display cursor with origin X");
	assertEquals(19, state->getDisplayCursorLocation().getY(), "Test display cursor with origin Y");

	state->setCursorLocation(81, 31);
	assertEquals(80, state->getCursorLocation().getX(), "Test cursor with origin X (6)");
	assertEquals(28, state->getCursorLocation().getY(), "Test cursor with origin Y (6)");
	state->setCursorLocation(23, 1);
	assertEquals(23, state->getCursorLocation().getX(), "Test cursor with origin X (7)");
	assertEquals(1, state->getCursorLocation().getY(), "Test cursor with origin Y (7)");

	state->removeTerminalModeFlags(TS_TM_ORIGIN);
	assertEquals(1, state->getCursorLocation().getX(), "Test cursor with origin X (8)");
	assertEquals(1, state->getCursorLocation().getY(), "Test cursor with origin Y (8)");
}

void testCursorMoveBackward(TerminalState *state)
{
	state->setMargin(3, 30);
	state->setCursorLocation(40, 20);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor backward X");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor backward Y");

	state->moveCursorBackward(0);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor backward X (2)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor backward Y (2)");

	state->moveCursorBackward(-1);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor backward X (3)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor backward Y (3)");

	state->moveCursorBackward(2);
	assertEquals(38, state->getCursorLocation().getX(), "Test move cursor backward X (4)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor backward Y (4)");

	state->moveCursorBackward(36);
	assertEquals(2, state->getCursorLocation().getX(), "Test move cursor backward X (5)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor backward Y (5)");

	state->moveCursorBackward(1);
	assertEquals(1, state->getCursorLocation().getX(), "Test move cursor backward X (6)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor backward Y (6)");

	state->moveCursorBackward(1);
	assertEquals(1, state->getCursorLocation().getX(), "Test move cursor backward X (7)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor backward Y (7)");
}

void testCursorMoveForward(TerminalState *state)
{
	state->setMargin(3, 30);
	state->setCursorLocation(40, 20);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor forward X");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor forward Y");

	state->moveCursorForward(0);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor forward X (2)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor forward Y (2)");

	state->moveCursorForward(-1);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor forward X (3)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor forward Y (3)");

	state->moveCursorForward(2);
	assertEquals(42, state->getCursorLocation().getX(), "Test move cursor forward X (4)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor forward Y (4)");

	state->moveCursorForward(36);
	assertEquals(78, state->getCursorLocation().getX(), "Test move cursor forward X (5)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor forward Y (5)");

	state->moveCursorForward(2);
	assertEquals(80, state->getCursorLocation().getX(), "Test move cursor forward X (6)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor forward Y (6)");

	state->moveCursorForward(1);
	assertEquals(80, state->getCursorLocation().getX(), "Test move cursor forward X (7)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor forward Y (7)");
}

void testCursorMoveUp(TerminalState *state)
{
	state->setMargin(3, 30);
	state->setCursorLocation(40, 20);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up X");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor up Y");

	state->moveCursorUp(0);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up X (2)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor up Y (2)");

	state->moveCursorUp(-1);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up X (3)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor up Y (3)");

	state->moveCursorUp(2);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up X (4)");
	assertEquals(18, state->getCursorLocation().getY(), "Test move cursor up Y (4)");

	state->moveCursorUp(14);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up X (5)");
	assertEquals(4, state->getCursorLocation().getY(), "Test move cursor up Y (5)");

	state->moveCursorUp(1);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up X (6)");
	assertEquals(3, state->getCursorLocation().getY(), "Test move cursor up Y (6)");

	state->moveCursorUp(1);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up X (7)");
	assertEquals(3, state->getCursorLocation().getY(), "Test move cursor up Y (7)");
}

void testCursorMoveDown(TerminalState *state)
{
	state->setMargin(3, 30);
	state->setCursorLocation(40, 20);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down X");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor down Y");

	state->moveCursorDown(0);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down X (2)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor down Y (2)");

	state->moveCursorDown(-1);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down X (3)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor down Y (3)");

	state->moveCursorDown(7);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down X (4)");
	assertEquals(27, state->getCursorLocation().getY(), "Test move cursor down Y (4)");

	state->moveCursorDown(2);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down X (5)");
	assertEquals(29, state->getCursorLocation().getY(), "Test move cursor down Y (5)");

	state->moveCursorDown(1);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down X (6)");
	assertEquals(30, state->getCursorLocation().getY(), "Test move cursor down Y (6)");

	state->moveCursorDown(1);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down X (7)");
	assertEquals(30, state->getCursorLocation().getY(), "Test move cursor down Y (7)");
}

void testCursorMoveUpOrigin(TerminalState *state)
{
	state->addTerminalModeFlags(TS_TM_ORIGIN);

	state->setMargin(3, 30);
	state->setCursorLocation(40, 20);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up origin X");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor up origin Y");

	state->moveCursorUp(0);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up origin X (2)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor up origin Y (2)");

	state->moveCursorUp(-1);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up origin X (3)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor up origin Y (3)");

	state->moveCursorUp(2);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up origin X (4)");
	assertEquals(18, state->getCursorLocation().getY(), "Test move cursor up origin Y (4)");

	state->moveCursorUp(14);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up origin X (5)");
	assertEquals(4, state->getCursorLocation().getY(), "Test move cursor up origin Y (5)");

	state->moveCursorUp(1);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up origin X (6)");
	assertEquals(3, state->getCursorLocation().getY(), "Test move cursor up origin Y (6)");

	state->moveCursorUp(3);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor up origin X (7)");
	assertEquals(1, state->getCursorLocation().getY(), "Test move cursor up origin Y (7)");

	state->removeTerminalModeFlags(TS_TM_ORIGIN);
}

void testCursorMoveDownOrigin(TerminalState *state)
{
	state->addTerminalModeFlags(TS_TM_ORIGIN);

	state->setMargin(3, 30);
	state->setCursorLocation(40, 20);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down origin X");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor down origin Y");

	state->moveCursorDown(0);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down origin X (2)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor down origin Y (2)");

	state->moveCursorDown(-1);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down origin X (3)");
	assertEquals(20, state->getCursorLocation().getY(), "Test move cursor down origin Y (3)");

	state->moveCursorDown(7);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down origin X (4)");
	assertEquals(27, state->getCursorLocation().getY(), "Test move cursor down origin Y (4)");

	state->moveCursorDown(4);
	assertEquals(40, state->getCursorLocation().getX(), "Test move cursor down origin X (5)");
	assertEquals(28, state->getCursorLocation().getY(), "Test move cursor down origin Y (5)");

	state->removeTerminalModeFlags(TS_TM_ORIGIN);
}

void testInsert(TerminalState *state)
{
	state->setMargin(1, 40);
	state->removeTerminalModeFlags(TS_TM_ORIGIN);

	for (int i = 0; i < 40; i++)
	{
		for (int j = 0; j < 80; j++)
		{
			state->insertChar(terminalBuffer[i][j], true, false);
		}
	}

	assertEquals(80, state->getCursorLocation().getX(), "Test insert X");
	assertEquals(1, state->getCursorLocation().getY(), "Test insert Y");

	assertEquals(80, (int)state->getBufferLine(0)->size(), "Test insert size");
	assertEquals(0, (int)state->getBufferLine(1)->size(), "Test insert size (2)");
	{
		char tmp[1024];
		assertEquals(0, state->getBufferLine(0)->copy(tmp, state->getBufferLine(0)->size()), "Test insert copy");
		assertEquals(terminalBuffer[0], 79, tmp, 79, "Test insert data");
		assertEquals(terminalBuffer[39][79], tmp[79], "Test insert data (2)");
	}

	state->setCursorLocation(1, 1);
	state->addTerminalModeFlags(TS_TM_AUTO_WRAP);

	for (int i = 0; i < 40; i++)
	{
		for (int j = 0; j < 80; j++)
		{
			if (i == 39 && j == 79)
			{
				//Don't write last character just yet to prevent scrolling.
				continue;
			}

			state->insertChar(terminalBuffer[i][j], true, false);
		}
	}

	assertEquals(80, state->getCursorLocation().getX(), "Test insert X");
	assertEquals(40, state->getCursorLocation().getY(), "Test insert Y");

	for (int i = 0; i < 40; i++)
	{
		if (i == 39)
		{
			assertEquals(79, (int)state->getBufferLine(i)->size(), "Test insert size line");
		}
		else
		{
			assertEquals(80, (int)state->getBufferLine(i)->size(), "Test insert size line");
		}
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test insert copy line");

			if (i == 39)
			{
				assertEquals(terminalBuffer[i], 79, tmp, 79, "Test insert data line");
			}
			else
			{
				assertEquals(terminalBuffer[i], 80, tmp, 80, "Test insert data line");
			}
		}
	}

	//Write last character.
	state->insertChar(terminalBuffer[39][79], true, false);
	assertEquals(81, state->getCursorLocation().getX(), "Test insert X");
	assertEquals(40, state->getCursorLocation().getY(), "Test insert Y");

	state->moveCursorNextLine();

	for (int i = 0; i < 39; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test insert size line scroll");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test insert copy line scroll");
			assertEquals(terminalBuffer[i + 1], 80, tmp, 80, "Test insert data line scroll");
		}
	}

	assertEquals(0, (int)state->getBufferLine(39)->size(), "Test insert size scroll");

	state->moveCursorDown(2, false);

	for (int i = 0; i < 39; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test insert size line scroll");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test insert copy line scroll");
			assertEquals(terminalBuffer[i + 1], 80, tmp, 80, "Test insert data line scroll");
		}
	}

	assertEquals(0, (int)state->getBufferLine(39)->size(), "Test insert size scroll");

	//Scroll down
	state->moveCursorDown(2, true);

	for (int i = 0; i < 37; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test insert size line scroll");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test insert copy line scroll");
			assertEquals(terminalBuffer[i + 3], 80, tmp, 80, "Test insert data line scroll");
		}
	}

	assertEquals(0, (int)state->getBufferLine(39)->size(), "Test insert size scroll");
	assertEquals(0, (int)state->getBufferLine(38)->size(), "Test insert size scroll");
	assertEquals(0, (int)state->getBufferLine(37)->size(), "Test insert size scroll");

	state->setCursorLocation(1, 1);
	state->moveCursorUp(2, false);

	for (int i = 0; i < 37; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test insert size line scroll");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test insert copy line scroll");
			assertEquals(terminalBuffer[i + 3], 80, tmp, 80, "Test insert data line scroll");
		}
	}

	assertEquals(0, (int)state->getBufferLine(39)->size(), "Test insert size scroll");
	assertEquals(0, (int)state->getBufferLine(38)->size(), "Test insert size scroll");
	assertEquals(0, (int)state->getBufferLine(37)->size(), "Test insert size scroll");

	state->moveCursorUp(3, true);

	for (int i = 0; i < 36; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i + 3)->size(), "Test insert size line scroll");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i + 3)->copy(tmp, state->getBufferLine(i + 3)->size()), "Test insert copy line scroll");
			assertEquals(terminalBuffer[i + 3], 80, tmp, 80, "Test insert data line scroll");
		}
	}
}

void testExpandedBuffer(TerminalState *state)
{
	state->setMargin(1, 40);
	state->removeTerminalModeFlags(TS_TM_ORIGIN);

	state->setNumBufferLines(80);

	for (int i = 0; i < 40; i++)
	{
		for (int j = 0; j < 80; j++)
		{
			state->insertChar(terminalBuffer[i][j], true, false);
		}
	}

	state->moveCursorNextLine();

	assertEquals(1, state->getBufferTopLineIndex(), "Test expanded buffer");

	for (int i = 0; i < 40; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test expanded buffer data");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test expanded buffer data");
			assertEquals(terminalBuffer[i], 80, tmp, 80, "Test expanded buffer data");
		}
	}

	state->moveCursorDown(3, true);

	assertEquals(4, state->getBufferTopLineIndex(), "Test expanded buffer");

	for (int i = 0; i < 40; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test expanded buffer data");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test expanded buffer data");
			assertEquals(terminalBuffer[i], 80, tmp, 80, "Test expanded buffer data");
		}
	}

	state->moveCursorDown(36, true);
	assertEquals(40, state->getBufferTopLineIndex(), "Test expanded buffer");

	for (int i = 0; i < 40; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test expanded buffer data");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test expanded buffer data");
			assertEquals(terminalBuffer[i], 80, tmp, 80, "Test expanded buffer data");
		}
	}

	state->moveCursorDown(2, true);
	assertEquals(40, state->getBufferTopLineIndex(), "Test expanded buffer");

	for (int i = 0; i < 38; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test expanded buffer data");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test expanded buffer data");
			assertEquals(terminalBuffer[i + 2], 80, tmp, 80, "Test expanded buffer data");
		}
	}

	state->setCursorLocation(1, 1);
	assertEquals(40, state->getBufferTopLineIndex(), "Test expanded buffer");

	for (int i = 0; i < 38; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test expanded buffer data");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test expanded buffer data");
			assertEquals(terminalBuffer[i + 2], 80, tmp, 80, "Test expanded buffer data");
		}
	}

	state->moveCursorUp(3, true);
	assertEquals(37, state->getBufferTopLineIndex(), "Test expanded buffer");

	for (int i = 0; i < 38; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test expanded buffer data");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test expanded buffer data");
			assertEquals(terminalBuffer[i + 2], 80, tmp, 80, "Test expanded buffer data");
		}
	}

	state->moveCursorUp(37, true);
	assertEquals(0, state->getBufferTopLineIndex(), "Test expanded buffer");

	for (int i = 0; i < 38; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test expanded buffer data");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test expanded buffer data");
			assertEquals(terminalBuffer[i + 2], 80, tmp, 80, "Test expanded buffer data");
		}
	}

	state->moveCursorUp(2, true);
	assertEquals(0, state->getBufferTopLineIndex(), "Test expanded buffer");

	for (int i = 0; i < 38; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i + 2)->size(), "Test expanded buffer data");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i + 2)->copy(tmp, state->getBufferLine(i + 2)->size()), "Test expanded buffer data");
			assertEquals(terminalBuffer[i + 2], 80, tmp, 80, "Test expanded buffer data");
		}
	}
}

void testErase(TerminalState *state)
{
	state->setMargin(1, 40);
	state->removeTerminalModeFlags(TS_TM_ORIGIN);
	state->addTerminalModeFlags(TS_TM_AUTO_WRAP);
	state->setNumBufferLines(40);
	state->moveCursorDown(1);

	assertEquals(1, state->getCursorLocation().getX(), "Test erase cursor X");
	assertEquals(2, state->getCursorLocation().getY(), "Test erase cursor Y");
	assertEquals(0, state->getBufferTopLineIndex(), "Test erase buffer");

	for (int i = 0; i < 39; i++)
	{
		for (int j = 0; j < 80; j++)
		{
			state->insertChar(terminalBuffer[i][j], true, false);
		}
	}

	state->moveCursorNextLine();

	assertEquals(1, state->getCursorLocation().getX(), "Test erase cursor X");
	assertEquals(40, state->getCursorLocation().getY(), "Test erase cursor Y");
	assertEquals(0, state->getBufferTopLineIndex(), "Test erase buffer");

	for (int i = 0; i < 39; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test erase buffer data");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test erase buffer data");
			assertEquals(terminalBuffer[i], 80, tmp, 80, "Test erase buffer data");
		}
	}

	state->eraseCurrentLine();

	for (int i = 0; i < 39; i++)
	{
		assertEquals(80, (int)state->getBufferLine(i)->size(), "Test erase buffer data");
		{
			char tmp[1024];
			assertEquals(0, state->getBufferLine(i)->copy(tmp, state->getBufferLine(i)->size()), "Test erase buffer data");
			assertEquals(terminalBuffer[i], 80, tmp, 80, "Test erase buffer data");
		}
	}

	state->setCursorLocation(32, 1);
	state->eraseCurrentLine();

	assertEquals(0, (int)state->getBufferLine(0)->size(), "Test erase buffer data (2)");

	state->setCursorLocation(32, 2);
	state->eraseCursorToEndOfLine();

	assertEquals(31, (int)state->getBufferLine(1)->size(), "Test erase buffer data (3)");
	{
		char tmp[1024], exp[1024];
		memset(exp, 0, sizeof(exp));
		memcpy(exp, terminalBuffer[1], 31);
		assertEquals(0, state->getBufferLine(1)->copy(tmp, state->getBufferLine(1)->size()), "Test erase buffer data (3)");
		assertEquals(exp, 31, tmp, 31, "Test erase buffer data (3)");
	}

	state->setCursorLocation(32, 3);
	state->eraseBeginOfLineToCursor();

	assertEquals(80, (int)state->getBufferLine(2)->size(), "Test erase buffer data (4)");
	{
		char tmp[1024], exp[1024];
		memset(exp, 0, sizeof(exp));
		memcpy(exp + 32, terminalBuffer[2] + 32, 48);
		assertEquals(0, state->getBufferLine(2)->copy(tmp, state->getBufferLine(2)->size()), "Test erase buffer data (4)");
		assertEquals(exp, 80, tmp, 80, "Test erase buffer data (4)");
	}

	state->setCursorLocation(60, 6);
	state->eraseBeginOfScreenToCursor();

	assertEquals(0, (int)state->getBufferLine(3)->size(), "Test erase buffer data (5)");
	assertEquals(0, (int)state->getBufferLine(4)->size(), "Test erase buffer data (6)");
	assertEquals(80, (int)state->getBufferLine(5)->size(), "Test erase buffer data (7)");
	{
		char tmp[1024], exp[1024];
		memset(exp, 0, sizeof(exp));
		memcpy(exp + 60, terminalBuffer[5] + 60, 20);
		assertEquals(0, state->getBufferLine(5)->copy(tmp, state->getBufferLine(5)->size()), "Test erase buffer data (7)");
		assertEquals(exp, 80, tmp, 80, "Test erase buffer data (7)");
	}

	state->setCursorLocation(60, 38);
	state->eraseCursorToEndOfScreen();

	assertEquals(59, (int)state->getBufferLine(37)->size(), "Test erase buffer data (8)");
	assertEquals(0, (int)state->getBufferLine(38)->size(), "Test erase buffer data (9)");
	assertEquals(0, (int)state->getBufferLine(39)->size(), "Test erase buffer data (10)");
	{
		char tmp[1024], exp[1024];
		memset(exp, 0, sizeof(exp));
		memcpy(exp, terminalBuffer[37], 59);
		assertEquals(0, state->getBufferLine(37)->copy(tmp, state->getBufferLine(37)->size()), "Test erase buffer data (10)");
		assertEquals(exp, 59, tmp, 59, "Test erase buffer data (10)");
	}

	state->eraseScreen();

	for (int i = 0; i < 40; i++)
	{
		assertEquals(0, (int)state->getBufferLine(i)->size(), "Test erase buffer data (11)");
	}
}


void testInsertShift(TerminalState *state)
{
	state->setDisplayScreenSize(4, 4);
	state->removeTerminalModeFlags(TS_TM_ORIGIN);
	state->addTerminalModeFlags(TS_TM_AUTO_WRAP);
	state->setNumBufferLines(4);
	state->eraseScreen();

	for (int i = 0; i < 4; i++)
	{
		assertEquals(0, (int)state->getBufferLine(i)->size(), "Test insert shift data size initial");
	}

	assertEquals(1, state->getCursorLocation().getX(), "Test insert shift cursor X");
	assertEquals(1, state->getCursorLocation().getY(), "Test insert shift cursor Y");

	state->insertChar('d', true, true, true);
	state->setCursorLocation(1, 1);
	state->insertChar('c', true, true, true);
	state->setCursorLocation(1, 1);
	state->insertChar('b', true, true, true);
	state->setCursorLocation(1, 1);
	state->insertChar('a', true, true, true);
	state->setCursorLocation(1, 1);

	assertEquals(4, (int)state->getBufferLine(0)->size(), "Test insert shift data");
	{
		char tmp[1024], exp[1024];
		assertEquals(0, state->getBufferLine(0)->copy(tmp, state->getBufferLine(0)->size()), "Test insert shift data");
		assertEquals("abcd", 4, tmp, 4, "Test insert shift data");
	}

	state->insertChar('1', true, true, true);
	state->insertChar('2', true, true, true);
	state->insertChar('3', true, true, true);
	state->insertChar('4', true, true, true);
	state->setCursorLocation(1, 1);

	assertEquals(4, (int)state->getBufferLine(1)->size(), "Test insert shift data (2)");
	{
		char tmp[1024], exp[1024];
		assertEquals(0, state->getBufferLine(1)->copy(tmp, state->getBufferLine(1)->size()), "Test insert shift data (2)");
		assertEquals("abcd", 4, tmp, 4, "Test insert shift data (2)");
	}

	assertEquals(4, (int)state->getBufferLine(0)->size(), "Test insert shift data (3)");
	{
		char tmp[1024], exp[1024];
		assertEquals(0, state->getBufferLine(0)->copy(tmp, state->getBufferLine(0)->size()), "Test insert shift data (3)");
		assertEquals("1234", 4, tmp, 4, "Test insert shift data (3)");
	}

	state->setCursorLocation(1, 4);
	state->insertChar('w', true, true, true);
	state->setCursorLocation(4, 2);
	state->insertChar('*', true, true, true);

	assertEquals(4, (int)state->getBufferLine(0)->size(), "Test insert shift data (4)");
	{
		char tmp[1024], exp[1024];
		assertEquals(0, state->getBufferLine(0)->copy(tmp, state->getBufferLine(0)->size()), "Test insert shift data (4)");
		assertEquals("1234", 4, tmp, 4, "Test insert shift data (4)");
	}

	assertEquals(4, (int)state->getBufferLine(1)->size(), "Test insert shift data (5)");
	{
		char tmp[1024], exp[1024];
		assertEquals(0, state->getBufferLine(1)->copy(tmp, state->getBufferLine(1)->size()), "Test insert shift data (5)");
		assertEquals("abc*", 4, tmp, 4, "Test insert shift data (5)");
	}

	assertEquals(1, (int)state->getBufferLine(2)->size(), "Test insert shift data (6)");
	{
		char tmp[1024], exp[1024];
		assertEquals(0, state->getBufferLine(2)->copy(tmp, state->getBufferLine(2)->size()), "Test insert shift data (6)");
		assertEquals("d", 1, tmp, 1, "Test insert shift data (6)");
	}

	assertEquals(2, (int)state->getBufferLine(3)->size(), "Test insert shift data (7)");
	{
		char tmp[1024], exp[1024];
		assertEquals(0, state->getBufferLine(3)->copy(tmp, state->getBufferLine(3)->size()), "Test insert shift data (7)");
		assertEquals(" w", 2, tmp, 2, "Test insert shift data (7)");
	}
}

void testDelete(TerminalState *state)
{
	state->setDisplayScreenSize(4, 4);
	state->removeTerminalModeFlags(TS_TM_ORIGIN);
	state->addTerminalModeFlags(TS_TM_AUTO_WRAP);
	state->setNumBufferLines(4);
	state->eraseScreen();

	for (int i = 0; i < 4; i++)
	{
		assertEquals(0, (int)state->getBufferLine(i)->size(), "Test insert shift data size initial");
	}

	state->insertChar('1', true, true, false);
	state->insertChar('2', true, true, false);
	state->insertChar('3', true, true, false);
	state->insertChar('4', true, true, false);
	state->insertChar('a', true, true, false);
	state->insertChar('b', true, true, false);
	state->insertChar('c', true, true, false);
	state->insertChar('d', true, true, false);

	state->setCursorLocation(1, 4);
	state->insertChar('w', true, true, false);
	state->insertChar('x', true, true, false);
	state->insertChar('y', true, true, false);

	state->setCursorLocation(1, 1);
	state->deleteChar(false, false);
	state->setCursorLocation(2, 1);
	state->deleteChar(false, false);

	assertEquals(0, (int)state->getBufferLine(2)->size(), "Test delete init data");
	assertEquals(3, (int)state->getBufferLine(3)->size(), "Test delete init data");

	assertEquals(4, (int)state->getBufferLine(0)->size(), "Test delete data");
	{
		char tmp[1024];
		char exp[] = { TerminalState::BLANK, TerminalState::BLANK, '3', '4' };
		assertEquals(0, state->getBufferLine(0)->copy(tmp, state->getBufferLine(0)->size()), "Test delete data");
		assertEquals(exp, 4, tmp, 4, "Test delete data");
	}

	state->setCursorLocation(1, 2);
	state->deleteChar(true, true);

	assertEquals(4, state->getCursorLocation().getX(), "Test delete cursor X");
	assertEquals(1, state->getCursorLocation().getY(), "Test delete cursor Y");

	state->deleteChar(true, true);

	assertEquals(3, state->getCursorLocation().getX(), "Test delete cursor X (2)");
	assertEquals(1, state->getCursorLocation().getY(), "Test delete cursor Y (2)");

	state->deleteChar(true, true);
	state->deleteChar(true, true);

	assertEquals(4, (int)state->getBufferLine(0)->size(), "Test delete data (2)");
	{
		char tmp[1024];
		assertEquals(0, state->getBufferLine(0)->copy(tmp, state->getBufferLine(0)->size()), "Test delete data (2)");
		assertEquals("abcd", 4, tmp, 4, "Test delete data (2)");
	}

	assertEquals(4, (int)state->getBufferLine(1)->size(), "Test delete data (3)");
	{
		char tmp[1024];
		char exp[] = { TerminalState::BLANK, TerminalState::BLANK, TerminalState::BLANK, TerminalState::BLANK };
		assertEquals(0, state->getBufferLine(1)->copy(tmp, state->getBufferLine(1)->size()), "Test delete data (3)");
		assertEquals(exp, 4, tmp, 4, "Test delete data (3)");
	}

	assertEquals(3, (int)state->getBufferLine(2)->size(), "Test delete data (4)");
	{
		char tmp[1024];
		assertEquals(0, state->getBufferLine(2)->copy(tmp, state->getBufferLine(2)->size()), "Test delete data (4)");
		assertEquals("wxy", 3, tmp, 3, "Test delete data (4)");
	}

	assertEquals(0, (int)state->getBufferLine(3)->size(), "Test delete data (5)");
}

void testVT(VTTerminalState *state)
{
	state->setDisplayScreenSize(10, 10);
	state->removeTerminalModeFlags(TS_TM_ORIGIN);
	state->addTerminalModeFlags(TS_TM_AUTO_WRAP);
	state->setNumBufferLines(10);
	state->eraseScreen();

	state->insertString("\x1B[1;1Habc\x1B[D123\x1B[3Dxyz", NULL);

	assertEquals(9, (int)state->getBufferLine(0)->size(), "Test vt data");
	{
		char tmp[1024];
		assertEquals(0, state->getBufferLine(0)->copy(tmp, state->getBufferLine(0)->size()), "Test vt data");
		assertEquals("abxyz123c", 9, tmp, 9, "Test vt data");
	}

	state->insertString("\x1B[2K", NULL);

	assertEquals(0, (int)state->getBufferLine(0)->size(), "Test vt data (2)");
}

void testGraphicsState()
{
	TerminalStateTest *testState = new TerminalStateTest();

	testState->testGraphicsState();

	delete testState;
}

int main()
{
	TerminalState *state = new VTTerminalState();
	Logger::getInstance()->setLogLevel(Logger::ERROR);

	state->enableShiftText(true);

	//Populate test data.
	for (int i = 0; i < 40; i++)
	{
		for (int j = 0; j < 80; j++)
		{
			terminalBuffer[i][j] = (rand() % 95) + 32;
		}
	}

	testInit(state);
	testCursor(state);
	testOrigin(state);
	testCursorMoveBackward(state);
	testCursorMoveForward(state);
	testCursorMoveUp(state);
	testCursorMoveDown(state);

	state->addTerminalModeFlags(TS_TM_ORIGIN);
	testCursorMoveBackward(state);
	testCursorMoveForward(state);
	state->removeTerminalModeFlags(TS_TM_ORIGIN);

	testCursorMoveUpOrigin(state);
	testCursorMoveDownOrigin(state);

	testInsert(state);
	testExpandedBuffer(state);
	testErase(state);
	testInsertShift(state);
	testDelete(state);
	testVT((VTTerminalState *)state);
	testGraphicsState();

	delete state;

	return 0;
}
