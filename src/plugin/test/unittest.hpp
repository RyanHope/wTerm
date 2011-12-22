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

#ifndef UNITTEST_HPP__
#define UNITTEST_HPP__

bool assertEquals(int nExpected, int nActual, const char *sMsg);
bool assertEquals(const char *sExpected, const char *sActual, const char *sMsg);
bool assertEquals(const char *sExpected, int nExpectedSize, const char *sActual, int nActualSize, const char *sMsg);

#endif
