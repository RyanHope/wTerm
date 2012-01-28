/**
 * This file is part of wTerm.
 * Copyright (C) 2012 Stefan Bühler <stbuehler@web.de>
 *
 * wTerm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * wTerm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with wTerm.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTIL_UTF8_HPP__
#define UTIL_UTF8_HPP__

#include "sdl/sdlcore.hpp"

/* parse first utf-8 char in 0-terminated str, returns 0 on failure.
 * only reads <= 0xffff codes
 */
Uint16 parseUtf8Char(const char *str);

/* write unicode (<= 0xffff) code to character string, needs 4 bytes
 * with 0-termination
 */
void writeUtf8Char(char *buf, Uint16 code);

#endif

