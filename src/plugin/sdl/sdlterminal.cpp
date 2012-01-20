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

// Some HP BT keycodes
#define HP_BT_LEFT 18
#define HP_BT_UP 19
#define HP_BT_RIGHT 20
#define HP_BT_DOWN 21

// These are different when used as a plugin.
// Credit to Brybry for finding these.
#define HP_BT_PLUGIN_UP    0xE0A0
#define HP_BT_PLUGIN_DOWN  0xE0A1
#define HP_BT_PLUGIN_LEFT  0xE0A2
#define HP_BT_PLUGIN_RIGHT 0xE0A3

SDLTerminal::SDLTerminal()
{
	m_terminalState = NULL;

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

	m_keys.clear();
	m_keys.push_back("\033OP");
	m_keys.push_back("\033OQ");
	m_keys.push_back("\033OR");
	m_keys.push_back("\033OS");
	m_keys.push_back("\033[15~");
	m_keys.push_back("\033[17~");
	m_keys.push_back("\033[18~");
	m_keys.push_back("\033[19~");
	m_keys.push_back("\033[20~");
	m_keys.push_back("\033[21~");
	m_keys.push_back("\033[23~");
	m_keys.push_back("\033[24~");

	initCharsets();
}

SDLTerminal::~SDLTerminal()
{
	if (m_terminalState != NULL)
	{
		delete m_terminalState;
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

	setReady(true);

	return 0;
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

void SDLTerminal::injectData(const char *data)
{
	ExtTerminal *extTerminal = getExtTerminal();
	extTerminal->insertData(data);
}

void SDLTerminal::handleKeyboardEvent(SDL_Event &event)
{
	SDLKey sym = event.key.keysym.sym;
	SDLMod mod = event.key.keysym.mod;
	Uint16 unicode = event.key.keysym.unicode;

	char c[3] = { '\0', '\0', '\0'};

	ExtTerminal *extTerminal = getExtTerminal();

	if (extTerminal == NULL || !extTerminal->isReady())
	{
		extTerminal = this;
	}

	switch (event.type)
	{
	case SDL_KEYDOWN:
		switch(sym)
		{
		case HP_BT_UP:
		case HP_BT_PLUGIN_UP:
		case SDLK_UP:
			m_terminalState->sendCursorCommand(VTTS_CURSOR_UP, extTerminal);
			break;
		case HP_BT_DOWN:
		case HP_BT_PLUGIN_DOWN:
		case SDLK_DOWN:
			m_terminalState->sendCursorCommand(VTTS_CURSOR_DOWN, extTerminal);
			break;
		case HP_BT_RIGHT:
		case HP_BT_PLUGIN_RIGHT:
		case SDLK_RIGHT:
			if (mod & KMOD_MODE)
				extTerminal->insertData("\x1B[F");
			else
				m_terminalState->sendCursorCommand(VTTS_CURSOR_RIGHT, extTerminal);
			break;
		case HP_BT_LEFT:
		case HP_BT_PLUGIN_LEFT:
		case SDLK_LEFT:
			if (mod & KMOD_MODE)
				extTerminal->insertData("\x1B[H");
			else
				m_terminalState->sendCursorCommand(VTTS_CURSOR_LEFT, extTerminal);
			break;
		case SDLK_ESCAPE:
			extTerminal->insertData("\x1b");
			break;
		case SDLK_F1:
			extTerminal->insertData(m_keys[TS_INPUT_F1].c_str());
			break;
		case SDLK_F2:
			extTerminal->insertData(m_keys[TS_INPUT_F2].c_str());
			break;
		case SDLK_F3:
			extTerminal->insertData(m_keys[TS_INPUT_F3].c_str());
			break;
		case SDLK_F4:
			extTerminal->insertData(m_keys[TS_INPUT_F4].c_str());
			break;
		case SDLK_F5:
			extTerminal->insertData(m_keys[TS_INPUT_F5].c_str());
			break;
		case SDLK_F6:
			extTerminal->insertData(m_keys[TS_INPUT_F6].c_str());
			break;
		case SDLK_F7:
			extTerminal->insertData(m_keys[TS_INPUT_F7].c_str());
			break;
		case SDLK_F8:
			extTerminal->insertData(m_keys[TS_INPUT_F8].c_str());
			break;
		case SDLK_F9:
			extTerminal->insertData(m_keys[TS_INPUT_F9].c_str());
			break;
		case SDLK_F10:
			extTerminal->insertData(m_keys[TS_INPUT_F10].c_str());
			break;
		case SDLK_F11:
			extTerminal->insertData(m_keys[TS_INPUT_F11].c_str());
			break;
		case SDLK_F12:
			extTerminal->insertData(m_keys[TS_INPUT_F12].c_str());
			break;
		case SDLK_TAB:
			extTerminal->insertData("\t");
			break;
		case SDLK_RETURN:
			if (m_terminalState->getTerminalModeFlags() & TS_TM_NEW_LINE)
				extTerminal->insertData("\r\n");
			else
				extTerminal->insertData("\r");
			break;
		case SDLK_BACKSPACE:
			if (m_terminalState->getTerminalModeFlags() & TS_TM_BACKSPACE)
				extTerminal->insertData("\x08");
			else
				extTerminal->insertData("\x7F");
			break;
		default:
			// Failed to handle based on 'sym', look to unicode:
			// Accordingly, if no unicode value, we're done here.
			if (!unicode) break;
			// We don't yet handle international characters
			if ((unicode & 0xFF80) != 0) {
				syslog(LOG_INFO, "An International Character.");
				break;
			}

			c[0] = unicode & 0x7F;

			if (mod & KMOD_CTRL) {
        // SDL gives us capitalized alpha, make them lowercase
        if (c[0] >= 'A' && c[0] <= 'Z') {
          c[0] -= 'A' - 'a';
        }

        // Encode control by masking
        c[0] &= 0x1f;
			} else if (mod & KMOD_ALT) {
					c[1] = c[0];
					c[0] = '\x1b';
			}
			extTerminal->insertData(c);

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
void SDLTerminal::insertData(const char *data)
{
	m_terminalState->insertString(data, getExtTerminal());
	refresh();
}

TerminalState *SDLTerminal::getTerminalState()
{
	return m_terminalState;
}

SDL_Color SDLTerminal::getColor(TSColor_t color)
{
	return m_colors[color];
}

void SDLTerminal::setKey(TSInput_t key, const char *cmd) {
	m_keys[key] = std::string(cmd);
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
