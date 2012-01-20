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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "databuffer.hpp"

const unsigned int DataBuffer::INIT_MAX_SIZE = (1024 * sizeof(char));

DataBuffer::DataBuffer()
{
	m_size = 0;
	m_maxSize = INIT_MAX_SIZE;
	m_buffer = (char *)malloc(m_maxSize);

	pthread_mutexattr_init(&m_rwLockAttr);
	pthread_mutexattr_settype(&m_rwLockAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_rwLock, &m_rwLockAttr);
}

DataBuffer::~DataBuffer()
{
	pthread_mutex_lock(&m_rwLock);

	if (m_buffer)
	{
		free(m_buffer);
		m_buffer = NULL;
	}

	pthread_mutex_unlock(&m_rwLock);

	pthread_mutexattr_destroy(&m_rwLockAttr);
	pthread_mutex_destroy(&m_rwLock);
}

/**
 * Replaces the data at the specified index with the new block of data. The index
 * must be within bounds of the data buffer. Overflow data is ignored.
 * Returns -1 if an error occurs. Returns 0 if success.
 */
int DataBuffer::replace(int startIndex, const char *data, size_t size)
{
	int nResult = 0;

	pthread_mutex_lock(&m_rwLock);

	if (startIndex < 0 || startIndex >= m_size)
	{
		nResult = -1;
	}
	else if ((startIndex + size) > m_size)
	{
		if (startIndex >= m_size)
		{
			nResult = -1;
		}
		else
		{
			size = m_size - startIndex;
		}
	}

	if (nResult == 0 && size > 0)
	{
		assert((startIndex + size) <= m_size);
		memcpy(m_buffer + startIndex, data, size);
	}

	pthread_mutex_unlock(&m_rwLock);

	return nResult;
}

/**
 * Appends the specified amount of bytes from the data source to the data buffer.
 * Returns -1 if an error occurs. Returns 0 if success.
 */
int DataBuffer::append(const char *data, size_t size)
{
	int result = 0;

	pthread_mutex_lock(&m_rwLock);

	size_t newSize = size + m_size;
	char *tmp;

	if (newSize > m_maxSize)
	{
		m_maxSize *= 2;
		tmp = (char *)realloc(m_buffer, m_maxSize);

		if (tmp == NULL)
		{
			result = -1;
		}
		else
		{
			m_buffer = tmp;
		}
	}

	if (result == 0 && size > 0)
	{
		memcpy(m_buffer + m_size, data, size);
		m_size = newSize;
	}

	pthread_mutex_unlock(&m_rwLock);

	return result;
}

/**
 * Inserts a block of data into the buffer at a specified index. If the
 * index is beyond the range of the buffer, then the data is simply appended.
 */
int DataBuffer::insert(int startIndex, const char *data, size_t size)
{
	int nResult = 0;

	pthread_mutex_lock(&m_rwLock);

	if (startIndex < 0)
	{
		nResult = -1;
	}

	if (nResult == 0 && size > 0)
	{
		if (startIndex >= m_size)
		{
			nResult = append(data, size);
		}
		else
		{
			size_t tailSize = (m_size - startIndex) * sizeof(char);
			char *tmp = NULL;

			if (tailSize > 0)
			{
				tmp = (char *)malloc(tailSize);
				memcpy(tmp, m_buffer + startIndex, tailSize);
				clear(startIndex, tailSize, true);
			}

			nResult = append(data, size);

			if (tailSize > 0)
			{
				assert(tmp != NULL);
				append(tmp, tailSize);
				free(tmp);
			}
		}
	}

	pthread_mutex_unlock(&m_rwLock);

	return nResult;
}

/**
 * Appends a block of data in the buffer filled with the same character.
 */
int DataBuffer::fill(char c, size_t size)
{
	int result = 0;

	pthread_mutex_lock(&m_rwLock);

	size_t newSize = size + m_size;
	char *tmp;

	if (newSize > m_maxSize)
	{
		m_maxSize *= 2;
		tmp = (char *)realloc(m_buffer, m_maxSize);
		
		if (tmp == NULL)
		{
			result = -1;
		}
		else
		{
			m_buffer = tmp;
		}
	}

	if (result == 0 && size > 0)
	{
		memset(m_buffer + m_size, c, size);
		m_size = newSize;
	}

	pthread_mutex_unlock(&m_rwLock);

	return result;
}

/**
 * Sets the specified by range to the given character.
 */
int DataBuffer::set(int startIndex, char c, size_t size, bool extend)
{
  int nResult = 0;
	pthread_mutex_lock(&m_rwLock);

	if (startIndex < 0 || startIndex > m_size)
	{
		nResult = -1;
	}
	else if ((startIndex + size) > m_size)
	{
    if (extend)
      fill(c, startIndex + size - m_size);
    else
      size = m_size - startIndex;
	}

	if (nResult == 0 && size > 0)
	{
		memset(m_buffer + startIndex, c, size);
	}

	pthread_mutex_unlock(&m_rwLock);

  return nResult;
}

/**
 * Copies specified amount of bytes from the data buffer to the destination.
 */
int DataBuffer::copy(int startIndex, char *dest, size_t size)
{
	int nResult = 0;

	pthread_mutex_lock(&m_rwLock);

	if (startIndex < 0 || startIndex >= m_size)
	{
		nResult = -1;
	}
	else if ((startIndex + size) > m_size)
	{
		size = m_size - startIndex;
	}

	if (nResult == 0 && size > 0)
	{
		memcpy(dest, m_buffer + startIndex, size);
	}

	pthread_mutex_unlock(&m_rwLock);

	return nResult;
}

/**
 * Copies specified amount of bytes from the data buffer to the destination.
 */
int DataBuffer::copy(char *dest, size_t size)
{
	return copy(0, dest, size);
}

/**
 * Clears the data at the specified index. The index must be within bounds of the data buffer.
 * If shift is specified, then the data gap created is completely removed; thus shifting the subsequent data.
 * The size of the buffer is decreased if data is shifted, or the tail of the buffer is removed.
 * Returns -1 if an error occurs. Returns 0 if success.
 */
int DataBuffer::clear(int startIndex, size_t size, bool bShift)
{
	int nResult = 0;

	pthread_mutex_lock(&m_rwLock);

	if (startIndex < 0 || startIndex >= m_size)
	{
		nResult = -1;
	}
	else if ((startIndex + size) > m_size)
	{
		size = m_size - startIndex;
	}

	if (nResult == 0 && size > 0)
	{
		assert((startIndex + size) <= m_size);

		if (bShift)
		{
			size_t tailSize = (m_size - startIndex - size) * sizeof(char);

			if (tailSize > 0)
			{
				char *tmp = (char *)malloc(tailSize);

				memcpy(tmp, m_buffer + startIndex + size, tailSize);
				memcpy(m_buffer + startIndex, tmp, tailSize);

				free(tmp);
			}

			m_size -= size;
		}
		else
		{
			memset(m_buffer + startIndex, 0, size);

			if ((startIndex + size) >= m_size)
			{
				m_size -= size;
			}
		}
	}

	pthread_mutex_unlock(&m_rwLock);

	return nResult;
}

/**
 * Clears the data buffer. Returns -1 if clearing did not succeed.
 * Returns 0 if clearing is successful.
 */
int DataBuffer::clear()
{
	int result = 0;

	pthread_mutex_lock(&m_rwLock);

	m_maxSize = INIT_MAX_SIZE;
	m_size = 0;

	if (m_buffer)
	{
		free(m_buffer);
	}

	m_buffer = (char *)malloc(m_maxSize);

	if (m_buffer == NULL)
	{
		result = -1;
	}

	pthread_mutex_unlock(&m_rwLock);

	return result;
}

/**
 * Returns the size of the data buffer in bytes.
 */
size_t DataBuffer::size() const
{
	return m_size;
}

/**
 * Prints to the data in ASCII to the specified file stream.
 */
void DataBuffer::print(FILE *out)
{
	pthread_mutex_lock(&m_rwLock);

	for (size_t i=0; i < m_size; i++)
	{
		fprintf(out, "%c", m_buffer[i]);
	}

	pthread_mutex_unlock(&m_rwLock);
}
