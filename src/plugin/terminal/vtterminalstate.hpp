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

#ifndef ANSITERMINALSTATE_HPP__
#define ANSITERMINALSTATE_HPP__

#include "terminalstate.hpp"
#include "extterminal.hpp"

typedef enum
{
	VTTS_CURSOR_UP,
	VTTS_CURSOR_DOWN,
	VTTS_CURSOR_LEFT,
	VTTS_CURSOR_RIGHT
} VTTS_Cursor;

class ControlSeqParser;

/**
 * Terminal state information catered to VT100.
 */
class VTTerminalState : public TerminalState
{
protected:
	ControlSeqParser *m_parser;

	void processControlSeq(int nToken, int *values, int numValues, ExtTerminal *extTerminal);

public:
	VTTerminalState();
	virtual ~VTTerminalState();

	void insertString(const char *sStr, int len, ExtTerminal *extTerminal);
	void sendCursorCommand(VTTS_Cursor cursor, ExtTerminal *extTerminal);
};

#endif
