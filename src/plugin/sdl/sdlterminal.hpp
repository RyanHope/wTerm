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

#ifndef SDLTERMINAL_HPP__
#define SDLTERMINAL_HPP__

#include "sdl/sdlcore.hpp"
#include "terminal/extterminal.hpp"
#include "terminal/vtterminalstate.hpp"
#include "terminal/terminalconfigmanager.hpp"

/**
 * SDL Terminal front end.
 */
class SDLTerminal : public SDLCore, public ExtTerminal, public ExtTerminalContainer
{
protected:
	VTTerminalState *m_terminalState;
	TerminalConfigManager *m_config;
	Term_KeyMod_t m_keyMod;
	bool m_bKeyModUsed;
	bool m_bKeyModLocked;
	bool m_bCtrlKeyModHeld;

	SDL_Surface *m_keyModShiftSurface;
	SDL_Surface *m_keyModCtrlSurface;
	SDL_Surface *m_keyModAltSurface;
	SDL_Surface *m_keyModFnSurface;
	SDL_Surface *m_keyModShiftLockedSurface;
	SDL_Surface *m_keyModCtrlLockedSurface;
	SDL_Surface *m_keyModAltLockedSurface;
	SDL_Surface *m_keyModFnLockedSurface;

	void handleKeyboardEvent(SDL_Event &event);
	void handleMouseEvent(SDL_Event &event);
	int initCustom();
	void toggleKeyMod(Term_KeyMod_t keyMod);
	void disableKeyMod();
	void initCharsets();

private:
	SDL_Color m_colors[TS_COLOR_MAX];

public:
	SDLTerminal();
	virtual ~SDLTerminal();

	void refresh();

	void redraw();
	void insertData(const char *data, size_t size);
	TerminalState *getTerminalState();

	SDL_Color getColor(TSColor_t color);
	void setColor(TSColor_t color, int r, int g, int b);
	void setForegroundColor(TSColor_t color);
	void setBackgroundColor(TSColor_t color);
	void setGraphicsState(TSLineGraphicsState_t &state);

	void updateDisplaySize();
};

#endif
