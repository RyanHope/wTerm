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

#include "sdl/sdlfontgl.hpp"

#include "terminal/terminalstate.hpp"
#include "util/glutils.hpp"
#include "util/utils.hpp"

#include <syslog.h>

/* 128-byte long slots */
static const Uint16 unicode_slots[] = {
	0x0000, // C0 Controls and Basic Latin
	0x0080, // C1 Controls and Latin-1 Supplement

	0x0100, // Latin Extended-A
	0x0180, // Latin Extended-B
	0x0200, // Latin Extended-B (+ part of IPA Extensions)

	0x0380, // Greek and Coptic (contains PI)

	0x2200, // Mathematical Operators
	0x2500,
	0x2580, // Block elements + Geometric shapes
	0x2600
};

#define NSLOTS (sizeof(unicode_slots)/sizeof(unicode_slots[0]))

// hardcoded in some other places as >> 7, 512 ( = 2^16/128), ...
#define SLOTSIZE (128u)


static const char char_vertex_shader[] =
	"precision highp float;\n"
	"precision highp int;\n"
	"uniform vec2 dim;\n"
	"attribute vec2 pos;\n"
	"varying vec2 v;\n"
	"\n"
	"void main(void) {\n"
	"  v = vec2(pos.x * dim.x, pos.y * dim.y);\n"
	"  gl_Position = vec4(-1.0 + 2.0*pos.x, -1.0+2.0*pos.y, 1.0, 1.0);\n"
	"}\n"
	;

static const char char_fragment_shader[] =
	"precision highp float;\n"
	"precision highp int;\n"
	"varying highp vec2 v;\n"
	"uniform sampler2D cells;\n"
	"uniform sampler2D glyphs;\n"
	"uniform sampler2D colors;\n"
	"uniform vec2 cellsize;\n"
	"uniform vec2 glyphsize;\n"
	"uniform float colorsize;\n"
	"uniform bool blink;\n"
	"\n"
	"void main(void) {\n"
	"  vec4 cell = texture2D(cells, vec2((floor(v.x))*cellsize.x, (floor(v.y))*cellsize.y));\n"
	"  vec4 fg = texture2D(colors, vec2((0.5 + ceil(mod(cell.z,64.0)*255.0)) * colorsize, 0.5));\n"
	"  vec4 bg = texture2D(colors, vec2((0.5 + ceil(cell.w*255.0)) * colorsize, 0.5));\n"

	"  float slot = ceil(cell.y*127.0);\n"

	"  float ch = ceil(cell.x*127.0);\n"
	"  bool bl = (cell.z > 0.249 && !blink);\n"

	"  vec4 g = texture2D(glyphs, vec2((ch + fract(v.x))*glyphsize.x, (slot + 1.0 - fract(v.y)) * glyphsize.y));\n"
	"  gl_FragColor = mix(bg, fg, bl ? 0.0 : g.a);\n"
	"}\n"
	;

static const char cursor_vertex_shader[] =
	"precision highp float;\n"
	"precision highp int;\n"
	"uniform vec2 dim;\n"
	"uniform vec2 cursorpos;\n"
	"attribute vec2 pos;\n"
	"uniform int cursorstyle;\n"
	"\n"
	"void main(void) {\n"
	"  vec2 p;\n"
	"  if (2 == cursorstyle) {\n"
	"    float offx = (cursorpos.x < 0.5 ? 0.1 : 0.0);\n"
	"    p = vec2((cursorpos.x - offx + 0.2*pos.x) / dim.x, (cursorpos.y + 0.15 + 0.85*pos.y) / dim.y);\n"
	"  } else if (1 == cursorstyle) {\n"
	"    p = vec2((cursorpos.x + pos.x) / dim.x, (cursorpos.y + 0.15 + 0.2*pos.y) / dim.y);\n"
	"  } else {\n"
	"    p = vec2((cursorpos.x + pos.x) / dim.x, (cursorpos.y + 0.15 + 0.85*pos.y) / dim.y);\n"
	"  }\n"
	"  gl_Position = vec4(-1.0 + 2.0*p.x, -1.0+2.0*p.y, 1.0, 1.0);\n"
	"}\n"
	;

static const char cursor_fragment_shader[] =
	"precision highp float;\n"
	"precision highp int;\n"
	"uniform vec4 cursorcolor;\n"
	"\n"
	"void main(void) {\n"
	"  gl_FragColor = cursorcolor;\n"
	"}\n"
	;

static const GLfloat vertices[] = {
	0.0f, 1.0f,   0.0f, 0.0f,   1.0f, 0.0f,
	0.0f, 1.0f,   1.0f, 0.0f,   1.0f, 1.0f
};


SDLFontGL::SDLFontGL(const char *fontfilename, unsigned int fontptsize)
: m_fontFilename(fontfilename), m_fontptsize(fontptsize), m_fontsLoaded(false),
  m_glyphTex(0), texW(0), texH(0), haveCacheLine(0), nWidth(0), nHeight(0),
  m_cellsTex(0), m_cellsWidth(0), m_cellsHeight(0), m_cellData(0), m_rows(0), m_cols(0),
  m_colorTex(0),
  m_cursorEnabled(0), m_cursorColor(0), m_cursorCol(0), m_cursorRow(0), m_cursorStyle(TS_CURSOR_STYLE_BLOCK_BLINK),
  m_dimX(0), m_dimY(0), m_dimW(0), m_dimH(0),
  m_curdimX(0), m_curdimY(0), m_curdimW(0), m_curdimH(0),
  m_openglActive(false)
{
	m_charShader.program = 0;
	m_cursorShader.program = 0;

	for (unsigned int i = 0; i < 512; i++) {
		m_slotMap[i] = -1;
	}

	for (unsigned int i = 0; i < NSLOTS; i++) {
		Uint16 slot = unicode_slots[i] >> 7;
		m_slotMap[slot] = i;
	}
}

SDLFontGL::~SDLFontGL() {
	clearGL();
	clearFonts();
}

void SDLFontGL::initGL(unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
	clearGL();
	checkGLError();

	m_openglActive = true;

	m_charShader.program = loadProgram(char_vertex_shader, char_fragment_shader);
	if (m_charShader.program) {
		glUseProgram(m_charShader.program);
		checkGLError();

		m_charShader.aDim = glGetUniformLocation(m_charShader.program, "dim");
		m_charShader.aPos = glGetAttribLocation(m_charShader.program, "pos");
		m_charShader.aCells = glGetUniformLocation(m_charShader.program, "cells");
		m_charShader.aGlyphs = glGetUniformLocation(m_charShader.program, "glyphs");
		m_charShader.aColors = glGetUniformLocation(m_charShader.program, "colors");
		m_charShader.aGlyphsize = glGetUniformLocation(m_charShader.program, "glyphsize");
		m_charShader.aCellsize = glGetUniformLocation(m_charShader.program, "cellsize");
		m_charShader.aColorsize = glGetUniformLocation(m_charShader.program, "colorsize");
		m_charShader.aBlink = glGetUniformLocation(m_charShader.program, "blink");
		checkGLError();

		glVertexAttribPointer(m_charShader.aPos, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		checkGLError();

		glUniform1i(m_charShader.aCells, 0);
		checkGLError();

		glUniform1i(m_charShader.aGlyphs, 1);
		checkGLError();

		glUniform1i(m_charShader.aColors, 2);
		checkGLError();
	}

	m_cursorShader.program = loadProgram(cursor_vertex_shader, cursor_fragment_shader);
	if (m_cursorShader.program) {
		glUseProgram(m_cursorShader.program);
		checkGLError();

		m_cursorShader.aDim = glGetUniformLocation(m_cursorShader.program, "dim");
		m_cursorShader.aPos = glGetAttribLocation(m_cursorShader.program, "pos");
		m_cursorShader.aCursorpos = glGetUniformLocation(m_cursorShader.program, "cursorpos");
		m_cursorShader.aCursorcolor = glGetUniformLocation(m_cursorShader.program, "cursorcolor");
		m_cursorShader.aCursorstyle = glGetUniformLocation(m_cursorShader.program, "cursorstyle");
		checkGLError();

		glVertexAttribPointer(m_cursorShader.aPos, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		checkGLError();
	}

	m_dimX = x; m_dimY = y; m_dimW = w; m_dimH = h;

	updateFonts();
	updateDimensions();
	updateColors();
	updateCursor();

	checkGLError();
}

void SDLFontGL::clearGL() {
	checkGLError();

	if (m_charShader.program) {
		glDeleteProgram(m_charShader.program);
		checkGLError();
		m_charShader.program = 0;
	}

	if (m_cursorShader.program) {
		glDeleteProgram(m_cursorShader.program);
		checkGLError();
		m_cursorShader.program = 0;
	}

	clearGLFonts();
	clearGLCells();
	clearGLColors();

	m_openglActive = false;

	checkGLError();
}

void SDLFontGL::updateColors() {
	clearGLColors();

	if (!m_openglActive || m_colors.empty()) return;

	unsigned int texWidth = nextPowerOfTwo(m_colors.size());
	unsigned char* texData = static_cast<unsigned char*>(malloc(texWidth * 4));
	assert(texData);
	memset(texData, 0, texWidth * 4);

	for (unsigned int i = 0; i < m_colors.size(); i++) {
		texData[4*i] = m_colors[i].r;
		texData[4*i+1] = m_colors[i].g;
		texData[4*i+2] = m_colors[i].b;
		texData[4*i+3] = 255;;
	}

	if (m_charShader.program) {
		glUseProgram(m_charShader.program);
		checkGLError();

		glUniform1f(m_charShader.aColorsize, 1.0/nextPowerOfTwo(m_colors.size()));
		checkGLError();
	}

	if (m_colorTex) {
		glDeleteTextures(1, &m_colorTex);
		checkGLError();
		m_colorTex = 0;
	}

	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &m_colorTex);
	checkGLError();
	glBindTexture(GL_TEXTURE_2D, m_colorTex);
	checkGLError();

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	checkGLError();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	checkGLError();

	free(texData);
}

void SDLFontGL::clearGLColors() {
	if (m_colorTex) {
		glDeleteTextures(1, &m_colorTex);
		checkGLError();
		m_colorTex = 0;
	}
}

void SDLFontGL::setupColors(std::vector<SDL_Color> colors) {
	assert(!colors.empty());
	m_colors = colors;

	updateColors();
	// cursor does a manual color lookup
	updateCursor();
}

void SDLFontGL::updateCells() {
	clearGLCells();

	if (0 == m_cols || 0 == m_rows) return;

	m_cellsWidth = nextPowerOfTwo(m_cols);
	assert(m_cellsWidth >= m_cols);
	m_cellsWidth = (m_cellsWidth+3) & ~0x3; /* align at four bytes */
	m_cellsHeight = nextPowerOfTwo(m_rows);
	assert(m_cellsHeight >= m_rows);

	if (!m_openglActive) return;

	syslog(LOG_DEBUG, "updateCells: rows = %i, cols = %i, width = %i, height = %i", m_rows, m_cols, m_cellsWidth, m_cellsHeight);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &m_cellsTex);
	checkGLError();
	glBindTexture(GL_TEXTURE_2D, m_cellsTex);
	checkGLError();

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	checkGLError();

	m_cellData = static_cast<unsigned char*>(malloc(4*m_cellsWidth * m_cellsHeight));
	assert(m_cellData);
	clearText();

	// set size
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_cellsWidth, m_cellsHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	checkGLError();

	if (m_charShader.program) {
		glUseProgram(m_charShader.program);
		checkGLError();

		glUniform2f(m_charShader.aCellsize, 1.0/(float)m_cellsWidth, 1.0/(float)m_cellsHeight);
		checkGLError();
	}
}

void SDLFontGL::clearGLCells() {
	if (m_cellsTex) {
		glDeleteTextures(1, &m_cellsTex);
		checkGLError();
		m_cellsTex = 0;
	}

	if (m_cellData) {
		free(m_cellData);
		m_cellData = 0;
	}
}

void SDLFontGL::updateDimensions() {
	if (!m_openglActive || 0 == nWidth || 0 == nHeight) return;

	unsigned int oldCols = m_cols, oldRows = m_rows;

	m_cols = m_dimW / nWidth;
	m_rows = m_dimH / nHeight;

	m_curdimW = m_cols * nWidth;
	m_curdimH = m_rows * nHeight;
	m_curdimX = m_dimX + (m_dimW - m_curdimW) / 2;
	m_curdimY = m_dimY + (m_dimH - m_curdimH) / 2;

	if (oldCols != m_cols || oldRows != m_rows || !m_cellsTex) {
		updateCells();
	}

	if (m_charShader.program) {
		glUseProgram(m_charShader.program);
		checkGLError();

		glUniform2f(m_charShader.aDim, m_cols, m_rows);
		checkGLError();
	}

	if (m_cursorShader.program) {
		glUseProgram(m_cursorShader.program);
		checkGLError();

		glUniform2f(m_cursorShader.aDim, m_cols, m_rows);
		checkGLError();
	}
}

void SDLFontGL::setDimension(unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
	m_dimX = x; m_dimY = y; m_dimW = w; m_dimH = h;

	updateDimensions();
}

void SDLFontGL::updateCursor() {
	if (m_openglActive && m_cursorEnabled && m_cursorShader.program) {
		glUseProgram(m_cursorShader.program);
		checkGLError();

		glUniform2f(m_cursorShader.aCursorpos, m_cursorCol, m_rows - m_cursorRow - 1);
		checkGLError();

		glUniform1i(m_cursorShader.aCursorstyle, m_cursorStyle >> 1); // strip blink bit
		checkGLError();

		// only block uses alpha value
		float curAlpha = (m_cursorStyle < 2 ? 0.5 : 1.0);
		if (m_cursorColor > m_colors.size()) {
			glUniform4f(m_cursorShader.aCursorcolor, 0.5, 0.5, 0.5, curAlpha);
		} else {
			SDL_Color c = m_colors[m_cursorColor];
			glUniform4f(m_cursorShader.aCursorcolor, c.r/255.0, c.g/255.0, c.b/255.0, curAlpha);
		}
		checkGLError();
	}
}


void SDLFontGL::setCursor(bool enable, unsigned int row, unsigned int col, unsigned int color, TSCursorStyle style) {
	m_cursorEnabled = enable;
	m_cursorRow = row;
	m_cursorCol = col;
	m_cursorColor = color;
	m_cursorStyle = style;

	updateCursor();
}

void SDLFontGL::updateFonts() {
	openFonts();

	if (!m_openglActive || m_fontStyles.empty()) return;

	haveCacheLine = (bool*)malloc(m_fontStyles.size()*NSLOTS*sizeof(bool));
	assert(haveCacheLine);
	memset(haveCacheLine, 0, m_fontStyles.size()*NSLOTS*sizeof(bool));

	const Uint16 OStr[] = {'O', 0};
	int w = 0, h = 0;
	if (TTF_SizeUNICODE(m_fonts[0], OStr, &w, &h) != 0)
		assert(0 && "Failed to size font");
	assert((w > 0) && (h > 0));

	nWidth = w;
	nHeight = h;

	updateDimensions();

	// Set size of the texture, but no data.
	texW = nextPowerOfTwo(SLOTSIZE*nWidth);
	texH = nextPowerOfTwo(m_fontStyles.size()*NSLOTS*nHeight);

	syslog(LOG_DEBUG, "glyph texture size: %u (%u x %u)", texW * texH, texW, texH);

	if (m_charShader.program) {
		glUseProgram(m_charShader.program);
		checkGLError();

		glUniform2f(m_charShader.aGlyphsize, nWidth/(float)texW, nHeight/(float)texH);
		checkGLError();
	}

	// Create Big GL texture:
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &m_glyphTex);
	checkGLError();
	glBindTexture(GL_TEXTURE_2D, m_glyphTex);
	checkGLError();

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	checkGLError();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	checkGLError();

	glTexImage2D(GL_TEXTURE_2D,
			0, GL_ALPHA,
			texW, texH,
			0, GL_ALPHA,
			GL_UNSIGNED_BYTE, NULL);
	checkGLError();

	getGlyphSlot(0, 0);
}

void SDLFontGL::clearFonts() {
	// this clears font caches in normal RAM too

	// TODO: cache glyphs in ram?

	clearGLFonts();
}

void SDLFontGL::clearGLFonts() {
	if (m_glyphTex) {
		glDeleteTextures(1, &m_glyphTex);
		checkGLError();
		m_glyphTex = 0;
	}

	free(haveCacheLine);
	haveCacheLine = 0;

	// the fonts use SDL/opengl resources too, and have to reopened after a resize
	m_fontsLoaded = false;
	for (unsigned int i = 0; i < m_fonts.size(); i++) {
		TTF_CloseFont(m_fonts[i]);
		checkGLError();
	}
	m_fonts.clear();
}

void SDLFontGL::openFonts() {
	clearGLFonts();

	if (!m_openglActive || m_fontStyles.empty()) return;

	for (unsigned int i = 0; i < m_fontStyles.size(); i++) {
		TTF_Font *font = TTF_OpenFont(m_fontFilename.c_str(), m_fontptsize);

		if (!font) {
			syslog(LOG_ERR, "couldn't open font file '%s'", m_fontFilename.c_str());
			abort();
		}

		TTF_SetFontStyle(font, m_fontStyles[i]);
		if (!TTF_FontFaceIsFixedWidth(font)) {
			syslog(LOG_ERR, "font '%s' with style %i doesn't have fixed width", m_fontFilename.c_str(), m_fontStyles[i]);
			abort();
		}
		m_fonts.push_back(font);
		checkGLError();
	}

	m_fontsLoaded = true;
}

void SDLFontGL::setFontSize(unsigned int ptsize) {
	if (m_fontptsize == ptsize) return;

	m_fontptsize = ptsize;

	clearFonts();
	clearText();
	updateFonts();
}

void SDLFontGL::setFontStyles(std::vector<int> styles) {
	if (styles.empty()) return;

	m_fontStyles = styles;

	clearFonts();
	clearText();
	updateFonts();
}

void SDLFontGL::setFontFilename(const char *filename) {
	m_fontFilename = filename;

	clearFonts();
	clearText();
	updateFonts();
}


unsigned int SDLFontGL::getGlyphSlot(unsigned int fnt, unsigned int slot) {
	assert(fnt < m_fontStyles.size());
	assert(slot < NSLOTS);
	assert(m_glyphTex);

	if (fnt >= m_fontStyles.size() || slot >= NSLOTS || !m_glyphTex || !haveCacheLine) return 0;

	unsigned int slotNdx = slot*m_fontStyles.size() + fnt;

	if (haveCacheLine[slotNdx]) return slotNdx;
	haveCacheLine[slotNdx] = true;

	if (!m_fontsLoaded) openFonts();

	// Lookup requested font
	TTF_Font * font = m_fonts[fnt];
	assert(font);

	// buffer for all characters
	unsigned char *glyphData = static_cast<unsigned char*>(malloc(texW * nHeight));
	assert(glyphData);
	// set texture to black
	memset(glyphData, 0, texW * nHeight);

	// Render font in white, will colorize on-the-fly
	SDL_Color fg = { 255, 255, 255 };

	// For each character, render the glyph, and put in the appropriate location
	for(Uint16 i = 0; i < SLOTSIZE; ++i) {
		// Lookup this character, and make a single-char string out of it.
		Uint16 C = unicode_slots[slot] + i;
		Uint16 buf[2] = { C, 0 };

		/* creates 32-bit ARGB surface; RGB is always fg, A is the interesting channel
		 * A is the the fourth byte in every component
		 */
		SDL_Surface* surface = TTF_RenderUNICODE_Blended(font, (const Uint16*)buf, fg);
		if (surface) {
			unsigned char *rectDst = glyphData + i * nWidth;
			unsigned char *rectSrc = static_cast<unsigned char*>(surface->pixels) + 3;
			for (unsigned int y = 0; y < (unsigned int) nHeight; y++) {
				unsigned char *dst = rectDst + (y * texW);
				unsigned char *src = rectSrc + (y * surface->pitch);
				for (unsigned int x = 0; x < (unsigned int) nWidth; x++, dst++, src += 4) {
					*dst = *src;
				}
			}

			SDL_FreeSurface(surface);
		}
	}

	// Now upload the big set of characters as a single texture:
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_glyphTex);
	checkGLError();

	// Upload this font to its place in the big texture
	glTexSubImage2D(GL_TEXTURE_2D, 0,
			0, slotNdx*nHeight,
			texW, nHeight,
			GL_ALPHA, GL_UNSIGNED_BYTE, glyphData);
	checkGLError();

	glFlush();
	checkGLError();

	free(glyphData);

	return slotNdx;
}

void SDLFontGL::drawGL(bool blink) {
	if (!m_openglActive) return;

	checkGLError();
	glViewport(m_curdimX, m_curdimY, m_curdimW, m_curdimH);

	if (m_charShader.program) {
		// bind textures for character, glyphs and colors
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_cellsTex);
		checkGLError();

		// upload all character on each draw. TODO: remember whether it/what changed?
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_cellsWidth, m_cellsHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_cellData);
		checkGLError();

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_glyphTex);
		checkGLError();

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_colorTex);
		checkGLError();

		// draw characters
		glUseProgram(m_charShader.program);
		checkGLError();

		glUniform1i(m_charShader.aBlink, blink);
		checkGLError();

		glEnableVertexAttribArray(m_charShader.aPos);
		checkGLError();

		glDrawArrays(GL_TRIANGLES, 0, 6);
		checkGLError();

		glDisableVertexAttribArray(m_charShader.aPos);
		checkGLError();
	}

	if (m_cursorEnabled && (blink || (0 != m_cursorStyle % 2)) && m_cursorShader.program) {
		// draw cursor
		glUseProgram(m_cursorShader.program);
		checkGLError();

		glEnableVertexAttribArray(m_cursorShader.aPos);
		checkGLError();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		checkGLError();
		glDisable(GL_BLEND);

		glDisableVertexAttribArray(m_cursorShader.aPos);
		checkGLError();
	}

	checkGLError();
}

void SDLFontGL::drawTextGL(TextGraphicsInfo & graphicsInfo, unsigned int col, unsigned int row, Uint16 cChar) {
	if (!m_cellData) return;

	int slot = m_slotMap[cChar >> 7];
	if (slot == -1) {
		cChar = 0x2595;
		slot = m_slotMap[cChar >> 7];
		if (slot == -1) {
			slot = 0;
			cChar = '?';
		}
	}

	slot = getGlyphSlot(graphicsInfo.font, slot);

	if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
		unsigned int ndx = 4*(col + m_cellsWidth*(m_rows - row - 1));
		m_cellData[ndx] = (cChar & 0x7f) << 1;
		m_cellData[ndx+1] = slot << 1;
		m_cellData[ndx+2] = (graphicsInfo.fg & 0x3f) | (graphicsInfo.blink ? 0x40: 0);
		m_cellData[ndx+3] = graphicsInfo.bg;
	} else {
		// syslog(LOG_DEBUG, "drawTextGL out of bound offset (row %i, col %i) for size (%i, %i)", row, col, m_rows, m_cols);
	}

	return;
}

void SDLFontGL::clearText() {
	if (m_cellData) {
		memset(m_cellData, 0, 4*m_cellsWidth * m_cellsHeight);
		// Set default background, used for unspecified cells.
		for(unsigned row = 0; row < m_rows; ++row) {
			for(unsigned col = 0; col < m_cols; ++col) {
				unsigned int ndx = 4*(col + m_cellsWidth*(m_rows - row - 1));
				m_cellData[ndx+3] = TS_COLOR_BACKGROUND;
			}
		}
	}
}
