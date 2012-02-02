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

#include "sdl/sdlcore.hpp"
#include "sdl/wterm.hpp"
#include "terminal/terminal.hpp"
#include "terminal/vtterminalstate.hpp"
#include "util/utf8.hpp"

#include <syslog.h>
#include <PDL.h>

WTerm *wTerm;



PDL_bool inject(PDL_JSParameters *params) {
	const char *cmd = PDL_GetJSParamString(params, 0);
	wTerm->injectData(cmd);
	wTerm->injectData("\n");
	return PDL_TRUE;
}

PDL_bool setScrollBufferLines(PDL_JSParameters *params) {
	wTerm->setScrollBufferLines(PDL_GetJSParamInt(params, 0));
	return PDL_TRUE;
}

PDL_bool setActive(PDL_JSParameters *params) {
	int active = PDL_GetJSParamInt(params, 0);
	wTerm->setActive(active);
	if (active == 0)
		wTerm->stopKeyRepeat();
	else
		wTerm->refresh();
	return PDL_TRUE;
}

PDL_bool setKey(PDL_JSParameters *params) {
	wTerm->setKey((TSInput)PDL_GetJSParamInt(params, 0), PDL_GetJSParamString(params, 1));
	return PDL_TRUE;
}

PDL_bool setColor(PDL_JSParameters *params) {

	TSColor color = (TSColor)PDL_GetJSParamInt(params, 0);
	int r = PDL_GetJSParamInt(params, 1);
	int g = PDL_GetJSParamInt(params, 2);
	int b = PDL_GetJSParamInt(params, 3);

	wTerm->setColor(color,r,g,b);

	wTerm->refresh();

	return PDL_TRUE;
}

PDL_bool setFontSize(PDL_JSParameters *params) {
	wTerm->setFontSize(PDL_GetJSParamInt(params, 0));
	char *reply = 0;
	asprintf(&reply, "%d", wTerm->getFontSize());
	PDL_JSReply(params, reply);
	free(reply);

	wTerm->updateDisplaySize();
	wTerm->refresh();

	return PDL_TRUE;
}

PDL_bool getFontSize(PDL_JSParameters *params) {
	char *reply = 0;
	asprintf(&reply, "%d", wTerm->getFontSize());
	PDL_JSReply(params, reply);
	free(reply);
	return PDL_TRUE;
}

PDL_bool getDimensions(PDL_JSParameters *params) {
	char *reply = 0;
	asprintf(&reply, "[%d,%d]", wTerm->getMaximumLinesOfText(), wTerm->getMaximumColumnsOfText());
	PDL_JSReply(params, reply);
	free(reply);
	return PDL_TRUE;
}

PDL_bool pushKeyEvent(PDL_JSParameters *params) {
	SDL_Event event;

	event.type = PDL_GetJSParamInt(params, 0) ? SDL_KEYDOWN : SDL_KEYUP;
	event.key.type = event.type;
	event.key.keysym.sym = (SDLKey)PDL_GetJSParamInt(params, 1);
	event.key.keysym.unicode = parseUtf8Char(PDL_GetJSParamString(params, 2));

	wTerm->fakeKeyEvent(event);

	char *reply = 0;
	asprintf(&reply, "%d", SDL_GetModState());
	PDL_JSReply(params, reply);
	free(reply);

	return PDL_TRUE;
}

int main(int argc, const char* argv[])
{
	openlog("us.ryanhope.wterm.plugin", LOG_PID, LOG_USER);
	setlogmask(LOG_UPTO(LOGLEVEL));

	PDL_Init(0);

	wTerm = new WTerm();
	Terminal *terminal = new Terminal();
	terminal->path = strdup(argv[0]);
	char *e = strrchr(terminal->path, '/');
	*e = 0;

	chdir(terminal->path);

	wTerm->setFontSize((argc > 1 && atoi(argv[1])) ? atoi(argv[1]) : 12);
	wTerm->start();

	if (argc > 2)
		terminal->setExec(argv[2]);
	else
		terminal->setExec("login -f root");

	PDL_RegisterJSHandler("setScrollBufferLines", setScrollBufferLines);
	PDL_RegisterJSHandler("inject", inject);
	PDL_RegisterJSHandler("setActive", setActive);
	PDL_RegisterJSHandler("setKey", setKey);
	PDL_RegisterJSHandler("setColor", setColor);
	PDL_RegisterJSHandler("pushKeyEvent", pushKeyEvent);
	PDL_RegisterJSHandler("getDimensions", getDimensions);
	PDL_RegisterJSHandler("getFontSize", getFontSize);
	PDL_RegisterJSHandler("setFontSize", setFontSize);

	PDL_JSRegistrationComplete();

	if (wTerm->isReady())
	{
		wTerm->setExtTerminal(terminal);
		terminal->setExtTerminal(wTerm);

		//Set defaults
		wTerm->getTerminalState()->resetTerminal();

		//Must set window size before starting the terminal.
		terminal->setWindowSize(wTerm->getMaximumColumnsOfText(), wTerm->getMaximumLinesOfText());

		if (terminal->start() == 0) //Non-blocking, creates a thread to read and a child process for slave device.
		{
			while (!terminal->isReady())
				sched_yield();
			sched_yield();

			if (PDL_IsPlugin()) PDL_CallJS("ready", NULL, 0);

			wTerm->run(); //Blocking.
		}
		else
		{
			syslog(LOG_CRIT, "TTY Terminal not started.");
		}
	}
	else
	{
		syslog(LOG_CRIT, "WTerm not started.");
	}

	wTerm->setExtTerminal(NULL);
	terminal->setExtTerminal(NULL);

	delete terminal;
	delete wTerm;

	closelog();

	exit(0);
}
