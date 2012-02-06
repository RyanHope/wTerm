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

class SDLCore_TimerCollection;
class SDLCore_IOCollection;
class SDLCore_ListenThread;

/**
 * Initializer and basic 2D function for webOS SDL.
 */
class SDLCore
{
public:
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
		friend class SDLCore_TimerCollection;
		SDLCore *m_core;
		bool m_running;
		unsigned int m_interval_msec;
		timespec m_nextEvent;
	};
	friend class Abstract_Timer;

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
		friend class SDLCore_IOCollection;
		SDLCore *m_core;
		int m_fd;
		bool m_read, m_write, m_registered;
		unsigned int m_colNdx;
	};
	friend class Abstract_IO;

private:
	SDLCore_TimerCollection *m_timers;
	SDLCore_IOCollection *m_iocollection;
	SDLCore_ListenThread *m_listenthread;
	bool m_listenNotified;

	class KeyRepeatTimer : protected Abstract_Timer {
	public:
		KeyRepeatTimer(SDLCore* core = 0);

		void start(const SDL_Event &event);
		void stop();

		void setDelay(unsigned int delay_msec);
		void setRepeat(unsigned int repeat_msec);
		void setPlayFeedback(bool playFeedback);
		bool getPlayFeedback();

		static PDL_bool playFeedbackCallback(PDL_ServiceParameters *params, void *context);

	protected:
		virtual void run();

	private:
		SDL_Event m_event;
		unsigned int m_delay_msec;
		unsigned int m_repeat_msec;
		bool m_playFeedback;
	};
	KeyRepeatTimer m_keyRepeatTimer; // only for faked keys

	class BlinkTimer : public Abstract_Timer {
	public:
		BlinkTimer(SDLCore* core = 0);
		virtual void run();
	};
	BlinkTimer m_blinkTimer;

	class RefreshDelayTimer : protected Abstract_Timer {
	public:
		RefreshDelayTimer(SDLCore* core = 0);

		/* if new delay makes timer "expired" it won't wakeup the loop
		 * again. so make sure update gets called later in the same loop
		 */
		void setDelay(unsigned int delay_msec);

		/* call when you want to update. return true if no delay
		 * is needed, false if refresh should delayed. on false a
		 * timer event is started to wakeup the loop after the delay passed
		 *
		 * call update in each loop to check whether it triggered
		 */
		bool update();

	protected:
		virtual void run();

	private:
		unsigned int m_delay_msec;
		timespec m_last_refresh;
	};
	RefreshDelayTimer m_refreshDelayTimer;

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

	void stopKeyRepeat();
	void fakeKeyEvent(SDL_Event &event);
	void setActive(int active);
};

#endif
