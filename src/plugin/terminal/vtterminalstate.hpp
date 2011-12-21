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

#ifndef ANSITERMINALSTATE_HPP__
#define ANSITERMINALSTATE_HPP__

#include "seqparser.hpp"
#include "terminalstate.hpp"
#include "extterminal.hpp"

typedef enum
{
	VTTS_CURSOR_UP,
	VTTS_CURSOR_DOWN,
	VTTS_CURSOR_LEFT,
	VTTS_CURSOR_RIGHT
} VTTS_Cursor_t;

/**
 * Terminal state information catered to VT100.
 */
class VTTerminalState : public TerminalState
{
protected:
	ControlSeqParser *m_parser;

	void processControlSeq(int nToken, int *values, int numValues, ExtTerminal *extTerminal);
	bool processNonPrintableChar(char &c);

public:
	VTTerminalState();
	virtual ~VTTerminalState();

	void insertString(const char *sStr, ExtTerminal *extTerminal);
	void sendCursorCommand(VTTS_Cursor_t cursor, ExtTerminal *extTerminal);
};

#endif
