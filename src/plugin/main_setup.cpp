/**
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <SDL/SDL.h>
#include <syslog.h>
#include <pwd.h>
#include <PDL.h>

#include <stdio.h>

void setup_su(int enable)
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

int isMountWritable(const char* dest)
{
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

int hasPassword(const char *user)
{
	struct passwd *pw = getpwnam(user);
	return (strlen(pw->pw_passwd)==34);
}

void setPassword(const char *user, const char *password)
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

void setup_wterm_user()
{
	system("adduser -D wterm -h /var/home/wterm -g \"wTerm User\"");
	system("mkdir /var/home/wterm");
	system("chown -R wterm /var/home/wterm");
	if (hasPassword("root"))
	{
		addToGroup("wterm", "root");
		setup_su(1);
	} else {
		setup_su(0);
	}
}

PDL_bool setupNonRoot(PDL_JSParameters *params)
{
	int write = isMountWritable("/");
	if (!write) system("mount -o remount,rw /");
	setup_wterm_user();
	if (!write) system("mount -o remount,ro /");

	return PDL_TRUE;
}

PDL_bool userSetPassword(PDL_JSParameters *params)
{
	const char *user = PDL_GetJSParamString(params, 0);
	const char *password = PDL_GetJSParamString(params, 1);

	int write = isMountWritable("/");
	if (!write) system("mount -o remount,rw /");
	setPassword(user, password);
	if (!write) system("mount -o remount,ro /");

	return PDL_TRUE;
}

PDL_bool userHasPassword(PDL_JSParameters *params)
{
	char *reply = 0;
	asprintf(&reply, "%d", hasPassword(PDL_GetJSParamString(params, 0)));
	PDL_JSReply(params, reply);
	free(reply);

	return PDL_TRUE;
}

PDL_bool userAddToGroup(PDL_JSParameters *params)
{
	const char *user = PDL_GetJSParamString(params, 0);
	const char *group = PDL_GetJSParamString(params, 1);

	int write = isMountWritable("/");
	if (!write) system("mount -o remount,rw /");
	addToGroup(user, group);
	if (!write) system("mount -o remount,ro /");

	return PDL_TRUE;
}

PDL_bool setupSU(PDL_JSParameters *params)
{
	int write = isMountWritable("/");
	if (!write) system("mount -o remount,rw /");
	setup_su(PDL_GetJSParamInt(params, 0));
	if (!write) system("mount -o remount,ro /");
	return PDL_TRUE;
}

void setup_main(int argc, const char* argv[])
{
	SDL_Event Event;

	SDL_Init(SDL_INIT_VIDEO);
	PDL_Init(0);

	PDL_RegisterJSHandler("setupNonRoot", setupNonRoot);
	PDL_RegisterJSHandler("setupSU", setupSU);
	PDL_RegisterJSHandler("userAddToGroup", userAddToGroup);
	PDL_RegisterJSHandler("userHasPassword", userHasPassword);
	PDL_RegisterJSHandler("userSetPassword", userSetPassword);

	PDL_JSRegistrationComplete();

	PDL_CallJS("ready", NULL, 0);

	do {
		SDL_WaitEvent(&Event);
	} while (Event.type != SDL_QUIT);

	PDL_Quit();
	SDL_Quit();
}
