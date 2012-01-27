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
#include "util/utf8.hpp"

#include <pwd.h>

#include <syslog.h>
#include <PDL.h>

SDLTerminal *sdlTerminal;

void setup_su(bool enable)
{
	if (enable)
	{
		system("mkdir -p /usr/local/bin");
		system("cp /bin/busybox /usr/local/bin/su");
		system("chmod 750 /usr/local/bin/su");
		system("chmod ug+s /usr/local/bin/su");
	}
	else
	{
		system("rm -rf /usr/local/bin/su");
	}
}

int isMountWritable(const char* dest) {
	int ret = 1;
	FILE *mountsfile;
	char tmpdest[4096];
	char tmpflags[4096];
	mountsfile = fopen("/proc/mounts", "r");
	if (mountsfile == NULL) {
		syslog(LOG_ERR, "error[fopen] in is_mounted()");
		ret = -1;
	} else {
		while (ret == 0 && fscanf(mountsfile, "%*s%s%*s%s%*d%*d", tmpdest, tmpflags) != EOF) {
			if (strcmp(dest,tmpdest)==0) {
				if (tmpflags[1] == 'w') ret = 1;
			}
		}
		fclose(mountsfile);
	}
	return ret;
}

bool hasPassword(const char *user)
{
	struct passwd *pw = getpwnam(user);
	return (strlen(pw->pw_passwd)==34);
}

int setPassword(const char *user, const char *password)
{
	char *cmd = 0;
	asprintf(&cmd, "echo -n \"%s:%s\" | chpasswd -m", user, password);
	system(cmd);
	if (cmd) free(cmd);
}

void addToGroup(const char *user, const char *group)
{
	char *cmd = 0;
	asprintf(&cmd, "if ! grep -q \"^%s:.*%s\" /etc/group; then sed -i -e 's/^%s:.*$/&,%s/' /etc/group; fi", group, user, group, user);
	system(cmd);
	if (cmd) free(cmd);
}

PDL_bool inject(PDL_JSParameters *params) {
	const char *cmd = PDL_GetJSParamString(params, 0);
	sdlTerminal->injectData(cmd);
	sdlTerminal->injectData("\n");
	return PDL_TRUE;
}

PDL_bool setScrollBufferLines(PDL_JSParameters *params) {
	sdlTerminal->setScrollBufferLines(PDL_GetJSParamInt(params, 0));
	return PDL_TRUE;
}

PDL_bool setActive(PDL_JSParameters *params) {
	int active = PDL_GetJSParamInt(params, 0);
	sdlTerminal->setActive(active);
	if (active == 0)
		sdlTerminal->stopKeyRepeat();
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
	event.key.keysym.unicode = parseUtf8Char(PDL_GetJSParamString(params, 2));

	sdlTerminal->fakeKeyEvent(event);

	char *reply = 0;
	asprintf(&reply, "%d", SDL_GetModState());
	PDL_JSReply(params, reply);
	free(reply);

	return PDL_TRUE;
}

PDL_bool userSetPassword(PDL_JSParameters *params) {

	const char *user = PDL_GetJSParamString(params, 0);
	const char *password = PDL_GetJSParamString(params, 1);

	int write = isMountWritable("/");
	if (!write) system("mount -o remount,rw /");
	setPassword(user, password);
	if (!write) system("mount -o remount,ro /");

	return PDL_TRUE;
}

PDL_bool userHasPassword(PDL_JSParameters *params) {

	char *reply = 0;
	asprintf(&reply, "%d", hasPassword(PDL_GetJSParamString(params, 0)));
	PDL_JSReply(params, reply);
	free(reply);

	return PDL_TRUE;
}

PDL_bool userAddToGroup(PDL_JSParameters *params) {

	const char *user = PDL_GetJSParamString(params, 0);
	const char *group = PDL_GetJSParamString(params, 1);

	int write = isMountWritable("/");
	if (!write) system("mount -o remount,rw /");
	addToGroup(user, group);
	if (!write) system("mount -o remount,ro /");

	return PDL_TRUE;
}

PDL_bool setupSU(PDL_JSParameters *params) {
	int write = isMountWritable("/");
	if (!write) system("mount -o remount,rw /");
	setup_su(PDL_GetJSParamInt(params, 0));
	if (!write) system("mount -o remount,ro /");
	return PDL_TRUE;
}

void setup_wterm_user()
{
	int write = isMountWritable("/");
	if (!write) system("mount -o remount,rw /");
	system("adduser -D wterm -h /var/home/wterm -g \"wTerm User\"");
	system("mkdir /var/home/wterm");
	system("chown -R wterm /var/home/wterm");
	if (hasPassword("root"))
	{
		addToGroup("wterm", "root");
		setup_su(true);
	} else {
		setup_su(false);
	}
	if (!write) system("mount -o remount,ro /");
}

int main(int argc, const char* argv[])
{
	openlog("us.ryanhope.wterm.plugin", LOG_PID, LOG_USER);
	setlogmask(LOG_UPTO((argc > 3 && atoi(argv[3])>=LOG_EMERG && atoi(argv[3])<=LOG_DEBUG) ? atoi(argv[3]) : LOGLEVEL));

	setup_wterm_user();

	PDL_Init(0);

	sdlTerminal = new SDLTerminal();
	Terminal *terminal = new Terminal();
	terminal->path = strdup(argv[0]);
	char *e = strrchr(terminal->path, '/');
	*e = 0;

	sdlTerminal->setFontSize((argc > 1 && atoi(argv[1])) ? atoi(argv[1]) : 12);
	sdlTerminal->start();

	if (argc > 2)
		terminal->setExec(argv[2]);
	else
		terminal->setExec("login -f root");

	PDL_RegisterJSHandler("setupSU", setupSU);
	PDL_RegisterJSHandler("userAddToGroup", userAddToGroup);
	PDL_RegisterJSHandler("userHasPassword", userHasPassword);
	PDL_RegisterJSHandler("userSetPassword", userSetPassword);
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

	if (sdlTerminal->isReady())
	{
		sdlTerminal->setExtTerminal(terminal);
		terminal->setExtTerminal(sdlTerminal);

		//Set defaults
		sdlTerminal->getTerminalState()->resetTerminal();

		//Must set window size before starting the terminal.
		terminal->setWindowSize(sdlTerminal->getMaximumColumnsOfText(), sdlTerminal->getMaximumLinesOfText());

		if (terminal->start() == 0) //Non-blocking, creates a thread to read and a child process for slave device.
		{
			while (!terminal->isReady())
				sched_yield();
			PDL_CallJS("ready", NULL, 0);

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
