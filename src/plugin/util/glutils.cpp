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

#include "glutils.hpp"

const char* glutils_gl_errorstr(GLenum err) {
	switch (err) {
	case GL_NO_ERROR: return "No error";
	case GL_INVALID_ENUM: return "Unacceptable value for an enumerated argument";
	case GL_INVALID_VALUE: return "Numeric argument out of range";
	case GL_INVALID_OPERATION: return "Invalid operation";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "Invalid framebuffer operation";
	case GL_OUT_OF_MEMORY: return "Out of memory";
	default: return "Unknown error";
	}
}

GLuint loadShader(GLenum type, const char *source) {
	const char *sname = (type == GL_VERTEX_SHADER) ? "Vertex Shader" : "Fragment Shader";
	GLuint shader;
	GLint compiled;

	shader = glCreateShader(type);
	checkGLError();

	if (!shader) return 0;

	glShaderSource(shader, 1, &source, 0);
	checkGLError();

	glCompileShader(shader);
	checkGLError();

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	checkGLError();

	if (!compiled) {
		char buf[1024];

		glGetShaderInfoLog(shader, sizeof(buf), 0, buf);

		syslog(LOG_ERR, "%s compile error: %s", sname, buf); 

		glDeleteShader(shader);
		checkGLError();
		return 0;
	} else {
		char buf[1024];

		glGetShaderInfoLog(shader, sizeof(buf), 0, buf);

		if (buf[0]) {
			syslog(LOG_INFO, "%s compile log: %s", sname, buf); 
		}
	}

	return shader;
}

GLuint loadProgram(const char *vertShader, const char *fragShader) {
	GLuint vs, fs, p;
	GLint linked;

	vs = loadShader(GL_VERTEX_SHADER, vertShader);

	if (!vs) {
		syslog(LOG_ERR, "Couldn't load vertex shader");
		return 0;
	}

	fs = loadShader(GL_FRAGMENT_SHADER, fragShader);

	if (!fs) {
		syslog(LOG_ERR, "Couldn't load fragment shader");
		glDeleteShader(vs);
		checkGLError();
		return 0;
	}

	p = glCreateProgram();

	if (!p) {
		glDeleteShader(vs);
		glDeleteShader(fs);
		checkGLError();
		return 0;
	}

	glAttachShader(p, vs);
	checkGLError();
	glAttachShader(p, fs);
	checkGLError();

	/* delete our reference, p keeps them alive */
	glDeleteShader(vs);
	glDeleteShader(fs);

	glLinkProgram(p);
	checkGLError();

	glGetProgramiv(p, GL_LINK_STATUS, &linked);
	checkGLError();

	if (!linked) {
		char buf[1024];

		glGetProgramInfoLog(p, sizeof(buf), 0, buf);
		syslog(LOG_ERR, "Program link error: %s", buf);

		glDeleteProgram(p);
		checkGLError();
		return 0;
	} else {
		char buf[1024];

		glGetProgramInfoLog(p, sizeof(buf), 0, buf);
		if (buf[0]) {
			syslog(LOG_INFO, "Program link log: %s", buf);
		}
	}

	return p;
}
