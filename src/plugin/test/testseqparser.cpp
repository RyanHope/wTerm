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

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "terminal/seqparser.hpp"
#include "util/logger.hpp"

int assertSeq(ControlSeqParser *parser, const char *seq, int expSeqId, int *expValues, int expNumValues, int expLength)
{
	int result = 0;
	int seqId;
	int values[20];
	int numValues = 0;
	int length = 0;

	memset(values, 0, sizeof(values));

	Logger::getInstance()->info("Start testing '%s'...", seq);

	seqId = parser->parse(seq, values, &numValues, &length);

	if (seqId != expSeqId)
	{
		Logger::getInstance()->error("\tExpecting ID %d, got %d.", expSeqId, seqId);
		result = -1;
	}

	if (length != expLength)
	{
		Logger::getInstance()->error("\tExpecting length %d, got %d.", expLength, length);
		result = -1;
	}

	if (numValues != expNumValues)
	{
		Logger::getInstance()->error("\tExpecting number of values %d, got %d.", expNumValues, numValues);
		result = -1;
	}

	for (int i=0; i < expNumValues; i++)
	{
		if (values[i] != expValues[i])
		{
			Logger::getInstance()->error("\tExpecting value[%d] %d, got %d.", i, expValues[i], values[i]);
			result = -1;
		}
	}

	if (result == 0)
	{
		Logger::getInstance()->info("Passed testing '%s'.", seq);
	}
	else
	{
		Logger::getInstance()->error("Failed testing '%s'.", seq);
	}

	return result;
}

int main()
{
	ControlSeqParser *parser = new ControlSeqParser();

	int numValues = 0;
	int length = 0;

	Logger::getInstance()->setLogLevel(Logger::ALL);

	{
		int values[] = { 12 };
		assertSeq(parser, "\x1B[12A", CS_CURSOR_UP, values, 1, 5);
	}

	{
		int values[] = { 6434 };
		assertSeq(parser, "\x1B[6434B", CS_CURSOR_DOWN, values, 1, 7);
	}

	{
		int values[] = { 0, 123 };
		assertSeq(parser, "\x1B[0;123H", CS_CURSOR_POSITION, values, 2, 8);
	}

	{
		int values[] = { 4, 512, 74 };
		assertSeq(parser, "\x1B[?4;512;74h", CS_MODE_SET, values, 3, 12);
	}

	delete parser;
	return 0;
}
