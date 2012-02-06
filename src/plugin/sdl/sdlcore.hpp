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
#include <PDL.h>

#include "terminal/vtterminalstate.hpp"
#include "sdlfontgl.hpp"

#include <time.h>


namespace SDL {

class SDLCore;

class TimerCollection;
class IOCollection;
class AsyncQueue;
class ListenThread;

class Abstract_Timer {
public:
	Abstract_Timer(SDLCore* core = 0);
	virtual ~Abstract_Timer();
	void setCore(SDLCore *core);
	SDLCore* core();

	virtual void run() = 0;

	void start(unsigned int msec); // repeating timer
	void start(timespec nextEvent); // one shot
	void stop();
	bool running();

private:
	friend class TimerCollection;
	SDLCore *m_core;
	bool m_running;
	unsigned int m_interval_msec;
	timespec m_nextEvent;
};

class Abstract_IO {
public:
	Abstract_IO(SDLCore* core = 0);
	virtual ~Abstract_IO();
	void setCore(SDLCore *core);
	SDLCore* core();

	virtual void read_ready();
	virtual void write_ready();

	void setFD(int fd);
	int fd();

	void waitRead();
	void stopRead();
	void waitWrite();
	void stopWrite();

	bool reading();
	bool writing();

private:
	friend class IOCollection;
	SDLCore *m_core;
	int m_fd;
	bool m_read, m_write, m_registered;
	unsigned int m_colNdx; // index in the collection vector
};

/* careful: AsyncJob instances get deleted after they were executed
 *   (in the sdl thread context)
 * so don't put such instances on your stack, and don't delete (or access)
 * them after you sent them!
 */
class Abstract_AsyncJob {
public:
	Abstract_AsyncJob();
	virtual ~Abstract_AsyncJob();

	virtual void run() = 0;
	void send(SDLCore *core);
};

class KeyRepeatTimer;
class RefreshDelayTimer;

/**
 * Initializer and basic 2D function for webOS SDL.
 */
class SDLCore
{
private:
	friend class Abstract_Timer; // access m_timers
	friend class Abstract_IO; // access m_iocolllection
	friend class Abstract_AsyncJob; // access m_asyncqueue

	TimerCollection *m_timers;
	IOCollection *m_iocollection;
	AsyncQueue *m_asyncqueue;
	ListenThread *m_listenthread;
	bool m_listenNotified;

	class BlinkTimer : public Abstract_Timer {
	public:
		BlinkTimer(SDLCore* core = 0);
		virtual void run();
	};

	BlinkTimer *m_blinkTimer;
	RefreshDelayTimer *m_refreshDelayTimer;

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
	int init();
	int initOpenGL();
	void shutdown();

	void waitForEvent(SDL_Event &event);
	void handleEvent(SDL_Event &event);
	void eventLoop();

	void pushColors();
	void pushFontStyles();

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

	void setActive(int active);
};

} // end namespace SDL

#endif
