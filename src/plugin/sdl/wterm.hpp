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

#ifndef WTERM_HPP__
#define WTERM_HPP__

#include "sdl/sdlcore.hpp"
#include "terminal/extterminal.hpp"

#include <vector>
#include <string>

/**
 * SDL Terminal front end.
 */
class WTerm : public SDL::SDLCore, public ExtTerminal, public ExtTerminalContainer
{
protected:
	VTTerminalState *m_terminalState;

	void handleKeyboardEvent(SDL_Event &event);
	void handleMouseEvent(SDL_Event &event);
	int initCustom();

private:
	SDL_Color m_colors[TS_COLOR_MAX];
	std::vector<std::string> m_keys;

public:
	WTerm();
	virtual ~WTerm();

	void refresh();

	void redraw();
	void redrawBlinked();

	void injectData(const char *data);
	void insertData(const char *data, int len);
	TerminalState *getTerminalState();

	SDL_Color getColor(TSColor color);
	void setColor(TSColor color, int r, int g, int b);
	void setKey(TSInput key, const char *command);
	void setScrollBufferLines(int lines);
	int getScrollBufferLines();

	void updateDisplaySize();
};

#endif
