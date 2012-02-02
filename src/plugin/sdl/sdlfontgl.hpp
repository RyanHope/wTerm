/**
 * This file is part of wTerm.
 * Copyright (C) 2012 Will Dietz <webos@wdtz.org>
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

#ifndef _SDLFONTGL_H_
#define _SDLFONTGL_H_

#include <GLES2/gl2.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include <vector>
#include <string>

// cursor styles
#include "terminal/terminalstate.hpp"

// OpenGL Font Rendering
class SDLFontGL {
public:
	typedef struct {
		unsigned int font;
		unsigned int fg;
		unsigned int bg;
		bool blink;
	} TextGraphicsInfo;
private:
	// font/glyph data
	// --------------
	std::string m_fontFilename;
	unsigned int m_fontptsize;
	std::vector<int> m_fontStyles;

	bool m_fontsLoaded;
	std::vector<TTF_Font*> m_fonts;

	GLuint m_glyphTex;
	int texW, texH;

	bool * haveCacheLine; // Dirty bit for each cache line

	int nWidth, nHeight; /* (font) character width/height in pixels */

	/* map slots: from (unicode >> 7) to slot index (or -1 if unsupported) */
	int m_slotMap[512];


	// character data
	// --------------
	GLuint m_cellsTex;
	unsigned int m_cellsWidth, m_cellsHeight;
	unsigned char *m_cellData;

	struct {
		GLuint program;

		GLuint aDim, aPos, aCells, aGlyphs, aColors, aCellsize, aGlyphsize, aColorsize, aBlink;
	} m_charShader;

	unsigned int m_rows, m_cols;


	// color data
	// --------------
	GLuint m_colorTex;
	std::vector<SDL_Color> m_colors;


	// cursor data
	// --------------
	struct {
		GLuint program;

		GLuint aDim, aPos, aCursorpos, aCursorcolor, aCursorstyle;
	} m_cursorShader;

	bool m_cursorEnabled;
	unsigned int m_cursorColor, m_cursorCol, m_cursorRow;
	TSCursorStyle m_cursorStyle;


	// misc data
	// --------------

	// pixels on output window to use
	// specified values
	unsigned int m_dimX, m_dimY, m_dimW, m_dimH;
	// calculated centered values with current font size
	unsigned int m_curdimX, m_curdimY, m_curdimW, m_curdimH;

	bool m_openglActive;



	// private funcs
	// --------------

	void updateFonts();
	void clearFonts();
	void clearGLFonts();
	void openFonts();
	unsigned int getGlyphSlot(unsigned int font, unsigned int slot);

	void updateColors();
	void clearGLColors();

	void updateCursor();

	void updateCells();
	void clearGLCells();
	void updateDimensions();

public:
	SDLFontGL(const char *fontfilename, unsigned int fontptsize);
	~SDLFontGL();

	void initGL(unsigned int x, unsigned int y, unsigned int w, unsigned int h);
	void clearGL();
	void setDimension(unsigned int x, unsigned int y, unsigned int w, unsigned int h);
	void setCursor(bool enabled, unsigned int row, unsigned int col, unsigned int color, TSCursorStyle style);
	void drawGL(bool blink);

	unsigned int rows() const { return m_rows; }
	unsigned int cols() const { return m_cols; }

	void setFontSize(unsigned int ptsize);
	void setFontStyles(std::vector<int> styles);
	void setFontFilename(const char *filename);
	unsigned int getFontSize() const { return m_fontptsize; }

	void setupColors(std::vector<SDL_Color> colors);

	void clearText();
	// col/row start at 0/0
	void drawTextGL(TextGraphicsInfo & graphicsInfo, unsigned int col, unsigned int row, Uint16 cChar);
};

#endif // _SDLFONTGL_H_
