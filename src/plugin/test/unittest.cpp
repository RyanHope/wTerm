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

#include "util/logger.hpp"

#include <assert.h>
#include <string.h>

bool assertEquals(int nExpected, int nActual, const char *sMsg)
{
	if (nExpected != nActual)
	{
		Logger::getInstance()->error("%s: Expecting %d, got %d.", sMsg, nExpected, nActual);
		assert(false);

		return false;
	}

	Logger::getInstance()->info("%s: Passed.", sMsg, nExpected, nActual);
	return true;
}

bool assertEquals(const char *sExpected, const char *sActual, const char *sMsg)
{
	if (strcmp(sExpected, sActual) != 0)
	{
		Logger::getInstance()->error("%s: Expecting '%s', got '%s'.", sMsg, sExpected, sActual);
		assert(false);

		return false;
	}

	Logger::getInstance()->info("%s: Passed.", sMsg, sExpected, sActual);
	return true;
}

bool assertEquals(const char *sExpected, int nExpectedSize, const char *sActual, int nActualSize, const char *sMsg)
{
	bool bEquals = true;

	if (nExpectedSize != nActualSize)
	{
		bEquals = false;
	}
	else
	{
		for (int i = 0; i < nExpectedSize; i++)
		{
			if (sExpected[i] != sActual[i])
			{
				bEquals = false;
				break;
			}
		}
	}

	if (!bEquals)
	{
		Logger::getInstance()->error("%s: Expecting '%s', got '%s'.", sMsg, sExpected, sActual);
		assert(false);

		return false;
	}

	Logger::getInstance()->info("%s: Passed.", sMsg, sExpected, sActual);
	return true;
}
