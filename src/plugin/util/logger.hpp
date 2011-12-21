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

#ifndef LOGGER_HPP__
#define LOGGER_HPP__

#include <pthread.h>
#include <stdio.h>
#include <time.h>

class Logger
{
private:
	static Logger *s_instance;
	FILE *m_out;
	int m_nLogLevel;
	pthread_mutex_t m_rwLock;

	time_t m_time;
	struct tm *m_timeInfo;
	char m_timeBuffer[64];

	Logger();
	void log(const char *message, va_list &args, const char *type, int logLevel);

public:
	static const int FATAL = 0;
	static const int ERROR = 1;
	static const int WARN = 2;
	static const int INFO = 3;
	static const int DEBUG = 4;
	static const int DUMP = 5;
	static const int ALL = 6;

	static Logger *getInstance();
	~Logger();

	void fatal(const char *message, ...);
	void error(const char *message, ...);
	void warn(const char *message, ...);
	void info(const char *message, ...);
	void debug(const char *message, ...);
	void dump(const char *message, ...);

	void setOutputStream(FILE *out);
	FILE *getOutputStream();

	void setLogLevel(int level);
	int getLogLevel();
};

#endif
