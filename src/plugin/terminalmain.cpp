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

#include "sdl/sdlcore.hpp"
#include "sdl/sdlterminal.hpp"
#include "terminal/terminal.hpp"
#include "terminal/vtterminalstate.hpp"

#include <syslog.h>
#include <PDL.h>

SDLTerminal *sdlTerminal;

PDL_bool setActive(PDL_JSParameters *params) {
	int active = PDL_GetJSParamInt(params, 0);
	sdlTerminal->setActive(active);
	return PDL_TRUE;
}

PDL_bool setKey(PDL_JSParameters *params) {
	sdlTerminal->setKey((TSInput_t)PDL_GetJSParamInt(params, 0), PDL_GetJSParamString(params, 1));
	return PDL_TRUE;
}

PDL_bool setColor(PDL_JSParameters *params) {

	TSColor_t color = (TSColor_t)PDL_GetJSParamInt(params, 0);
	int r = PDL_GetJSParamInt(params, 1);
	int g = PDL_GetJSParamInt(params, 2);
	int b = PDL_GetJSParamInt(params, 3);

	sdlTerminal->setColor(color,r,g,b);

	sdlTerminal->refresh();

	return PDL_TRUE;
}

PDL_bool setFontSize(PDL_JSParameters *params) {
	sdlTerminal->setFontSize(PDL_GetJSParamInt(params, 0));
	char *reply = 0;
	asprintf(&reply, "%d", sdlTerminal->getFontSize());
	PDL_JSReply(params, reply);
	free(reply);

	sdlTerminal->updateDisplaySize();
	sdlTerminal->refresh();

	return PDL_TRUE;
}

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

	event.type = PDL_GetJSParamInt(params, 0) ? SDL_KEYDOWN : SDL_KEYUP;
	event.key.type = event.type;
	event.key.keysym.sym = (SDLKey)PDL_GetJSParamInt(params, 1);
	event.key.keysym.unicode = PDL_GetJSParamString(params, 2)[0];

	sdlTerminal->fakeKeyEvent(event);

	char *reply = 0;
	asprintf(&reply, "%d", SDL_GetModState());
	PDL_JSReply(params, reply);
	free(reply);

	return PDL_TRUE;
}

int main(int argc, const char* argv[])
{

	openlog("us.ryanhope.wterm.plugin", LOG_PID, LOG_USER);
	setlogmask(LOG_UPTO((argc > 2 && atoi(argv[2])>=LOG_EMERG && atoi(argv[2])<=LOG_DEBUG) ? atoi(argv[2]) : LOGLEVEL));

	PDL_Init(0);

	sdlTerminal = new SDLTerminal();
	Terminal *terminal = new Terminal();

	sdlTerminal->start();
	sdlTerminal->setFontSize((argc > 1 && atoi(argv[1])) ? atoi(argv[1]) : 12);

	PDL_RegisterJSHandler("setActive", setActive);
	PDL_RegisterJSHandler("setKey", setKey);
	PDL_RegisterJSHandler("setColor", setColor);
	PDL_RegisterJSHandler("pushKeyEvent", pushKeyEvent);
	PDL_RegisterJSHandler("getDimensions", getDimensions);
	PDL_RegisterJSHandler("getFontSize", getFontSize);
	PDL_RegisterJSHandler("setFontSize", setFontSize);

	PDL_JSRegistrationComplete();
	PDL_CallJS("ready", NULL, 0);

	if (sdlTerminal->isReady())
	{
		//Must set window size before starting the terminal.
		terminal->setWindowSize(sdlTerminal->getMaximumColumnsOfText(), sdlTerminal->getMaximumLinesOfText());

		if (terminal->start() == 0) //Non-blocking, creates a thread to read and a child process for slave device.
		{
			//Enable autowrap
			sdlTerminal->getTerminalState()->resetTerminal();

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

	closelog();

	exit(0);
}
