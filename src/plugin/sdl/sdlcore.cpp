/**
 * This file is part of SDLTerminal.
 * Copyright (C) 2011 Vincent Ho <www.whimsicalvee.com>
 * Copyright (C) 2011-2012 Ryan Hope <rmh3093@gmail.com>
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

#include "sdlcore.hpp"

#include <GLES/gl.h>
#include <SDL/SDL_image.h>

#include <math.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

const int SDLCore::BUFFER_DIRTY_BIT = 1;
const int SDLCore::FONT_DIRTY_BIT = 2;
const int SDLCore::FOREGROUND_COLOR_DIRTY_BIT = 4;
const int SDLCore::BACKGROUND_COLOR_DIRTY_BIT = 8;

SDLCore::SDLCore()
{
	m_bRunning = false;

	m_surface = NULL;
	m_nFontSize = 12;

	m_foregroundColor = TS_COLOR_FOREGROUND;
	m_backgroundColor = TS_COLOR_BACKGROUND;
	m_bBold = false;
	m_bUnderline = false;
	m_bBlink = false;
	doBlink = false;
	m_slot1 = TS_CS_G0_ASCII;
	m_slot2 = TS_CS_G1_ASCII;

	m_reverse = false;

	m_fontNormal = NULL;
	m_fontBold = NULL;
	m_fontUnder = NULL;
	m_fontBoldUnder = NULL;
	m_nFontHeight = 0;
	m_nFontWidth = 0;

	clearDirty(0);
}

SDLCore::~SDLCore()
{
	if (isRunning())
	{
		pthread_join(m_blinkThread, NULL);
		shutdown();
	}
}

/**
 * Initializes SDL components. Returns -1 if an error occurs.
 */
int SDLCore::init()
{
	// Initialize the SDL library with the Video subsystem
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		syslog(LOG_ERR, "Cannot initialize SDL: %s", SDL_GetError());
		return -1;
	}

	if (TTF_Init() != 0)
	{
		syslog(LOG_ERR, "Cannot initialize SDL TTF: %s", TTF_GetError());
		return -1;
	}

	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1) != 0)
	{
		syslog(LOG_ERR, "Cannot set OpenGL version: %s", SDL_GetError());
		return -1;
	}

	m_surface = SDL_SetVideoMode(0, 0, 0, SDL_OPENGL);

	if (m_surface == NULL)
	{
		syslog(LOG_ERR, "Cannot create SDL framebuffer: %s", SDL_GetError());
		return -1;
	}

	if (createFonts(m_nFontSize) != 0)
	{
		syslog(LOG_ERR, "Cannot create default fonts.");
		return -1;
	}

	if (initOpenGL() != 0)
	{
		syslog(LOG_ERR, "Cannot initialize OpenGL.");
		return -1;
	}

	if (initCustom() != 0)
	{
		syslog(LOG_ERR, "Cannot initialize customizations.");
		return -1;
	}

	return 0;
}

int SDLCore::initOpenGL()
{
	const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();

	clearScreen(m_backgroundColor);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, videoInfo->current_w, videoInfo->current_h, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	resetGlyphCache();

	return 0;
}

int SDLCore::initCustom()
{
	return 0;
}

/**
 * Create a set of fonts with the current size. Returns -1 if an error occurs.
 * If an error occurs when changing size, the current valid fonts will not be invalidated.
 * Currently, the default font is assumed to be monospaced.
 */
int SDLCore::createFonts(int nSize)
{
	if (nSize < 1)
	{
		return -1;
	}

	// TODO: Let user set this?
	const char * Font = "./LiberationMono-Regular.ttf";

	m_fontNormal = TTF_OpenFont(Font, nSize);
	m_fontBold = TTF_OpenFont(Font, nSize);
	m_fontUnder = TTF_OpenFont(Font, nSize);
	m_fontBoldUnder = TTF_OpenFont(Font, nSize);

	if (!m_fontNormal || !m_fontBold || !m_fontUnder || !m_fontBoldUnder)
	{
		syslog(LOG_ERR, "Error loading font!");
		closeFonts();
		return -1;
	}

	if (TTF_SizeText(m_fontNormal, "O", &m_nFontWidth, &m_nFontHeight) != 0)
	{
		syslog(LOG_ERR, "Cannot calculate font size: %s", TTF_GetError());
		closeFonts();
		return -1;
	}

	// Set font styles:
	TTF_SetFontStyle(m_fontBold, TTF_STYLE_BOLD);
	TTF_SetFontStyle(m_fontUnder, TTF_STYLE_UNDERLINE);
	TTF_SetFontStyle(m_fontBoldUnder, TTF_STYLE_BOLD | TTF_STYLE_UNDERLINE);

	m_nMaxLinesOfText = getMaximumLinesOfText();
	m_nMaxColumnsOfText = getMaximumColumnsOfText();

	resetGlyphCache();

	setDirty(FONT_DIRTY_BIT);

	return 0;
}

void SDLCore::handleKeyboardEvent(SDL_Event &event)
{
}

void SDLCore::handleMouseEvent(SDL_Event &event)
{
}

void SDLCore::redraw()
{
}

void SDLCore::updateDisplaySize()
{
}

SDL_Color SDLCore::getColor(TSColor_t color)
{
}

/**
 * Main event loop. Does not return until the application exits.
 */
void SDLCore::eventLoop()
{
	// Event descriptor
	SDL_Event event;
	Uint32 lOldTime = SDL_GetTicks();
	Uint32 lCurrentTime = SDL_GetTicks();
	Uint32 lCycleTimeSlot = 25;
	Uint32 lDelay;

	while (isRunning()) {
		// Block for an event, and then handle all queued events.
		// This is important since input events (particularly mouse events)
		// shouldn't be synchronous with a redraw but especially because various
		// terminal-source events (a quickly scrolling buffer, for example) mark
		// the screen as dirty and force a refresh.  We should never end up trying
		// to draw faster than a controlled amount in all cases.
		SDL_WaitEvent(&event);
		do {
			switch (event.type)
			{
				case SDL_MOUSEMOTION:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					handleMouseEvent(event);
					break;
				case SDL_KEYUP:
				case SDL_KEYDOWN:
					handleKeyboardEvent(event);
					break;
				case SDL_VIDEOEXPOSE:
					redraw();
					setDirty(BUFFER_DIRTY_BIT);
					break;
				case SDL_VIDEORESIZE:
					closeFonts();
					m_surface = SDL_SetVideoMode(0, 0, 0, SDL_OPENGL);
					createFonts(m_nFontSize);
					initOpenGL();
					updateDisplaySize();
					redraw();
					break;
				case SDL_QUIT:
					return;
				default:
					break;
			}
		} while (SDL_PollEvent(&event));

		if (isDirty(FONT_DIRTY_BIT))
		{
			clearDirty(FONT_DIRTY_BIT);
			resetGlyphCache();
			redraw();
		}

		// Redraw if needed
		if (isDirty(BUFFER_DIRTY_BIT))
		{
			clearDirty(BUFFER_DIRTY_BIT);
			SDL_GL_SwapBuffers();
		}

		// Are we going too fast?  If so, sleep some accordingly.
		lCurrentTime = SDL_GetTicks();

		if ((lCurrentTime - lOldTime) < lCycleTimeSlot)
		{
			lDelay = lOldTime + lCycleTimeSlot - lCurrentTime;

			if (lDelay > lCycleTimeSlot)
			{
				lDelay = lCycleTimeSlot;
			}

			SDL_Delay(lDelay);
		}

		lOldTime = SDL_GetTicks();
	}
}

/**
 * Initializes SDL.
 */
void SDLCore::start()
{
	if (!isRunning())
	{
		m_bRunning = true;

		if (init() != 0)
		{
			shutdown();
		}
	}
}

int SDLCore::startBlinkThread()
{
	return pthread_create(&m_blinkThread, NULL, blinkThread, this);
}

void *SDLCore::blinkThread(void *ptr)
{
	SDL_Event event;
	event.type = SDL_VIDEOEXPOSE;
	SDLCore *core = (SDLCore *)ptr;
	while (core->isRunning()) {
		core->doBlink = !core->doBlink;
		SDL_PushEvent(&event);
		usleep(500000);
	}
	pthread_exit(NULL);
	return NULL;
}

/**
 * Starts the main application event loop. Will not return until the application exits.
 * If SDL is not initialized, then this will return immediately.
 */
void SDLCore::run()
{
	if (isRunning())
	{
		startBlinkThread();
		eventLoop();
	}
}

void SDLCore::closeFonts()
{
	//Releases current fonts.
	if (m_fontNormal != NULL)
	{
		TTF_CloseFont(m_fontNormal);
		m_fontNormal = NULL;
	}

	if (m_fontBold != NULL)
	{
		TTF_CloseFont(m_fontBold);
		m_fontBold = NULL;
	}

	if (m_fontUnder != NULL)
	{
		TTF_CloseFont(m_fontUnder);
		m_fontUnder = NULL;
	}

	if (m_fontBoldUnder != NULL)
	{
		TTF_CloseFont(m_fontBoldUnder);
		m_fontBoldUnder = NULL;
	}

	m_nFontHeight = 0;
	m_nFontWidth = 0;
}

/**
 * Releases all resources.
 */
void SDLCore::shutdown()
{
	closeFonts();

	if (TTF_WasInit() != 0)
	{
		TTF_Quit();
	}

	if (SDL_WasInit(SDL_INIT_VIDEO) != 0)
	{
		SDL_Quit();
	}

	m_bRunning = false;
	clearDirty(0);
}

/**
 * Checks whether the SDL application is running.
 */
bool SDLCore::isRunning()
{
	return m_bRunning;
}

int SDLCore::getFontSize()
{
	return m_nFontSize;
}

/**
 * Sets the size of the current font. Returns -1 if an error occurs.
 */
int SDLCore::setFontSize(int nSize)
{
	if (nSize < 1)
	{
		nSize = 1;
	}

	closeFonts();

	if (createFonts(nSize) != 0)
	{
		syslog(LOG_ERR, "Cannot set new font size.");
		return -1;
	}

	m_nFontSize = nSize;

	return 0;
}

/**
 * Draws a string on the display. Given the monospaced font, assumes that the display is a grid of text.
 * The top left corner of the screen is (1, 1). If the give location is out of bounds, then no action is taken.
 */
void SDLCore::printText(int nColumn, int nLine, const char *sText)
{
	if (nColumn < 1 || nLine < 1 || nColumn > m_nMaxColumnsOfText || nLine > m_nMaxLinesOfText)
	{
		return;
	}

	drawText((nColumn - 1) * m_nFontWidth, (nLine - 1) * m_nFontHeight, sText);
}

void SDLCore::drawRect(int nX, int nY, int nWidth, int nHeight, SDL_Color color, float fAlpha)
{
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLfloat vtx[] = {
		nX, nY + nHeight,
		nX + nWidth, nY + nHeight,
		nX + nWidth, nY,
		nX, nY
	};

	glColor4f(
		((float)color.r) / 255.0f,
		((float)color.g) / 255.0f,
		((float)color.b) / 255.0f,
		fAlpha);

	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glFlush();

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	setDirty(BUFFER_DIRTY_BIT);
}

void SDLCore::drawCursor(int nColumn, int nLine)
{
	int nX = (nColumn - 1) * m_nFontWidth;
	int nY = (nLine - 1) * m_nFontHeight;

	drawRect(nX, nY, m_nFontWidth, m_nFontHeight, m_reverse ? getColor(TS_COLOR_BACKGROUND) : getColor(TS_COLOR_FOREGROUND), 0.35f);
}

/**
 * Clears the screen with the current background color.
 */
void SDLCore::clearScreen(TSColor_t color)
{
	SDL_Color bkgd = getColor(color);
	glClearColor(
		((float)bkgd.r) / 255.0f,
		((float)bkgd.g) / 255.0f,
		((float)bkgd.b) / 255.0f,
		1.0f);

	glClear(GL_COLOR_BUFFER_BIT);

	setDirty(BUFFER_DIRTY_BIT);
}

void SDLCore::drawSurface(int nX, int nY, SDL_Surface *surface)
{
	int nMode = GL_RGB;

	if (m_surface->format->BytesPerPixel == 3)
	{
		//RGB 24bit
		nMode = GL_RGB;
	}
	else if (m_surface->format->BytesPerPixel == 4)
	{
		//RGBA 32bit
		nMode = GL_RGBA;
	}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	   Uint32 rmask = 0xff000000;
	   Uint32 gmask = 0x00ff0000;
	   Uint32 bmask = 0x0000ff00;
	   Uint32 amask = 0x000000ff;
#else
	   Uint32 rmask = 0x000000ff;
	   Uint32 gmask = 0x0000ff00;
	   Uint32 bmask = 0x00ff0000;
	   Uint32 amask = 0xff000000;
#endif

	SDL_Surface* mainSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, getNextPowerOfTwo(surface->w), getNextPowerOfTwo(surface->h), m_surface->format->BitsPerPixel, rmask, gmask, bmask, amask);
	GLuint texture = 0;

	if (mainSurface == NULL)
	{
		syslog(LOG_ERR, "Cannot create rendering surface.");
		return;
	}

	SDL_BlitSurface(surface, 0, mainSurface, 0);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, nMode, mainSurface->w, mainSurface->h, 0, nMode, GL_UNSIGNED_BYTE, mainSurface->pixels);

	GLfloat vtx[] = {
		nX, nY + mainSurface->h,
		nX + mainSurface->w, nY + mainSurface->h,
		nX + mainSurface->w, nY,
		nX, nY
	};

	GLfloat tex[] = {
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0
	};

	glVertexPointer(2, GL_FLOAT, 0, vtx);
	glTexCoordPointer(2, GL_FLOAT, 0, tex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glFlush();

	//Clean up.
	glDeleteTextures(1, &texture);
	SDL_FreeSurface(mainSurface);

	setDirty(BUFFER_DIRTY_BIT);
}

void SDLCore::drawImage(int nX, int nY, const char *sImage)
{
	SDL_Surface* surface = IMG_Load(sImage);

	if (surface == NULL)
	{
		syslog(LOG_ERR, "Cannot load image.");
		return;
	}

	SDL_SetAlpha(surface, 0, 0);
	drawSurface(nX, nY, surface);

	SDL_FreeSurface(surface);

	setDirty(BUFFER_DIRTY_BIT);
}

/**
 * Draws a string on an arbiturary location of the screen.
 */
void SDLCore::drawText(int nX, int nY, const char *sText)
{
	if (sText == NULL)
	{
		return;
	}
	
	// Match mapping in resetGlyphCache
	int fnt = 0;
	if (m_bBold && m_bUnderline)
		fnt = 1;
	else if (m_bUnderline)
		fnt = 2;
	else if (m_bBold)
		fnt = 3;

	SDLFontGL::TextGraphicsInfo_t graphicsInfo;
	graphicsInfo.font = fnt;
	if (m_reverse) {
		graphicsInfo.bg = (int)m_foregroundColor;
		graphicsInfo.fg = (int)m_backgroundColor;
	} else {
		graphicsInfo.fg = (int)m_foregroundColor;
		graphicsInfo.bg = (int)m_backgroundColor;
	}
	graphicsInfo.blink = ((int)m_bBlink && !(int)doBlink) ? 1 : 0;

	graphicsInfo.slot1 = (int)m_slot1;
	graphicsInfo.slot2 = (int)m_slot2;

	drawTextGL(graphicsInfo, nX, nY, sText);

	setDirty(BUFFER_DIRTY_BIT);
}

/**
 * Gets the maximum lines of text that can fit on the screen.
 * Calculation is performed using the current font.
 */
int SDLCore::getMaximumLinesOfText()
{
	const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();

	if (videoInfo == NULL)
	{
		return 0;
	}

	return (videoInfo->current_h / m_nFontHeight);
}

/**
 * Gets the maximum number of characters that fit on the screen on a single line.
 * Calculation is performed using the current font.
 */
int SDLCore::getMaximumColumnsOfText()
{
	const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();

	if (videoInfo == NULL)
	{
		return 0;
	}

	return (videoInfo->current_w / m_nFontWidth);
}

bool SDLCore::isDirty()
{
	return isDirty(0);
}

bool SDLCore::isDirty(int nDirtyBits)
{
	return ((m_nDirtyBits & nDirtyBits) != 0);
}

void SDLCore::setDirty(int nDirtyBits)
{
	m_nDirtyBits |= nDirtyBits;
}

/**
 * If 0 is passed in as an argument, all dirty bits will be cleared.
 */
void SDLCore::clearDirty(int nDirtyBits)
{
	if (nDirtyBits == 0)
	{
		m_nDirtyBits = 0;
	}
	else
	{
		m_nDirtyBits &= ~nDirtyBits;
	}
}

int SDLCore::getNextPowerOfTwo(int n) {
	if (n <= 0) return 1;

	/* http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return (n+1); /* warning: overflow possible */
}

void SDLCore::resetGlyphCache()
{
	int nFonts = 4;
	TTF_Font* fnts[4];

	// Match mapping in drawText()
	fnts[0] = m_fontNormal;
	fnts[1] = m_fontBoldUnder;
	fnts[2] = m_fontUnder;
	fnts[3] = m_fontBold;

	int nCols = TS_COLOR_MAX;
	SDL_Color cols[nCols];
	for(unsigned i = 0; i < TS_COLOR_MAX; ++i)
		cols[i] = getColor((TSColor_t)i);

	setupFontGL(nFonts, (TTF_Font**)fnts, nCols, (SDL_Color*)&cols);
}

// pulled from SDL_keyboard.c / lgpl Copyright (C) 1997-2006 Sam Lantinga
void SDLCore::fakeKeyEvent(SDL_Event &event)
{
	Uint16 modstate;
	Uint8 state;
	int map;

	modstate = (Uint16)SDL_GetModState();

	if (event.type == SDL_KEYDOWN) 
	{
		state = SDL_PRESSED;
		event.key.keysym.mod = (SDLMod)modstate;
		switch (event.key.keysym.sym) 
		{
			case SDLK_UNKNOWN:
				break;
			case SDLK_NUMLOCK:
				modstate ^= KMOD_NUM;
				if ( ! (modstate&KMOD_NUM) )
					state = SDL_RELEASED;
				event.key.keysym.mod = (SDLMod)modstate;
				break;
			case SDLK_CAPSLOCK:
				modstate ^= KMOD_CAPS;
				if ( ! (modstate&KMOD_CAPS) )
					state = SDL_RELEASED;
				event.key.keysym.mod = (SDLMod)modstate;
				break;
			case SDLK_LCTRL:
				modstate |= KMOD_LCTRL;
				break;
			case SDLK_RCTRL:
				modstate |= KMOD_RCTRL;
				break;
			case SDLK_LSHIFT:
				modstate |= KMOD_LSHIFT;
				break;
			case SDLK_RSHIFT:
				modstate |= KMOD_RSHIFT;
				break;
			case SDLK_LALT:
				modstate |= KMOD_LALT;
				break;
			case SDLK_RALT:
				modstate |= KMOD_RALT;
				break;
			case SDLK_LMETA:
				modstate |= KMOD_LMETA;
				break;
			case SDLK_RMETA:
				modstate |= KMOD_RMETA;
				break;
			case SDLK_MODE:
				modstate |= KMOD_MODE;
				break;
			default:
				break;
		}
	} 
	else  // key up
	{
		state = SDL_RELEASED;
		switch (event.key.keysym.sym) 
		{
			case SDLK_UNKNOWN:
				break;
			case SDLK_NUMLOCK:
			case SDLK_CAPSLOCK:
				/* Only send keydown events */
				return;
			case SDLK_LCTRL:
				modstate &= ~KMOD_LCTRL;
				break;
			case SDLK_RCTRL:
				modstate &= ~KMOD_RCTRL;
				break;
			case SDLK_LSHIFT:
				modstate &= ~KMOD_LSHIFT;
				break;
			case SDLK_RSHIFT:
				modstate &= ~KMOD_RSHIFT;
				break;
			case SDLK_LALT:
				modstate &= ~KMOD_LALT;
				break;
			case SDLK_RALT:
				modstate &= ~KMOD_RALT;
				break;
			case SDLK_LMETA:
				modstate &= ~KMOD_LMETA;
				break;
			case SDLK_RMETA:
				modstate &= ~KMOD_RMETA;
				break;
			case SDLK_MODE:
				modstate &= ~KMOD_MODE;
				break;
			default:
				break;
		}
		event.key.keysym.mod = (SDLMod)modstate;
	}

	if (event.key.keysym.sym != SDLK_UNKNOWN) 
	{
		/* Drop events that don't change state */
		Uint8 *keyState = SDL_GetKeyState(NULL);
		if (keyState[event.key.keysym.sym] == state)
			return;

		/* Update internal keyboard state */
		keyState[event.key.keysym.sym] = state;
		SDL_SetModState((SDLMod)modstate);
	}

	SDL_PushEvent(&event);
}
