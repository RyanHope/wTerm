/**
 * This file is part of wTerm.
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

#include <cstdlib>
#include <syslog.h>

void terminal_main(int argc, const char* argv[]);
void setup_main(int argc, const char* argv[]);

int main(int argc, const char* argv[])
{
	openlog("us.ryanhope.wterm.plugin", LOG_PID, LOG_USER);
	setlogmask(LOG_UPTO(LOGLEVEL));

	if (argc>1)
		terminal_main(argc, argv);
	else
		setup_main(argc, argv);

	closelog();
	exit(0);
}
