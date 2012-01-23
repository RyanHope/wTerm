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

#ifndef EXTTERMINAL_HPP__
#define EXTTERMINAL_HPP__

#include <string.h>

/**
 * Interface for external terminals to transfer data.
 */
class ExtTerminal
{
private:
	bool m_bExtTerminalReady;

public:
	ExtTerminal()
	{
		m_bExtTerminalReady = false;
	}

	/**
	 * Insert data into this terminal.
	 */
	virtual void insertData(const char *data, int len) = 0;
	virtual void insertData(const char *data) {
		insertData(data, strlen(data));
	}

	bool isReady()
	{
		return m_bExtTerminalReady;
	}

	void setReady(bool bReady)
	{
		m_bExtTerminalReady = bReady;
	}
};

class ExtTerminalContainer
{
private:
	ExtTerminal *m_extTerminal;

public:
	ExtTerminalContainer()
	{
		m_extTerminal = NULL;
	}

	void setExtTerminal(ExtTerminal *extTerminal)
	{
		m_extTerminal = extTerminal;
	}

	ExtTerminal *getExtTerminal()
	{
		return m_extTerminal;
	}
};

#endif
