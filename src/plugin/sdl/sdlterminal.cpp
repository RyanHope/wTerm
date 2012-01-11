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

#include "sdl/sdlterminal.hpp"
#include "util/databuffer.hpp"
#include "terminal/terminal.hpp"

#include <GLES/gl.h>
#include <SDL/SDL_image.h>
#include <PDL.h>
#include <string.h>
#include <time.h>
#include <syslog.h>

// Convert mid-string null characters to blanks
static void stringify(char * str, size_t len) {
	for(unsigned i = 0; i < len; ++i)
		if (!str[i]) str[i] = ' ';
	str[len] = '\0';
}

SDLTerminal::SDLTerminal()
{
	m_terminalState = NULL;
	m_keyMod = TERM_KEYMOD_NONE;
	m_bCtrlKeyModHeld = false;
	m_bKeyModUsed = true;
	m_bKeyModLocked = false;
	m_config = new TerminalConfigManager();

	m_keyModShiftSurface = NULL;
	m_keyModCtrlSurface = NULL;
	m_keyModAltSurface = NULL;
	m_keyModFnSurface = NULL;
	m_keyModShiftLockedSurface = NULL;
	m_keyModCtrlLockedSurface = NULL;
	m_keyModAltLockedSurface = NULL;
	m_keyModFnLockedSurface = NULL;

	SDL_Color defaultColors[] = {
		{ 0, 0, 0 }, // COLOR_BLACK
		{ 187, 0, 0 }, // COLOR_RED
		{ 0, 187, 0 }, // COLOR_GREEN
		{ 187, 187, 0 }, // COLOR_YELLOW
		{ 0, 0, 187 }, // COLOR_BLUE
		{ 187, 0, 187 }, // COLOR_MAGENTA
		{ 0, 187, 187 }, // COLOR_CYAN
		{ 187, 187, 187 }, // COLOR_WHITE
		{ 85, 85, 85 }, // COLOR_BLACK_BRIGHT
		{ 255, 85, 85 }, // COLOR_RED_BRIGHT
		{ 85, 255, 85 }, // COLOR_GREEN_BRIGHT
		{ 255, 255, 85 }, // COLOR_YELLOW_BRIGHT
		{ 85, 85, 255 }, // COLOR_BLUE_BRIGHT
		{ 255, 85, 255 }, // COLOR_MAGENTA_BRIGHT
		{ 85, 255, 255 }, // COLOR_CYAN_BRIGHT
		{ 255, 255, 255 }, // COLOR_WHITE_BRIGHT
		{ 187, 187, 187 }, // COLOR_FOREGROUND
		{ 0, 0, 0 }, // COLOR_BACKGROUND
		{ 255, 255, 255 }, // COLOR_FOREGROUND_BRIGHT
		{ 0, 0, 0 }, // COLOR_BACKGROUND_BRIGHT
	};
	size_t colorCount = sizeof(defaultColors)/sizeof(defaultColors[0]);
	size_t mcolorCount = sizeof(m_colors)/sizeof(m_colors[0]);

	memset(m_colors, 0, sizeof(m_colors));
	for (int i = 0; i < colorCount && i < mcolorCount; ++i)
		m_colors[i] = defaultColors[i];

	m_config->parse("./terminal.config");

	initCharsets();
}

SDLTerminal::~SDLTerminal()
{
	if (m_terminalState != NULL)
	{
		delete m_terminalState;
	}

	if (m_keyModShiftSurface != NULL)
	{
		SDL_FreeSurface(m_keyModShiftSurface);
	}

	if (m_keyModCtrlSurface != NULL)
	{
		SDL_FreeSurface(m_keyModCtrlSurface);
	}

	if (m_keyModAltSurface != NULL)
	{
		SDL_FreeSurface(m_keyModAltSurface);
	}

	if (m_keyModFnSurface != NULL)
	{
		SDL_FreeSurface(m_keyModFnSurface);
	}

	if (m_keyModShiftLockedSurface != NULL)
	{
		SDL_FreeSurface(m_keyModShiftLockedSurface);
	}

	if (m_keyModCtrlLockedSurface != NULL)
	{
		SDL_FreeSurface(m_keyModCtrlLockedSurface);
	}

	if (m_keyModAltLockedSurface != NULL)
	{
		SDL_FreeSurface(m_keyModAltLockedSurface);
	}

	if (m_keyModFnLockedSurface != NULL)
	{
		SDL_FreeSurface(m_keyModFnLockedSurface);
	}

	if (m_config != NULL)
	{
		delete m_config;
	}
}

void SDLTerminal::updateDisplaySize()
{
	if (m_terminalState != NULL) 
	{
		m_terminalState->setDisplayScreenSize(getMaximumColumnsOfText(), getMaximumLinesOfText());
		m_terminalState->setMargin(1,m_terminalState->getDisplayScreenSize().getY());
		Terminal *extTerminal = (Terminal *)getExtTerminal();
		if (extTerminal != NULL)
			extTerminal->setWindowSize(getMaximumColumnsOfText(), getMaximumLinesOfText());
	}
}

void SDLTerminal::initCharsets()
{
	CharMapping_t lineDrawing;
	memset(&lineDrawing, 0, sizeof(lineDrawing));
	lineDrawing.map[96] = 9830;
	lineDrawing.map[97] = 9618;
	lineDrawing.map[98] = 9621;
	lineDrawing.map[99] = 9621;
	lineDrawing.map[100] = 9621;
	lineDrawing.map[101] = 9621;
	lineDrawing.map[102] = 176;
	lineDrawing.map[103] = 177;
	lineDrawing.map[104] = 9621;
	lineDrawing.map[105] = 9621;
	lineDrawing.map[106] = 9496;
	lineDrawing.map[107] = 9488;
	lineDrawing.map[108] = 9484;
	lineDrawing.map[109] = 9492;
	lineDrawing.map[110] = 9532;
	lineDrawing.map[111] = 9621;
	lineDrawing.map[112] = 9621;
	lineDrawing.map[113] = 9472;
	lineDrawing.map[114] = 9621;
	lineDrawing.map[115] = 9621;
	lineDrawing.map[116] = 9500;
	lineDrawing.map[117] = 9508;
	lineDrawing.map[118] = 9524;
	lineDrawing.map[119] = 9516;
	lineDrawing.map[120] = 9474;
	lineDrawing.map[121] = 8804;
	lineDrawing.map[122] = 8805;
	lineDrawing.map[123] = 960;
	lineDrawing.map[124] = 8800;
	lineDrawing.map[125] = 163;
	lineDrawing.map[126] = 183;
	lineDrawing.map[127] = 0;
	setCharMapping(TS_CS_G0_SPEC, lineDrawing);
	setCharMapping(TS_CS_G1_SPEC, lineDrawing);
}

int SDLTerminal::initCustom()
{
	m_terminalState = new VTTerminalState();

	updateDisplaySize();

	SDL_EnableUNICODE(1);

	if (SDL_EnableKeyRepeat(500, 35) != 0)
	{
		syslog(LOG_ERR, "Cannot enable keyboard repeat.");
		return -1;
	}

	m_keyModShiftSurface = IMG_Load("images/shkey.png");
	m_keyModCtrlSurface = IMG_Load("images/ctrlkey.png");
	m_keyModAltSurface = IMG_Load("images/altkey.png");
	m_keyModFnSurface = IMG_Load("images/fnkey.png");
	m_keyModShiftLockedSurface = IMG_Load("images/shkeylocked.png");
	m_keyModCtrlLockedSurface = IMG_Load("images/ctrlkeylocked.png");
	m_keyModAltLockedSurface = IMG_Load("images/altkeylocked.png");
	m_keyModFnLockedSurface = IMG_Load("images/fnkeylocked.png");

	if (m_keyModShiftSurface == NULL || m_keyModCtrlSurface == NULL
		|| m_keyModAltSurface == NULL || m_keyModFnSurface == NULL)
	{
		syslog(LOG_ERR, "Cannot create keyboard modifier image.");
		return -1;
	}

	if (m_keyModShiftLockedSurface == NULL || m_keyModCtrlLockedSurface == NULL
		|| m_keyModAltLockedSurface == NULL || m_keyModFnLockedSurface == NULL)
	{
		syslog(LOG_ERR, "Cannot create keyboard modifier locked image.");
		return -1;
	}

	SDL_SetAlpha(m_keyModShiftSurface, 0, 0);
	SDL_SetAlpha(m_keyModCtrlSurface, 0, 0);
	SDL_SetAlpha(m_keyModAltSurface, 0, 0);
	SDL_SetAlpha(m_keyModFnSurface, 0, 0);
	SDL_SetAlpha(m_keyModShiftLockedSurface, 0, 0);
	SDL_SetAlpha(m_keyModCtrlLockedSurface, 0, 0);
	SDL_SetAlpha(m_keyModAltLockedSurface, 0, 0);
	SDL_SetAlpha(m_keyModFnLockedSurface, 0, 0);

	setReady(true);

	return 0;
}

void SDLTerminal::toggleKeyMod(Term_KeyMod_t keyMod)
{
	if (!m_bKeyModUsed)
	{
		if (m_keyMod == keyMod && !m_bKeyModLocked)
		{
			m_bKeyModLocked = true;
		}
		else if (m_keyMod == keyMod)
		{
			disableKeyMod();
		}
		else
		{
			m_keyMod = keyMod;
			m_bKeyModLocked = false;
		}
	}
}

void SDLTerminal::disableKeyMod()
{
	m_keyMod = TERM_KEYMOD_NONE;
	m_bKeyModUsed = false;
	m_bKeyModLocked = false;
}

void SDLTerminal::handleMouseEvent(SDL_Event &event)
{
	/*switch (event.type)
	{
		case SDL_MOUSEBUTTONDOWN:
			m_bCtrlKeyModHeld = true;
			m_bKeyModUsed = false;
			break;

		case SDL_MOUSEBUTTONUP:
			m_bCtrlKeyModHeld = false;

			toggleKeyMod(TERM_KEYMOD_CTRL);
			redraw();
			break;
	}*/
}

void SDLTerminal::handleKeyboardEvent(SDL_Event &event)
{
	char c[2] = { '\0', '\0' };
	int nKey;
	SDLKey sym = event.key.keysym.sym;
	SDLMod mod = event.key.keysym.mod;
	Uint16 unicode = event.key.keysym.unicode;
	bool bPrint = true;

	ExtTerminal *extTerminal = getExtTerminal();

	if (extTerminal == NULL || !extTerminal->isReady())
	{
		extTerminal = this;
	}

	switch (event.type)
	{
		case SDL_KEYUP:
			//Toggle modifiers.
			if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
			{
				toggleKeyMod(TERM_KEYMOD_SHIFT);
				redraw();
			}
			else if (sym == SDLK_RCTRL || sym == SDLK_LCTRL)
			{
				toggleKeyMod(TERM_KEYMOD_CTRL);
				redraw();
			}
			else if (sym == SDLK_RALT || sym == SDLK_LALT)
			{
				toggleKeyMod(TERM_KEYMOD_ALT);
				redraw();
			}
			else if (sym == SDLK_MODE)
			{
				toggleKeyMod(TERM_KEYMOD_FN);
				redraw();
			}
			break;
		case SDL_KEYDOWN:
			if (sym == SDLK_UP)
			{
				m_terminalState->sendCursorCommand(VTTS_CURSOR_UP, extTerminal);
			}
			else if (sym == SDLK_DOWN)
			{
				m_terminalState->sendCursorCommand(VTTS_CURSOR_DOWN, extTerminal);
			}
			else if (sym == SDLK_RIGHT)
			{
				m_terminalState->sendCursorCommand(VTTS_CURSOR_RIGHT, extTerminal);
			}
			else if (sym == SDLK_LEFT)
			{
				m_terminalState->sendCursorCommand(VTTS_CURSOR_LEFT, extTerminal);
			}
			else if (sym == SDLK_F1)
			{
				extTerminal->insertData("\033OP\0", 4);
			}
			else if (sym == SDLK_F2)
			{
				extTerminal->insertData("\033OQ\0", 4);
			}
			else if (sym == SDLK_F3)
			{
				extTerminal->insertData("\033OR\0", 4);
			}
			else if (sym == SDLK_F4)
			{
				extTerminal->insertData("\033OS\0", 4);
			}
			else if (sym == SDLK_F5)
			{
				extTerminal->insertData("\033[15~\0", 5);
			}
			else if (sym == SDLK_F6)
			{
				extTerminal->insertData("\033[17~\0", 5);
			}
			else if (sym == SDLK_F7)
			{
				extTerminal->insertData("\033[18~\0", 5);
			}
			else if (sym == SDLK_F8)
			{
				extTerminal->insertData("\033[19~\0", 5);
			}
			else if (sym == SDLK_F9)
			{
				extTerminal->insertData("\033[20~\0", 5);
			}
			else if (sym == SDLK_F10)
			{
				extTerminal->insertData("\033[21~\0", 5);
			}
			else if (sym == SDLK_F11)
			{
				extTerminal->insertData("\033[23~\0", 5);
			}
			else if (sym == SDLK_F12)
			{
				extTerminal->insertData("\033[24~\0", 5);
			}
			else if (sym == SDLK_RETURN)
			{
				if ((m_terminalState->getTerminalModeFlags() & TS_TM_NEW_LINE) > 0)
					extTerminal->insertData("\r\n", 2);
				else
					extTerminal->insertData("\r", 1);
			}
			//Printable characters.
			else if ((unicode & 0xFF80) == 0 )
			{
				nKey = -1;
				c[0] = (unicode & 0x7F);

				//Screen area takes precedence over Sym key.
				if (m_bCtrlKeyModHeld || m_keyMod == TERM_KEYMOD_CTRL)
				{
					nKey = m_config->getKeyBinding(TERM_KEYMOD_CTRL, c[0]);
				}
				else if ((mod & KMOD_MODE) || m_keyMod == TERM_KEYMOD_FN)
				{
					nKey = m_config->getKeyBinding(TERM_KEYMOD_FN, c[0]);
				}
				else if ((mod & KMOD_ALT) == 0 && m_keyMod == TERM_KEYMOD_ALT)
				{
					nKey = m_config->getKeyBinding(TERM_KEYMOD_ALT, c[0]);
				}
				else if ((mod & KMOD_SHIFT) == 0 && m_keyMod == TERM_KEYMOD_SHIFT)
				{
					nKey = m_config->getKeyBinding(TERM_KEYMOD_SHIFT, c[0]);
				}

				if (nKey >= 0)
				{
					bPrint = false;

					//WASD as arrow keys.
					if (nKey == SDLK_UP)
					{
						m_terminalState->sendCursorCommand(VTTS_CURSOR_UP, extTerminal);
					}
					else if (nKey == SDLK_DOWN)
					{
						m_terminalState->sendCursorCommand(VTTS_CURSOR_DOWN, extTerminal);
					}
					else if (nKey == SDLK_RIGHT)
					{
						m_terminalState->sendCursorCommand(VTTS_CURSOR_RIGHT, extTerminal);
					}
					else if (nKey == SDLK_LEFT)
					{
						m_terminalState->sendCursorCommand(VTTS_CURSOR_LEFT, extTerminal);
					}
					else if (nKey < 256)
					{
						bPrint = true;
						c[0] = nKey;
					}
				}

				if (bPrint)
				{
					extTerminal->insertData(c, 1);
				}
			}
			
			if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT || sym == SDLK_RCTRL || sym == SDLK_LCTRL
				|| sym == SDLK_RALT || sym == SDLK_LALT || sym == SDLK_MODE)
			{
				//Holding a key modifier while pressing a key modifier nullifies the current key modifier.
				if ((mod & KMOD_ALT) || (mod & KMOD_CTRL) || (mod & KMOD_SHIFT) || (mod & KMOD_MODE) || m_bCtrlKeyModHeld)
				{
					if (m_keyMod != TERM_KEYMOD_NONE)
					{
						disableKeyMod();
						redraw();
					}

					m_bKeyModUsed = true;
				}
				else
				{
					m_bKeyModUsed = false;
				}
			}
			else if (!m_bKeyModLocked)
			{
				if (m_keyMod != TERM_KEYMOD_NONE)
				{
					disableKeyMod();
					redraw();
				}

				m_bKeyModUsed = true;
			}
			break;
		default:
			break;
	}
}

void SDLTerminal::redraw()
{
	m_terminalState->lock();

	char *sBuffer = NULL;
	size_t size = (m_terminalState->getDisplayScreenSize().getX() + 1) * sizeof(char);
	DataBuffer *databuffer;
	int nTopLineIndex = m_terminalState->getBufferTopLineIndex();
	int nEndLine = nTopLineIndex + m_terminalState->getDisplayScreenSize().getY();
	int nLine = 1;
	int nResult = 0;

	TSLineGraphicsState_t **states = NULL;
	TSLineGraphicsState_t *tmpState = NULL;
	int nNumStates = 0;
	int nMaxStates = m_terminalState->getDisplayScreenSize().getX();
	int nStartIdx;
	TSLineGraphicsState_t defState = m_terminalState->getDefaultGraphicsState();

	setGraphicsState(defState);
	m_reverse = (m_terminalState->getTerminalModeFlags() & TS_TM_SCREEN);
	clearScreen(m_reverse ? defState.foregroundColor : defState.backgroundColor);

	if (size <= 0)
	{
		nResult = -1;
	}

	if (nResult == 0)
	{
		sBuffer = (char *)malloc(size);

		if (sBuffer == NULL)
		{
			nResult = -1;
		}

		states = (TSLineGraphicsState_t **)malloc(nMaxStates * sizeof(TSLineGraphicsState_t *));

		if (states == NULL)
		{
			nResult = -1;
		}
	}

	if (nResult == 0)
	{

		startTextGL(m_terminalState->getDisplayScreenSize().getX() + 1,
				m_terminalState->getDisplayScreenSize().getY() + 1);

		for (int i = nTopLineIndex; i < nEndLine; i++)
		{
			m_terminalState->getLineGraphicsState(nLine, states, nNumStates, nMaxStates);
			databuffer = m_terminalState->getBufferLine(i);
			memset(sBuffer, 0, size);
			databuffer->copy(sBuffer, size - 1);
			stringify(sBuffer, databuffer->size());

			if (nNumStates > 0)
			{
				nStartIdx = 0;

				for (int j = nNumStates - 1; j >= 0; j--)
				{
					if (j < nMaxStates)
					{
						tmpState = states[j];

						if (tmpState->nLine != nLine)
						{
							nStartIdx = 0;
						}
						else
						{
							nStartIdx = tmpState->nColumn - 1;
						}

						if (nStartIdx < 0)
						{
							nStartIdx = 0;
						}
						else if (nStartIdx >= strlen(sBuffer))
						{
							continue;
						}

						setGraphicsState(*tmpState);
					}
					else
					{
						setGraphicsState(defState);
						nStartIdx = 0;
					}

					if (strlen(sBuffer + nStartIdx) > 0)
					{
						printText(nStartIdx + 1, nLine, sBuffer + nStartIdx);
						sBuffer[nStartIdx] = '\0';
					}

					if (nStartIdx == 0)
					{
						break;
					}
				}
			}
			else
			{
				if (strlen(sBuffer) > 0)
				{
					printText(1, nLine, sBuffer);
				}
			}

			nLine++;
		}

		endTextGL();

		if (m_terminalState->getTerminalModeFlags() & TS_TM_CURSOR)
			drawCursor(m_terminalState->getCursorLocation().getX(), m_terminalState->getCursorLocation().getY());
	}

	if (sBuffer != NULL)
	{
		free(sBuffer);
	}

	if (states != NULL)
	{
		free(states);
	}

	m_terminalState->unlock();

	if (m_keyMod != TERM_KEYMOD_NONE)
	{
		int nY = m_surface->h - 32;

		glColor4f(1.0f, 1.0f, 1.0f, 0.70f);

		if (m_keyMod == TERM_KEYMOD_CTRL)
		{
			if (m_bKeyModLocked)
			{
				drawSurface(0, nY, m_keyModCtrlLockedSurface);
			}
			else
			{
				drawSurface(0, nY, m_keyModCtrlSurface);
			}
		}
		else if (m_keyMod == TERM_KEYMOD_FN)
		{
			if (m_bKeyModLocked)
			{
				drawSurface(0, nY, m_keyModFnLockedSurface);
			}
			else
			{
				drawSurface(0, nY, m_keyModFnSurface);
			}
		}
		else if (m_keyMod == TERM_KEYMOD_ALT)
		{
			if (m_bKeyModLocked)
			{
				drawSurface(0, nY, m_keyModAltLockedSurface);
			}
			else
			{
				drawSurface(0, nY, m_keyModAltSurface);
			}
		}
		else if (m_keyMod == TERM_KEYMOD_SHIFT)
		{
			if (m_bKeyModLocked)
			{
				drawSurface(0, nY, m_keyModShiftLockedSurface);
			}
			else
			{
				drawSurface(0, nY, m_keyModShiftSurface);
			}
		}
	}
}

void SDLTerminal::refresh()
{
	SDL_Event event;

	setDirty(BUFFER_DIRTY_BIT);

	memset(&event, 0, sizeof(event));
	event.type = SDL_VIDEOEXPOSE;

	SDL_PushEvent(&event);
}

/**
 * Accepts NULL terminating string.
 */
void SDLTerminal::insertData(const char *data, size_t size)
{
	if (size > 0)
	{
		m_terminalState->insertString(data, getExtTerminal());
		refresh();
	}
}

TerminalState *SDLTerminal::getTerminalState()
{
	return m_terminalState;
}

SDL_Color SDLTerminal::getColor(TSColor_t color)
{
	return m_colors[color];
}

void SDLTerminal::setColor(TSColor_t color, int r, int g, int b)
{
/*
	// Should probably do something like this to prevent worst case scenarios
	// though maybe just make m_colors a map
	int colorSize = sizeof(m_colors) / sizeof(SDL_Color);
	if (color >= colorSize)
		return;
*/
	m_colors[color].r = r;
	m_colors[color].g = g;
	m_colors[color].b = b;
	setDirty(FONT_DIRTY_BIT);
}

void SDLTerminal::setForegroundColor(TSColor_t color)
{
	m_foregroundColor = color;
	setDirty(FOREGROUND_COLOR_DIRTY_BIT);
}

void SDLTerminal::setBackgroundColor(TSColor_t color)
{
	m_backgroundColor = color;
	setDirty(BACKGROUND_COLOR_DIRTY_BIT);
}

void SDLTerminal::setGraphicsState(TSLineGraphicsState_t &state)
{
	if ((state.nGraphicsMode & TS_GM_NEGATIVE) > 0)
	{
		setForegroundColor(state.backgroundColor);
	}
	else
	{
		setForegroundColor(state.foregroundColor);
	}

	if ((state.nGraphicsMode & TS_GM_NEGATIVE) > 0)
	{
		if (state.foregroundColor>7 && state.foregroundColor<16)
			setBackgroundColor((TSColor_t)(state.foregroundColor-8));
		else if (state.foregroundColor>17)
			setBackgroundColor((TSColor_t)(state.foregroundColor-2));
		else
			setBackgroundColor(state.foregroundColor);
	}
	else
	{
		setBackgroundColor(state.backgroundColor);
	}

	m_bBold = ((state.nGraphicsMode & TS_GM_BOLD) > 0);
	m_bUnderline = ((state.nGraphicsMode & TS_GM_UNDERSCORE) > 0);
	m_bBlink = ((state.nGraphicsMode & TS_GM_BLINK) > 0);

	m_slot1 = state.g0charset;
	m_slot2 = state.g1charset;
}
