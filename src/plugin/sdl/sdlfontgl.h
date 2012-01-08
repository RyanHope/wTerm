/**
 * This file is part of SDLTerminal.
 * Copyright (C) 2012 Will Dietz <webos@wdtz.org>
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

#ifndef _SDLFONTGL_H_
#define _SDLFONTGL_H_

#include <GLES/gl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

// OpenGL Font Rendering
class SDLFontGL {
private:
	// Master font rendering texture.
	GLuint GlyphCache;
	int texW, texH;

	// Dirty bit for each font
	bool * haveFontLine;

	int nFonts, nCols;
	TTF_Font** fnts;
	SDL_Color* cols;
	int nWidth, nHeight;

	int screenCols, screenRows;
	GLfloat * colorValues;
	GLfloat * texValues;
	GLfloat * vtxValues;
	int numChars;
	char printableChars[223];
	size_t nChars;

	void clearGL();
	void ensureFontLine(int font);
	void createTexture();
	void drawBackground(int color, int X, int Y, int cells);

	void initializePrintables();

public:
	SDLFontGL() : GlyphCache(0), texW(0), texH(0),
	nFonts(0), nCols(0), fnts(0), cols(0), screenCols(0), screenRows(0),
	colorValues(0), texValues(0), vtxValues(0) {
		initializePrintables();
	}
	~SDLFontGL();

	// Indicate what fonts and colors to use
	// This invalidates the cache, so only call when things change.
	void setupFontGL(int fnCount, TTF_Font** fnts, int colCount, SDL_Color *cols);

	// Begin drawing text to the screen, assuming the given screen size
	void startTextGL(int cols, int rows);
	void drawTextGL(int font, int fg, int bg, int x, int y, const char * text);
	// Done drawing text, commit!
	void endTextGL();
};

#endif // _SDLFONTGL_H_
