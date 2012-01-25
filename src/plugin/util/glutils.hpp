/**
 * This file is part of SDLTerminal / wTerm.
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

#ifndef UTIL_GLUTILS_HPP__
#define UTIL_GLUTILS_HPP__

#include <GLES2/gl2.h>
#include <syslog.h>

// For debugging
#define checkGLError() \
	do { \
		GLenum err = glGetError(); \
		if (!err) break; \
		syslog(LOG_ERR, "GL Error %x at %s:%d: %s", \
				err, __FILE__, __LINE__, glutils_gl_errorstr(err)); \
	} while (1)

const char* glutils_gl_errorstr(GLenum err);

GLuint loadShader(GLenum type, const char *source);

GLuint loadProgram(const char *vertShader, const char *fragShader);

#endif

