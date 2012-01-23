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

#include "sdl/sdlfontgl.h"

#include <syslog.h>

// Log assertion failures to syslog
#define assert(c) \
	do { \
		if (!(c)) \
			syslog(LOG_ERR, "Assertion \"%s\" failed at %s:%d", \
					__STRING(c), __FILE__, __LINE__); \
	} while(0)

// For debugging
#define checkGLError() \
	do { \
		int err = glGetError(); \
		if (err) syslog(LOG_ERR, "GL Error %x at %s:%d", \
				err, __FILE__, __LINE__); \
	} while(0)

// How many characters do we support?
static const int nChars = 128;

// Helper functions
static void getRGBAMask(Uint32 &rmask, Uint32 &gmask,
												Uint32 &bmask, Uint32 &amask) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
#endif
}

static int getGLFormat() {
	SDL_Surface *s = SDL_GetVideoSurface();
	if (s->format->BytesPerPixel == 3)
		return GL_RGB;
	if (s->format->BytesPerPixel == 4)
		return GL_RGBA;
	assert(0 && "Unsupported bpp");
	return -1;
}

static unsigned nextPowerOfTwo(int n) {
	assert(n > 0);

	unsigned res = n;

	/* http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */
	--res;
	res |= res >> 1;
	res |= res >> 2;
	res |= res >> 4;
	res |= res >> 8;
	res |= res >> 16;
	return (res+1); /* warning: overflow possible */
}

void SDLFontGL::setupFontGL(int fnCount, TTF_Font** fnts, int colCount, SDL_Color *cols) {
	clearGL();

	assert(fnts && cols);
	assert((fnCount > 0) && (colCount > 0));

	nFonts = fnCount;
	nCols = colCount;

	this->fnts = (TTF_Font**)malloc(nFonts*sizeof(TTF_Font*));
	memcpy(this->fnts, fnts, nFonts*sizeof(TTF_Font*));

	this->cols = (SDL_Color*)malloc(nCols*sizeof(SDL_Color));
	memcpy(this->cols, cols, nCols*sizeof(SDL_Color));

	GlyphCache = 0;

	haveCacheLine = (bool*)malloc(nFonts*MAX_CHARSETS*sizeof(bool));
	memset(haveCacheLine, 0, nFonts*MAX_CHARSETS*sizeof(bool));

	const Uint16 OStr[] = {'O', 0};
	if (TTF_SizeUNICODE(fnts[0], OStr, &nWidth, &nHeight) != 0)
		assert(0 && "Failed to size font");
	assert((nWidth > 0) && (nHeight > 0));
}

void SDLFontGL::createTexture() {
	// Create Big GL texture:
	glGenTextures(1,&GlyphCache);
	glBindTexture(GL_TEXTURE_2D, GlyphCache);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Set size of the texture, but no data.
	// We want 1 extra row of pixel data
	// so we can draw solid colors as part
	// of the same operations.
	texW = nextPowerOfTwo(nChars*nWidth);
	texH = nextPowerOfTwo(nFonts*MAX_CHARSETS*nHeight + 1);
	int nMode = getGLFormat();
	glTexImage2D(GL_TEXTURE_2D,
			0, nMode,
			texW, texH,
			0, nMode,
			GL_UNSIGNED_BYTE, NULL);
	checkGLError();


	// Put a single white pixel at bottom of texture.
	// We use this as the 'texture' data for blitting
	// solid backgrounds.
	char whitepixel[] = { 255, 255, 255, 255 };
	assert(nFonts && nHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0,
			0,nFonts*MAX_CHARSETS*nHeight,
			1, 1,
			GL_RGBA, GL_UNSIGNED_BYTE, whitepixel);
	checkGLError();
}

SDLFontGL::~SDLFontGL() {
	clearGL();
}

void SDLFontGL::clearGL() {
	if (GlyphCache) {
		glDeleteTextures(1, &GlyphCache);
		GlyphCache = 0;
	}

	free(haveCacheLine);
	free(fnts);
	free(cols);

	haveCacheLine = 0;
	fnts = NULL;
	cols = NULL;

	nFonts = nCols = 0;
	numChars = 0;
}

void SDLFontGL::ensureCacheLine(unsigned int fnt, unsigned int slot)
{

	assert(fnt >= 0 && fnt < nFonts);
	assert(slot >= 0 && slot < MAX_CHARSETS);
	assert(fnts && cols && GlyphCache && haveCacheLine);

	bool & have = hasCacheLine(fnt, slot);
	if (have) {
		return;
	}
	have = true;

	// Lookup requested font
	TTF_Font * font = fnts[fnt];
	assert(font);

	// Grab the native video surface (so we can match its bpp)
	SDL_Surface* videoSurface = SDL_GetVideoSurface();
	assert(videoSurface);
	assert(videoSurface->format->BitsPerPixel == 32);

	// Create a surface for all the characters
	Uint32 rmask, gmask, bmask, amask;
	getRGBAMask(rmask, gmask, bmask, amask);
	SDL_Surface* mainSurface =
		SDL_CreateRGBSurface(SDL_SWSURFACE,
				nChars*nWidth, nHeight,
				videoSurface->format->BitsPerPixel,
				rmask, gmask, bmask, amask);
	assert(mainSurface);

	// Render font in white, will colorize on-the-fly
	SDL_Color fg = { 255, 255, 255 };

	// Set texture to entirely clear
	// TODO: Needed?
	Uint32 fillColor = SDL_MapRGBA(mainSurface->format, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
	SDL_FillRect(mainSurface, NULL, fillColor);

	// For each character, render the glyph, and put in the appropriate location
	for(int i = 0; i < nChars; ++i)
	{
		// Lookup this character, and make a single-char string out of it.
		Uint16 C = charMappings[slot].map[i];
		if (!C) C = (Uint16)i;
		Uint16 buf[2] = { C, 0 };

		SDL_Surface* surface = TTF_RenderUNICODE_Blended(font, (const Uint16*)buf, fg);
		if (surface)
		{
			SDL_SetAlpha(surface, 0, 0);

			SDL_Rect dstRect = { 0, 0, nWidth, nHeight };
			dstRect.x = i*nWidth;

			SDL_BlitSurface(surface, 0, mainSurface, &dstRect);

			SDL_FreeSurface(surface);
		}
	}

	// Now upload the big set of characters as a single texture:
	{
		int nMode = getGLFormat();

		glBindTexture(GL_TEXTURE_2D, GlyphCache);

		// Upload this font to its place in the big texture
		glTexSubImage2D(GL_TEXTURE_2D, 0,
				0, (slot*nFonts + fnt)*nHeight,
				mainSurface->w, mainSurface->h,
				nMode, GL_UNSIGNED_BYTE, mainSurface->pixels);
		glFlush();
	}

	SDL_FreeSurface(mainSurface);
}

void SDLFontGL::drawBackground(int color, int nX, int nY, int cells) {
	// Blit a rectangle of the specified color at the specified coordinates
	// (Err, don't blit, but insert into our arrays the equivalent)
	const int stride = 12;
	GLfloat *tex = &texValues[stride*numChars];
	GLfloat *vtx = &vtxValues[stride*numChars];
	GLfloat *clrs = &colorValues[2*stride*numChars];
	++numChars;

	GLfloat vtxCopy[] = {
		nX, nY,
		nX, nY + nHeight,
		nX + cells*nWidth, nY,
		nX, nY + nHeight,
		nX + cells*nWidth, nY,
		nX + cells*nWidth, nY + nHeight
	};
	memcpy(vtx, vtxCopy, sizeof(vtxCopy));

	float y_offset = ((float)nFonts*MAX_CHARSETS*nHeight)/(float)texH;
	float x1 = ((float)1)/(float)texW;
	float y1 = ((float)1)/(float)texH;
	GLfloat texCopy[] = {
		0.0, y_offset,
		0.0, y_offset + y1,
		x1,  y_offset,
		0.0, y_offset + y1,
		x1,  y_offset,
		x1,  y_offset + y1
	};
	memcpy(tex, texCopy, sizeof(texCopy));

	// Duplicate color...
	SDL_Color bgc = cols[color];
	GLfloat colorCopy[] = {
			((float)bgc.r)/255.f,
			((float)bgc.g)/255.f,
			((float)bgc.b)/255.f,
			1.f
	};

	for(unsigned i = 0; i < 6; ++i) {
		memcpy(&clrs[i*4], colorCopy, sizeof(colorCopy));
	}
}

void SDLFontGL::drawTextGL(TextGraphicsInfo_t & graphicsInfo,
													 int nX, int nY, Uint16 cChar) {
	if (!GlyphCache) createTexture();

	unsigned int fnt = graphicsInfo.font;
	unsigned int fg = graphicsInfo.fg;
	unsigned int bg = graphicsInfo.bg;
	int blink = graphicsInfo.blink;

	assert(fnt >= 0 && fnt < nFonts);
	assert(fg >= 0 && fg < nCols);
	assert(bg >= 0 && bg < nCols);
	assert(fnts && cols && GlyphCache);

	const unsigned int stride = 12; // GL_TRIANGLE_STRIP 2*6

	// Is our operation buffer full?
	// If so, flush it now.
	if (numChars > RENDER_BUFFER_SIZE-2)
		flushGLBuffer();

	drawBackground(bg, nX, nY, 1);

	if (blink) return;

	GLfloat *tex = &texValues[stride*numChars];
	GLfloat *vtx = &vtxValues[stride*numChars];
	GLfloat *clrs = &colorValues[2*stride*numChars];
	numChars += 1;

	float x_scale = ((float)nWidth) / (float)texW;
	float y_scale = ((float)nHeight) / (float)texH;
	GLfloat texCopy[] = {
		0.0, 0.0,
		0.0, y_scale,
		x_scale, 0.0,
		0.0, y_scale,
		x_scale, 0.0,
		x_scale, y_scale
	};
	GLfloat vtxCopy[] = {
		nX, nY,
		nX, nY + nHeight,
		nX + nWidth, nY,
		nX, nY + nHeight,
		nX + nWidth, nY,
		nX + nWidth, nY + nHeight
	};
	SDL_Color fgc = cols[fg];
	GLfloat colorCopy[] = {
			((float)fgc.r)/255.f,
			((float)fgc.g)/255.f,
			((float)fgc.b)/255.f,
			1.f,
			((float)fgc.r)/255.f,
			((float)fgc.g)/255.f,
			((float)fgc.b)/255.f,
			1.f,
			((float)fgc.r)/255.f,
			((float)fgc.g)/255.f,
			((float)fgc.b)/255.f,
			1.f,
			((float)fgc.r)/255.f,
			((float)fgc.g)/255.f,
			((float)fgc.b)/255.f,
			1.f,
			((float)fgc.r)/255.f,
			((float)fgc.g)/255.f,
			((float)fgc.b)/255.f,
			1.f,
			((float)fgc.r)/255.f,
			((float)fgc.g)/255.f,
			((float)fgc.b)/255.f,
			1.f
	};

	// Populate texture coordinates
	memcpy(tex, texCopy, sizeof(texCopy));

	int x,y;
	getTextureCoordinates(graphicsInfo, cChar, x, y);

	float x_offset = ((float)x) / (float)texW;
	float y_offset = ((float)y) / (float)texH;

	for(unsigned j = 0; j < stride; j += 2) {
		tex[j] += x_offset;
		tex[j+1] += y_offset;
	}

	// Populate vertex coordinates
	memcpy(vtx,vtxCopy,sizeof(vtxCopy));

	// Populate color coodinates
	memcpy(clrs, colorCopy, sizeof(colorCopy));
}

void SDLFontGL::startTextGL() {
	// Start over in buffer
	numChars = 0;

	glClear(GL_COLOR_BUFFER_BIT);

	// Bind the master font texture
	glBindTexture(GL_TEXTURE_2D, GlyphCache);
	glEnableClientState(GL_COLOR_ARRAY);

	// Point GL to our arrays...
	glColorPointer(4, GL_FLOAT, 0, colorValues);
	glTexCoordPointer(2, GL_FLOAT, 0, texValues);
	glVertexPointer(2, GL_FLOAT, 0, vtxValues);
}

void SDLFontGL::flushGLBuffer() {
	// Render what we've gathered so far
	glDrawArrays(GL_TRIANGLES, 0, 6*numChars);
	numChars = 0;
}

void SDLFontGL::endTextGL() {
	flushGLBuffer();

	glDisableClientState(GL_COLOR_ARRAY);
	glFlush();
}

void SDLFontGL::setCharMapping(int index, CharMapping_t map) {
	assert(index >= 0 && index < MAX_CHARSETS);
	memcpy(&charMappings[index], &map, sizeof(CharMapping_t));

	// Invalidate the appropriate cache lines
	for(unsigned i = 0; i < nFonts; ++i)
		hasCacheLine(i, index) = false;
}

void SDLFontGL::getTextureCoordinates(TextGraphicsInfo_t & graphicsInfo, Uint16 c, int &x, int &y) {
	/* TODO: handle unicode chars */
	if (c > 0x100) c = '?';

	int slot = c > 127 ? graphicsInfo.slot1 : graphicsInfo.slot2;
	ensureCacheLine(graphicsInfo.font, slot);

	// Set by reference
	x = (c % 128)*nWidth;
	y = (slot*nFonts + graphicsInfo.font)*nHeight;
}

bool &SDLFontGL::hasCacheLine(unsigned int font, unsigned int slot) {
	return haveCacheLine[slot*nFonts + font];
}
