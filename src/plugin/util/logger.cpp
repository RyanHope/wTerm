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

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "logger.hpp"

Logger *Logger::s_instance = new Logger();

Logger::Logger()
{
	m_out = stdout;
	m_nLogLevel = ERROR;
	pthread_mutex_init(&m_rwLock, NULL);
}

Logger::~Logger()
{
	s_instance = NULL;
	pthread_mutex_destroy(&m_rwLock);
}

Logger *Logger::getInstance()
{
	if (s_instance == NULL)
	{
		s_instance = new Logger();
	}

	return s_instance;
}

FILE *Logger::getOutputStream()
{
	return m_out;
}

void Logger::setOutputStream(FILE *out)
{
	m_out = out;
}

int Logger::getLogLevel()
{
	return m_nLogLevel;
}

void Logger::setLogLevel(int level)
{
	m_nLogLevel = level;
}

void Logger::log(const char *message, va_list &args, const char *type, int logLevel)
{
	pthread_mutex_lock(&m_rwLock);

	if (getLogLevel() >= logLevel)
	{
		time(&m_time);
		m_timeInfo = localtime(&m_time);
		strftime(m_timeBuffer, sizeof(m_timeBuffer), "%Y %m %d %X", m_timeInfo);

		fprintf(m_out, "%s [%s] ", m_timeBuffer, type);
		vfprintf(m_out, message, args);
		fprintf(m_out, "\n");
		fflush(m_out);
	}

	pthread_mutex_unlock(&m_rwLock);
}

void Logger::fatal(const char *message, ...)
{
	va_list args;

	va_start(args, message);
	log(message, args, "FATAL", FATAL);
	va_end(args);
}

void Logger::error(const char *message, ...)
{
	va_list args;

	va_start(args, message);
	log(message, args, "ERROR", ERROR);
	va_end(args);
}

void Logger::warn(const char *message, ...)
{
	va_list args;

	va_start(args, message);
	log(message, args, "WARN", WARN);
	va_end(args);
}

void Logger::info(const char *message, ...)
{
	va_list args;

	va_start(args, message);
	log(message, args, "INFO", INFO);
	va_end(args);
}

void Logger::debug(const char *message, ...)
{
	va_list args;

	va_start(args, message);
	log(message, args, "DEBUG", DEBUG);
	va_end(args);
}

void Logger::dump(const char *message, ...)
{
	va_list args;

	va_start(args, message);
	log(message, args, "DUMP", DUMP);
	va_end(args);
}
