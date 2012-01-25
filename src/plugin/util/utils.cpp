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

#include <errno.h>
#include <time.h>
#include <string.h>
#include <syslog.h>

struct time_measure {
	struct timespec ts;

	time_measure() { ts.tv_sec = 0; ts.tv_nsec = 0; }
};

time_measure* time_measure_start() {
	time_measure *tmStart = new time_measure();

	if (-1 == clock_gettime(CLOCK_MONOTONIC, &tmStart->ts)) {
		syslog(LOG_ERR, "couldn't get start time: clock_gettime(CLOCK_MONOTONIC) failed: %s", strerror(errno));
	}

	return tmStart;
}

double time_measure_end(time_measure *tmStart) {
	if (!tmStart) return 0;

	time_measure start = *tmStart, end;
	delete tmStart;

	if (-1 == clock_gettime(CLOCK_MONOTONIC, &end.ts)) {
		syslog(LOG_ERR, "couldn't get end time: clock_gettime(CLOCK_MONOTONIC) failed: %s", strerror(errno));
	}

	return (end.ts.tv_sec - start.ts.tv_sec) + ((double) 1e-9)*(end.ts.tv_nsec - start.ts.tv_nsec);
}

unsigned int nextPowerOfTwo(unsigned int n) {
	/* http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return (n+1); /* warning: overflow possible */
}
