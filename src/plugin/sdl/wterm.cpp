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

#include "sdl/wterm.hpp"
#include "terminal/terminal.hpp"
#include "util/utf8.hpp"
#include "util/utils.hpp"

#include <SDL/SDL_image.h>
#include <string.h>
#include <time.h>
#include <syslog.h>

#define HP_BT_LEFT	18
#define HP_BT_UP	19
#define HP_BT_RIGHT	20
#define HP_BT_DOWN	21

#define HP_BT_PLUGIN_UP		0xE0A0
#define HP_BT_PLUGIN_DOWN	0xE0A1
#define HP_BT_PLUGIN_LEFT	0xE0A2
#define HP_BT_PLUGIN_RIGHT	0xE0A3

WTerm::WTerm()
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
	for (unsigned int i = 0; i < colorCount && i < mcolorCount; ++i)
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
}

WTerm::~WTerm()
{
	if (m_terminalState != NULL)
	{
		delete m_terminalState;
	}
}

void WTerm::updateDisplaySize()
{
	if (m_terminalState != NULL)
	{
		unsigned int cols = m_fontgl.cols(), rows = m_fontgl.rows();

		m_terminalState->setDisplayScreenSize(cols, rows);
		m_terminalState->setMargin(1, rows);
		Terminal *extTerminal = (Terminal *)getExtTerminal();
		if (extTerminal != NULL)
			extTerminal->setWindowSize(cols, rows);
	}
}

int WTerm::initCustom()
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

void WTerm::handleMouseEvent(SDL_Event &event)
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

void WTerm::injectData(const char *data)
{
	ExtTerminal *extTerminal = getExtTerminal();
	extTerminal->insertData(data);
}

void WTerm::handleKeyboardEvent(SDL_Event &event)
{
	SDLKey sym = event.key.keysym.sym;
	SDLMod mod = event.key.keysym.mod;
	Uint16 unicode = event.key.keysym.unicode;

	bool snapBottom = true;

	char c[4];

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
		case SDLK_WORLD_30:
			if (mod & KMOD_MODE) extTerminal->insertData(".");
			break;
		case HP_BT_UP:
		case HP_BT_PLUGIN_UP:
		case SDLK_UP:
			if (!(mod & KMOD_MODE)) {
				m_terminalState->sendCursorCommand(VTTS_CURSOR_UP, extTerminal);
				break;
			}
		case SDLK_PAGEUP:
			if (mod & KMOD_SHIFT) {
				m_terminalState->setScrollOffset(m_terminalState->getScrollOffset()+10);
				snapBottom = false;
				refresh();
			} else
				extTerminal->insertData("\x1B[5~");
			break;
		case HP_BT_DOWN:
		case HP_BT_PLUGIN_DOWN:
		case SDLK_DOWN:
			if (!(mod & KMOD_MODE)) {
				m_terminalState->sendCursorCommand(VTTS_CURSOR_DOWN, extTerminal);
				break;
			}
		case SDLK_PAGEDOWN:
			if (mod & KMOD_SHIFT) {
				m_terminalState->setScrollOffset(m_terminalState->getScrollOffset()-10);
				snapBottom = false;
				refresh();
			} else
				extTerminal->insertData("\x1B[6~");
			break;
		case HP_BT_RIGHT:
		case HP_BT_PLUGIN_RIGHT:
		case SDLK_RIGHT:
			if (!(mod & KMOD_MODE)) {
				m_terminalState->sendCursorCommand(VTTS_CURSOR_RIGHT, extTerminal);
				break;
			}
			/* fall through for KMOD_MODE */
		case SDLK_END:
			extTerminal->insertData("\x1B[F");
			break;
		case HP_BT_LEFT:
		case HP_BT_PLUGIN_LEFT:
		case SDLK_LEFT:
			if (!(mod & KMOD_MODE)) {
				m_terminalState->sendCursorCommand(VTTS_CURSOR_LEFT, extTerminal);
				break;
			}
			/* fall through for KMOD_MODE */
		case SDLK_HOME:
			extTerminal->insertData("\x1B[H");
			break;
		case SDLK_INSERT:
			extTerminal->insertData("\x1B[2~");
			break;
		case SDLK_DELETE:
			extTerminal->insertData("\x1B[3~");
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
		case SDLK_NUMLOCK:
		case SDLK_CAPSLOCK:
		case SDLK_SCROLLOCK:
		case SDLK_RSHIFT:
		case SDLK_LSHIFT:
		case SDLK_RCTRL:
		case SDLK_LCTRL:
		case SDLK_RALT:
		case SDLK_LALT:
		case SDLK_RMETA:
		case SDLK_LMETA:
		case SDLK_LSUPER:
		case SDLK_RSUPER:
		case SDLK_MODE:
			snapBottom = false;
			break;
		case SDLK_UNKNOWN:
		default:
			// Failed to handle based on 'sym', look to unicode:
			// Accordingly, if no unicode value, we're done here.
			if (!unicode) break;

			if (0 != (mod & KMOD_CTRL) && unicode < 0x80) {
				// Encode control by masking (lowercase/uppercase only differs in 0x20 bit)
				c[0] = 0x1f & unicode;
				c[1] = 0;
			} else if (0 != (mod & KMOD_ALT) && unicode < 0x80) {
				c[0] = '\x1b';
				c[1] = unicode;
				c[2] = 0;
			} else {
				writeUtf8Char(c, unicode);
			}
			extTerminal->insertData(c);
			break;
		}
		break;
	default:
		break;
	}

	if ((event.type == SDL_KEYDOWN) && (snapBottom) && (m_terminalState->getScrollOffset() != 0)) {
		m_terminalState->setScrollOffset(0);
		refresh();
	}
}

void WTerm::redraw()
{
	m_terminalState->lock();

	TSGraphicsState defState;

	m_reverse = (m_terminalState->getTerminalModeFlags() & TS_TM_SCREEN);

	// Clear the entire screen to the default background color
	clearScreen(m_reverse ? defState.foregroundColor : defState.backgroundColor);

	bool hasBlinkText = false;

	m_fontgl.clearText();

	unsigned int nRow = 1;
	ScreenBuffer::LinesIterator l = m_terminalState->screen_start(), le = m_terminalState->screen_end();
	for ( ; l != le; ++l, ++nRow) {
		unsigned int nCol = 1;
		ScreenBuffer::Line::const_iterator r = l->begin(), re = l->end();

		for ( ; r != re; ++r, ++nCol) {
			hasBlinkText |= (r->graphics.nGraphicsMode & TS_GM_BLINK) != 0;
			printCharacter(nCol, nRow, *r);
		}
	}

	m_fontgl.setCursor(m_terminalState->getTerminalModeFlags() & TS_TM_CURSOR && (m_terminalState->getScrollOffset() == 0),
		m_terminalState->getDisplayCursorLocation().getY()-1,
		m_terminalState->getDisplayCursorLocation().getX()-1,
		m_reverse ? TS_COLOR_BACKGROUND : TS_COLOR_FOREGROUND,
		m_terminalState->getCursorStyle());

	m_terminalState->unlock();

	// don't keep lock while drawing
	m_fontgl.drawGL(doBlink);

	m_bNeedsBlink = hasBlinkText || (0 == m_terminalState->getCursorStyle() % 2);
}

void WTerm::redrawBlinked()
{
	m_terminalState->lock();

	TSGraphicsState defState;

	m_reverse = (m_terminalState->getTerminalModeFlags() & TS_TM_SCREEN);

	m_terminalState->unlock();

	// Clear the entire screen to the default background color
	clearScreen(m_reverse ? defState.foregroundColor : defState.backgroundColor);

	// don't keep lock while drawing
	m_fontgl.drawGL(doBlink);
}

void WTerm::refresh()
{
	SDL_Event event;

	memset(&event, 0, sizeof(event));
	event.type = SDL_VIDEOEXPOSE;

	SDL_PushEvent(&event);
}

/**
 * Accepts NULL terminating string.
 */
void WTerm::insertData(const char *data, int len)
{
	if (data) {
		m_terminalState->insertString(data, len, getExtTerminal());
	} else {
		refresh();
	}
}

TerminalState *WTerm::getTerminalState()
{
	return m_terminalState;
}

SDL_Color WTerm::getColor(TSColor color)
{
	return m_colors[color];
}

void WTerm::setKey(TSInput key, const char *cmd) {
	m_keys[key] = std::string(cmd);
}

void WTerm::setColor(TSColor color, int r, int g, int b)
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
	setDirty(COLOR_DIRTY_BIT);
}

void WTerm::setScrollBufferLines(int lines)
{
	m_terminalState->setScrollBufferLines(lines);
	refresh();
}

int WTerm::getScrollBufferLines()
{
	return m_terminalState->getScrollBufferLines();
}
