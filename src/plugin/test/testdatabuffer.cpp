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

#include "unittest.hpp"

#include "util/databuffer.hpp"
#include "util/logger.hpp"

#include <string.h>

int main()
{
	char buffer[1024];
	DataBuffer *data = new DataBuffer();

	Logger::getInstance()->setLogLevel(Logger::INFO);
	memset(buffer, 0, sizeof(buffer));

	assertEquals(0, data->size(), "Testing initial size");

	assertEquals(0, data->append("abc", 4), "Testing return after append");
	data->copy(buffer, data->size());
	assertEquals(4, data->size(), "Testing size after append");
	assertEquals("abc", 4, buffer, data->size(), "Testing data after append");

	assertEquals(0, data->clear(2, 1, false), "Testing return after clear");
	data->copy(buffer, data->size());
	assertEquals(4, data->size(), "Testing size after clear");
	{
		char tmp[] = { 'a', 'b', '\0', '\0' };
		assertEquals(tmp, 4, buffer, data->size(), "Testing data after clear");
	}

	assertEquals(-1, data->clear(5, 1, false), "Testing return after failed clear");
	data->copy(buffer, data->size());
	assertEquals(4, data->size(), "Testing size after failed clear");
	{
		char tmp[] = { 'a', 'b', '\0', '\0' };
		assertEquals(tmp, 4, buffer, data->size(), "Testing data after failed clear");
	}

	assertEquals(0, data->clear(0, 2, false), "Testing return clear (2)");
	data->copy(buffer, data->size());
	assertEquals(4, data->size(), "Testing size after clear (2)");
	{
		char tmp[] = { '\0', '\0', '\0', '\0' };
		assertEquals(tmp, 4, buffer, data->size(), "Testing data after clear (2)");
	}

	assertEquals(0, data->append("more", 3), "Testing return append (2)");
	data->copy(buffer, data->size());
	assertEquals(7, data->size(), "Testing size after append (2)");
	{
		char tmp[] = { '\0', '\0', '\0', '\0', 'm', 'o', 'r'};
		assertEquals(tmp, 7, buffer, data->size(), "Testing data after append (2)");
	}

	assertEquals(0, data->fill('q', 3), "Testing return fill");
	data->copy(buffer, data->size());
	assertEquals(10, data->size(), "Testing size after fill");
	{
		char tmp[] = { '\0', '\0', '\0', '\0', 'm', 'o', 'r', 'q', 'q', 'q'};
		assertEquals(tmp, 10, buffer, data->size(), "Testing data after fill");
	}

	assertEquals(0, data->replace(0, "abcd", 4), "Testing return replace");
	data->copy(buffer, data->size());
	assertEquals(10, data->size(), "Testing size after replace");
	{
		char tmp[] = { 'a', 'b', 'c', 'd', 'm', 'o', 'r', 'q', 'q', 'q'};
		assertEquals(tmp, 10, buffer, data->size(), "Testing data after replace");
	}

	assertEquals(0, data->replace(7, "pew", 3), "Testing return replace (2)");
	data->copy(buffer, data->size());
	assertEquals(10, data->size(), "Testing size after replace (2)");
	{
		char tmp[] = { 'a', 'b', 'c', 'd', 'm', 'o', 'r', 'p', 'e', 'w'};
		assertEquals(tmp, 10, buffer, data->size(), "Testing data after replace (2)");
	}

	assertEquals(0, data->clear(), "Testing return clear (3)");
	data->copy(buffer, data->size());
	assertEquals(0, data->size(), "Testing size after clear (3)");
	{
		char tmp[] = { '\0' };
		assertEquals(tmp, 0, buffer, data->size(), "Testing data after clear (3)");
	}

	assertEquals(0, data->append("abcdefgh", 9), "Testing return after append (3)");
	data->copy(buffer, data->size());
	assertEquals(9, data->size(), "Testing size after append (3)");
	assertEquals("abcdefgh", 9, buffer, data->size(), "Testing data after append (3)");

	assertEquals(0, data->clear(3, 4, true), "Testing return clear shift");
	data->copy(buffer, data->size());
	assertEquals(5, data->size(), "Testing size after clear shift");
	{
		char tmp[] = { 'a', 'b', 'c', 'h', '\0' };
		assertEquals(tmp, 5, buffer, data->size(), "Testing data after clear shift");
	}

	assertEquals(0, data->clear(2, 3, true), "Testing return clear shift (2)");
	data->copy(buffer, data->size());
	assertEquals(2, data->size(), "Testing size after clear shift (2)");
	{
		char tmp[] = { 'a', 'b' };
		assertEquals(tmp, 2, buffer, data->size(), "Testing data after clear shift (2)");
	}

	assertEquals(0, data->clear(0, 1, true), "Testing return clear shift (3)");
	data->copy(buffer, data->size());
	assertEquals(1, data->size(), "Testing size after clear shift (3)");
	{
		char tmp[] = { 'b' };
		assertEquals(tmp, 1, buffer, data->size(), "Testing data after clear shift (3)");
	}

	data->append("more", 5);
	assertEquals(0, data->clear(3, 3, false), "Testing return clear (3)");
	data->copy(buffer, data->size());
	assertEquals(3, data->size(), "Testing size after clear (3)");
	{
		char tmp[] = { 'b', 'm', 'o' };
		assertEquals(tmp, 3, buffer, data->size(), "Testing data after clear (3)");
	}

	data->clear();
	data->append("more", 5);
	assertEquals(0, data->insert(0, "abc", 3), "Testing return insert");
	data->copy(buffer, data->size());
	assertEquals(8, data->size(), "Testing size after insert");
	{
		char tmp[] = { 'a', 'b', 'c', 'm', 'o', 'r', 'e', '\0' };
		assertEquals(tmp, 8, buffer, data->size(), "Testing data after insert");
	}

	assertEquals(0, data->insert(3, "def", 3), "Testing return insert (2)");
	data->copy(buffer, data->size());
	assertEquals(11, data->size(), "Testing size after insert (2)");
	{
		char tmp[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'm', 'o', 'r', 'e', '\0' };
		assertEquals(tmp, 11, buffer, data->size(), "Testing data after insert (2)");
	}

	assertEquals(0, data->insert(11, "xyz", 3), "Testing return insert (3)");
	data->copy(buffer, data->size());
	assertEquals(14, data->size(), "Testing size after insert (3)");
	{
		char tmp[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'm', 'o', 'r', 'e', '\0', 'x', 'y', 'z' };
		assertEquals(tmp, 14, buffer, data->size(), "Testing data after insert (3)");
	}

	assertEquals(0, data->copy(4, buffer, data->size()), "Testing return copy index");
	assertEquals(14, data->size(), "Testing size after copy index");
	{
		char tmp[] = { 'e', 'f', 'm', 'o', 'r', 'e', '\0', 'x', 'y', 'z' };
		assertEquals(tmp, 10, buffer, 10, "Testing data after copy index");
	}

	delete data;

	return 0;
}
