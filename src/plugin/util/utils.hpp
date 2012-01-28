/**
 * This file is part of SDLTerminal/wTerm.
 * Copyright (C) 2012 Stefan Bühler <stbuehler@web.de>
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

#ifndef UTIL_UTILS_HPP__
#define UTIL_UTILS_HPP__

#include <syslog.h>

// Log assertion failures to syslog
#define assert(c) \
	do { \
		if (!(c)) { \
			syslog(LOG_ERR, "Assertion \"%s\" failed at %s:%d", \
					__STRING(c), __FILE__, __LINE__); \
			abort(); \
		} \
	} while(0)

struct time_measure;

time_measure* time_measure_start();
double time_measure_end(time_measure *tmStart);

/* warning: overflow is possible (returns 0) */
unsigned int nextPowerOfTwo(unsigned int n);


#endif
