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

#ifndef SDLCORE_HPP__
#define SDLCORE_HPP__

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <map>

#include "terminal/vtterminalstate.hpp"
#include "sdlfontgl.hpp"

/**
 * Initializer and basic 2D function for webOS SDL.
 */
class SDLCore
{
protected:
	static const int BUFFER_DIRTY_BIT;
	static const int FONT_SIZE_DIRTY_BIT;
	static const int COLOR_DIRTY_BIT;
	static const int BLINK_DIRTY_BIT;

	SDLFontGL m_fontgl;

	SDL_Surface *m_surface;
	bool doBlink;
	bool m_bNeedsBlink;

	bool m_reverse;

	bool active;
	Uint32 lCycleTimeSlot;

	pthread_t m_blinkThread;
	static void *blinkThread(void *ptr);
	int startBlinkThread();

	bool isDirty();
	bool isDirty(int nDirtyBits);
	void setDirty(int nDirtyBits);
	void clearDirty(int nDirtyBits);

	virtual int initCustom() = 0;
	virtual void handleKeyboardEvent(SDL_Event &event);
	virtual void handleMouseEvent(SDL_Event &event);

private:
	bool m_bRunning;

	int m_nWidth;
	int m_nHeight;

	int m_nDirtyBits;

	unsigned int m_fontSize;

private:
	// pulled from SDL_keyboard.c / lgpl Copyright (C) 1997-2006 Sam Lantinga
	struct {
		int firsttime;    /* if we check against the delay or repeat value */
		int delay;        /* the delay before we start repeating */
		int interval;     /* the delay between key repeat events */
		Uint32 timestamp; /* the time the first keydown event occurred */
		SDL_Event evt;    /* the event we are supposed to repeat */
	} m_keyRepeat;


	int init();
	int initOpenGL();
	void shutdown();
	void eventLoop();

	void pushColors();
	void pushFontStyles();

	void checkKeyRepeat();

public:
	SDLCore();
	virtual ~SDLCore();

	void start();
	void run();
	bool isRunning();

	unsigned int getFontSize();
	void setFontSize(unsigned int nSize);

	void printCharacter(int nColumn, int nLine, TSCell cCell);

	void clearScreen(TSColor color);

	int getMaximumLinesOfText();
	int getMaximumColumnsOfText();

	virtual SDL_Color getColor(TSColor color) = 0;
	virtual void redraw() = 0;
	virtual void redrawBlinked() = 0;

	virtual void updateDisplaySize() = 0;

	void stopKeyRepeat();
	void fakeKeyEvent(SDL_Event &event);
	void setActive(int active);
};

#endif
