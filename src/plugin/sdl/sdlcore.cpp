/**
 * This file is part of wTerm.
 * Copyright (C) 2011 Vincent Ho <www.whimsicalvee.com>
 * Copyright (C) 2011-2012 Ryan Hope <rmh3093@gmail.com>
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

#include "sdlcore.hpp"

#include <GLES2/gl2.h>
#include <SDL/SDL_image.h>
#include "util/glutils.hpp"
#include "util/utils.hpp"

#include <math.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

const int SDLCore::BUFFER_DIRTY_BIT = 1;
const int SDLCore::FONT_SIZE_DIRTY_BIT = 2;
const int SDLCore::COLOR_DIRTY_BIT = 4;
const int SDLCore::BLINK_DIRTY_BIT = 8;

SDLCore::SDLCore()
: m_fontgl("./LiberationMono-Regular.ttf", 12)
{
	m_bRunning = false;
	m_blinkThread = NULL;

	m_surface = NULL;

	m_fontSize = m_fontgl.getFontSize();

	doBlink = false;
	m_bNeedsBlink = false;

	m_reverse = false;

	m_keyRepeat.firsttime = 0;
	m_keyRepeat.delay = 500; // 500
	m_keyRepeat.interval = 35; // 35
	m_keyRepeat.timestamp = 0;

	active = true;
	lCycleTimeSlot = 25;

	clearDirty(0);
}

SDLCore::~SDLCore()
{
	if (isRunning())
	{
		if (m_blinkThread) pthread_join(m_blinkThread, NULL);
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

	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2) != 0)
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

	pushFontStyles();

	if (initOpenGL() != 0)
	{
		syslog(LOG_ERR, "Cannot initialize OpenGL.");
		return -1;
	}

	pushColors();

	if (initCustom() != 0)
	{
		syslog(LOG_ERR, "Cannot initialize customizations.");
		return -1;
	}

	return 0;
}

void SDLCore::pushColors() {
	std::vector<SDL_Color> colors;
	for(unsigned i = 0; i < TS_COLOR_MAX; ++i)
		colors.push_back(getColor((TSColor)i));
	m_fontgl.setupColors(colors);
}

void SDLCore::pushFontStyles() {
	std::vector<int> fontStyles;
	fontStyles.push_back(0);
	fontStyles.push_back(TTF_STYLE_BOLD);
	fontStyles.push_back(TTF_STYLE_UNDERLINE);
	fontStyles.push_back(TTF_STYLE_BOLD | TTF_STYLE_UNDERLINE);

	m_fontgl.setFontStyles(fontStyles);
}


int SDLCore::initOpenGL()
{
	const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();

	if (!videoInfo) return -1;

	m_fontgl.initGL(0, 0, videoInfo->current_w, videoInfo->current_h);

	clearScreen(TS_COLOR_BACKGROUND);

	return 0;
}

void SDLCore::handleKeyboardEvent(SDL_Event &event)
{
}

void SDLCore::handleMouseEvent(SDL_Event &event)
{
}

void SDLCore::setActive(int active)
{
	this->active = active;
	lCycleTimeSlot = this->active ? 25 : 1000;
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
	Uint32 lDelay;

	while (isRunning()) {
		// If a key repeat event is not active:
		// Block for an event, and then handle all queued events.
		// This is important since input events (particularly mouse events)
		// shouldn't be synchronous with a redraw but especially because various
		// terminal-source events (a quickly scrolling buffer, for example) mark
		// the screen as dirty and force a refresh.  We should never end up trying
		// to draw faster than a controlled amount in all cases.

		bool gotEvent = false;
		if (!m_keyRepeat.timestamp)
		{
			SDL_WaitEvent(&event);
			gotEvent = true;
		}
		else
			gotEvent = SDL_PollEvent(&event);

		while (gotEvent)
		{
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
					setDirty(BUFFER_DIRTY_BIT);
					break;
				case SDL_VIDEORESIZE:
					m_fontgl.clearGL();
					m_surface = SDL_SetVideoMode(0, 0, 0, SDL_OPENGL);
					m_fontgl.initGL(0, 0, event.resize.w, event.resize.h);
					updateDisplaySize();
					setDirty(BUFFER_DIRTY_BIT);
					break;
				case SDL_QUIT:
					return;
				case SDL_USEREVENT: // blink
					setDirty(BLINK_DIRTY_BIT);
					break;
				default:
					break;
			}
			gotEvent = SDL_PollEvent(&event);
		}
		checkKeyRepeat();


		if (isDirty(FONT_SIZE_DIRTY_BIT) && active)
		{
			clearDirty(FONT_SIZE_DIRTY_BIT);
			m_fontgl.setFontSize(m_fontSize);
			updateDisplaySize();
			setDirty(BUFFER_DIRTY_BIT);
		}

		if (isDirty(COLOR_DIRTY_BIT)) {
			clearDirty(COLOR_DIRTY_BIT);
			pushColors();
		}

		// Redraw if needed
		if (isDirty(BUFFER_DIRTY_BIT) && active)
		{
			clearDirty(BUFFER_DIRTY_BIT | BLINK_DIRTY_BIT);
			redraw();
			glFlush();
			checkGLError();
			SDL_GL_SwapBuffers();
			checkGLError();
		} else if (isDirty(BLINK_DIRTY_BIT) && active) {
			clearDirty(BLINK_DIRTY_BIT);
			redrawBlinked();
			glFlush();
			checkGLError();
			SDL_GL_SwapBuffers();
			checkGLError();
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
	event.type = SDL_USEREVENT;
	SDLCore *core = (SDLCore *)ptr;
	while (core->isRunning()) {
		core->doBlink = !core->doBlink;
		// Only bother redrawing if we have any blink text on-screen
		if (core->m_bNeedsBlink)
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

/**
 * Releases all resources.
 */
void SDLCore::shutdown()
{
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

unsigned int SDLCore::getFontSize()
{
	return m_fontgl.getFontSize();
}

/**
 * Sets the size of the current font.
 */
void SDLCore::setFontSize(unsigned int nSize) {
	m_fontSize = nSize;

	setDirty(FONT_SIZE_DIRTY_BIT);
}

/**
 * Draws a string on the display. Given the monospaced font, assumes that the display is a grid of text.
 * The top left corner of the screen is (1, 1). If the give location is out of bounds, then no action is taken.
 */
void SDLCore::printCharacter(int nColumn, int nLine, TSCell cell)
{
	// Match mapping in pushFontStyles
	int fnt = (cell.graphics.bold() ? 1 : 0) | (cell.graphics.underline() ? 2 : 0);

	TSColor fg, bg;

	if (cell.graphics.negative()) {
		fg = cell.graphics.backgroundColor;
		bg = cell.graphics.foregroundColor;
		if (bg > 7 && bg < 16) {
			bg = (TSColor) (bg - 8);
		} else if (bg > 17) {
			bg = (TSColor) (bg - 2);
		}
	} else {
		fg = cell.graphics.foregroundColor;
		bg = cell.graphics.backgroundColor;
	}

	SDLFontGL::TextGraphicsInfo graphicsInfo;
	graphicsInfo.font = fnt;
	if (m_reverse) {
		graphicsInfo.bg = (int)fg;
		graphicsInfo.fg = (int)bg;
	} else {
		graphicsInfo.fg = (int)fg;
		graphicsInfo.bg = (int)bg;
	}

	if (cell.graphics.bold()) {
		if (graphicsInfo.fg < TS_COLOR_BLACK_BRIGHT) {
			graphicsInfo.fg += TS_COLOR_BLACK_BRIGHT;
		} else {
			graphicsInfo.fg += 2; // TS_..._FG/BG -> _FG/BG_BRIGHT
		}
	}
	graphicsInfo.blink = cell.graphics.blink();

	m_fontgl.drawTextGL(graphicsInfo, nColumn-1, nLine-1, cell.data);

	setDirty(BUFFER_DIRTY_BIT);
}

/**
 * Clears the screen with the current background color.
 */
void SDLCore::clearScreen(TSColor color)
{
	SDL_Color bkgd = getColor(color);
	glClearColor(
		((float)bkgd.r) / 255.0f,
		((float)bkgd.g) / 255.0f,
		((float)bkgd.b) / 255.0f,
		1.0f);
	checkGLError();

	glClear(GL_COLOR_BUFFER_BIT);
	checkGLError();

	setDirty(BUFFER_DIRTY_BIT);
}

/**
 * Gets the maximum lines of text that can fit on the screen.
 * Calculation is performed using the current font.
 */
int SDLCore::getMaximumLinesOfText()
{
	return m_fontgl.rows();
}

/**
 * Gets the maximum number of characters that fit on the screen on a single line.
 * Calculation is performed using the current font.
 */
int SDLCore::getMaximumColumnsOfText()
{
	return m_fontgl.cols();
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

void SDLCore::stopKeyRepeat()
{
	m_keyRepeat.timestamp = 0;
	return;
}

// pulled from SDL_keyboard.c / lgpl Copyright (C) 1997-2006 Sam Lantinga
void SDLCore::checkKeyRepeat()
{
	//syslog(LOG_ERR, "check ran %i", SDL_GetTicks());
	if (m_keyRepeat.timestamp)
	{
		Uint32 now, interval;
		now = SDL_GetTicks();
		interval = (now - m_keyRepeat.timestamp);
		//syslog(LOG_ERR, "Key Repeat Active [now,interval,delay,timestamp,first] %i - %i - %i - %i - %i",now,interval,m_keyRepeat.delay,m_keyRepeat.timestamp,m_keyRepeat.firsttime);
		if (m_keyRepeat.firsttime)
		{
			if (interval > (Uint32)m_keyRepeat.delay)
			{
				m_keyRepeat.timestamp = now;
				m_keyRepeat.firsttime = 0;
				//syslog(LOG_ERR, "First time delay hit");
			}
		}
		else
		{
			if (interval > (Uint32)m_keyRepeat.interval)
			{
				m_keyRepeat.timestamp = now;
				SDL_PushEvent(&m_keyRepeat.evt);
				//syslog(LOG_ERR, "Pushed Repeat Event");
			}
		}
	}
}

// pulled from SDL_keyboard.c / lgpl Copyright (C) 1997-2006 Sam Lantinga
void SDLCore::fakeKeyEvent(SDL_Event &event)
{
	int repeatable = 0;
	Uint16 modstate;
	Uint8 state;

	modstate = (Uint16)SDL_GetModState();

	if (event.type == SDL_KEYDOWN)
	{
		state = SDL_PRESSED;
		event.key.keysym.mod = (SDLMod)modstate;
		switch (event.key.keysym.sym)
		{
			case SDLK_UNKNOWN:
				repeatable = 1;
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
				repeatable = 1;
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

	if (event.type == SDL_KEYUP)
	{
		if (m_keyRepeat.timestamp && m_keyRepeat.evt.key.keysym.sym == event.key.keysym.sym)
		{
			m_keyRepeat.timestamp = 0;
//			syslog(LOG_ERR, "Removed Repeat Event");
		}
	}
	else
	{
		if (repeatable && m_keyRepeat.delay != 0)
		{
			m_keyRepeat.evt = event;
			m_keyRepeat.firsttime = 1;
			m_keyRepeat.timestamp = SDL_GetTicks();
//			syslog(LOG_ERR, "Added Repeat Event");
		}
	}

	SDL_PushEvent(&event);
}
