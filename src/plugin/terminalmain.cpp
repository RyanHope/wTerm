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

#include "sdl/sdlterminal.hpp"
#include "terminal/terminal.hpp"
#include "terminal/vtterminalstate.hpp"
#include "util/logger.hpp"

#include <PDL.h>

PDL_bool pushKeyEvent(PDL_JSParameters *params) {

	SDL_Event event;

	event.type=SDL_USEREVENT;
	event.user.code=2;
	event.user.data1=NULL;
	event.user.data2=NULL;
	SDL_PushEvent(&event);

	return PDL_TRUE;
}

int main()
{

	PDL_Init(0);

	PDL_RegisterJSHandler("pushKeyEvent", pushKeyEvent);

	PDL_JSRegistrationComplete();
	PDL_CallJS("ready", NULL, 0);

	SDLTerminal *sdlTerminal = new SDLTerminal();
	Terminal *terminal = new Terminal();

	sdlTerminal->start();

	if (sdlTerminal->isReady())
	{
		//Must set window size before starting the terminal.
		terminal->setWindowSize(sdlTerminal->getMaximumColumnsOfText(), sdlTerminal->getMaximumLinesOfText());

		if (terminal->start() == 0) //Non-blocking, creates a thread to read and a child process for slave device.
		{
			//Enable autowrap
			sdlTerminal->getTerminalState()->addTerminalModeFlags(TS_TM_AUTO_WRAP);

			sdlTerminal->setExtTerminal(terminal);
			terminal->setExtTerminal(sdlTerminal);

			sdlTerminal->run(); //Blocking.
		}
		else
		{
			Logger::getInstance()->fatal("TTY Terminal not started.");
		}
	}
	else
	{
		Logger::getInstance()->fatal("SDLTerminal not started.");
	}

	sdlTerminal->setExtTerminal(NULL);
	terminal->setExtTerminal(NULL);

	delete terminal;
	delete sdlTerminal;

	exit(0);
}
