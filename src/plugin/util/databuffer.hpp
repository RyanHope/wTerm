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

#ifndef DATABUFFER_HPP__
#define DATABUFFER_HPP__

#include <pthread.h>
#include <stdio.h>

/**
 * A thread safe data buffer.
 */
class DataBuffer
{
private:
	static const size_t INIT_MAX_SIZE;
	size_t m_size;
	size_t m_maxSize;
	char *m_buffer;
	pthread_mutexattr_t m_rwLockAttr;
	pthread_mutex_t m_rwLock;

public:
	DataBuffer();
	~DataBuffer();

	int replace(int startIndex, const char *data, size_t size);
	int append(const char *data, size_t size);
	int fill(char c, size_t size);
	int copy(char *dest, size_t size);
	int copy(int startIndex, char *dest, size_t size);
	int insert(int startIndex, const char *data, size_t size);
	int clear(int startIndex, size_t size, bool bShift);
	int clear();
	size_t size() const;
	void print(FILE *out);
};

#endif
