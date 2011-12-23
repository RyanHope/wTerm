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

#ifndef TERMINALCONFIGMANAGER_HPP__
#define TERMINALCONFIGMANAGER_HPP__

#include "util/configmanager.hpp"

#include <list>
#include <map>

typedef enum
{
	TERM_KEYMOD_NONE = 0,
	TERM_KEYMOD_SHIFT,
	TERM_KEYMOD_CTRL,
	TERM_KEYMOD_ALT,
	TERM_KEYMOD_FN,
	TERM_KEYMOD_MAX
} Term_KeyMod_t;

class TerminalConfigManager : public ConfigManager
{
protected:
	std::map<int, int> *m_keyBindings[TERM_KEYMOD_MAX];

	void freeKeyBindings();
	int generateKeyBindings();
	int generateDefaultKeyBindings();
	int parseString(const char *src, int *values, int nMaxValues);

public:
	TerminalConfigManager();
	virtual ~TerminalConfigManager();

	int getKeyBinding(Term_KeyMod_t keyMod, int nInput);
	virtual int parse(const char *sFileName);
};

#endif
