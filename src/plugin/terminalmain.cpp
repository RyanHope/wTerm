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

#include "sdl/sdlcore.hpp"
#include "sdl/sdlterminal.hpp"
#include "terminal/terminal.hpp"
#include "terminal/vtterminalstate.hpp"

#include <syslog.h>
#include <PDL.h>

SDLTerminal *sdlTerminal;

PDL_bool getFontSize(PDL_JSParameters *params) {
	char *reply = 0;
	asprintf(&reply, "%d", sdlTerminal->getFontSize());
	PDL_JSReply(params, reply);
	free(reply);
	return PDL_TRUE;
}

PDL_bool getDimensions(PDL_JSParameters *params) {
	char *reply = 0;
	asprintf(&reply, "[%d,%d]", sdlTerminal->getMaximumLinesOfText(), sdlTerminal->getMaximumColumnsOfText());
	PDL_JSReply(params, reply);
	free(reply);
	return PDL_TRUE;
}

PDL_bool pushKeyEvent(PDL_JSParameters *params) {

	SDL_Event event;

	int type = PDL_GetJSParamInt(params, 0);

	event.type = type ? SDL_KEYDOWN : SDL_KEYUP;
	event.key.type = type ? SDL_KEYDOWN : SDL_KEYUP;
	event.key.state = type ? SDL_PRESSED : SDL_RELEASED;
	event.key.keysym.mod = SDL_GetModState();
	event.key.keysym.sym = (SDLKey)PDL_GetJSParamInt(params, 1);
	event.key.keysym.unicode = PDL_GetJSParamString(params, 2)[0];

	int ret = SDL_PushEvent(&event);

	syslog(LOG_WARNING, "%d %d %d %d", event.type, event.key.state, event.key.keysym.sym, ret);

	return PDL_TRUE;
}

int main()
{

	openlog("us.ryanhope.wterm.plugin", LOG_PID, LOG_USER);

	sdlTerminal = new SDLTerminal();
	Terminal *terminal = new Terminal();

	sdlTerminal->start();

	PDL_Init(0);

	PDL_RegisterJSHandler("pushKeyEvent", pushKeyEvent);
	PDL_RegisterJSHandler("getDimensions", getDimensions);
	PDL_RegisterJSHandler("getFontSize", getFontSize);

	PDL_JSRegistrationComplete();
	PDL_CallJS("ready", NULL, 0);

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
			syslog(LOG_CRIT, "TTY Terminal not started.");
		}
	}
	else
	{
		syslog(LOG_CRIT, "SDLTerminal not started.");
	}

	sdlTerminal->setExtTerminal(NULL);
	terminal->setExtTerminal(NULL);

	delete terminal;
	delete sdlTerminal;

	exit(0);
}
