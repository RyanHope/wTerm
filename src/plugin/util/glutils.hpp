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

#ifndef UTIL_GLUTILS_HPP__
#define UTIL_GLUTILS_HPP__

#include <GLES2/gl2.h>
#include <syslog.h>

// For debugging
#define checkGLError() \
	do { \
		GLenum checkGLError_err = glGetError(); \
		for (;checkGLError_err;) { \
			syslog(LOG_ERR, "GL Error %x at %s:%d: %s", \
					checkGLError_err, __FILE__, __LINE__, glutils_gl_errorstr(checkGLError_err)); \
			GLenum checkGLError_e = glGetError(); \
			if (checkGLError_e == checkGLError_err) break; \
			checkGLError_err = checkGLError_e; \
		} \
	} while (0)

const char* glutils_gl_errorstr(GLenum err);

GLuint loadShader(GLenum type, const char *source);

GLuint loadProgram(const char *vertShader, const char *fragShader);

#endif

