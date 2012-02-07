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

#include "sdlgeneric.hpp"
#include "sdlcore_p.hpp"

#include <syslog.h>

#include <PDL.h>

namespace SDL {

	KeyRepeatTimer::KeyRepeatTimer(SDLCore* core)
	: Abstract_Timer(core), m_delay_msec(0), m_repeat_msec(0), m_playFeedback(true) {
	}
	void KeyRepeatTimer::start(const SDL_Event &event) {
		m_event = event;
		Abstract_Timer::start(m_delay_msec);
	}
	void KeyRepeatTimer::stop() {
		Abstract_Timer::stop();
	}
	void KeyRepeatTimer::setDelay(unsigned int delay_msec) {
		m_delay_msec = delay_msec;
	}
	void KeyRepeatTimer::setRepeat(unsigned int repeat_msec) {
		m_repeat_msec = repeat_msec;
	}
	void KeyRepeatTimer::setPlayFeedback(bool playFeedback) {
		m_playFeedback = playFeedback;
	}
	bool KeyRepeatTimer::getPlayFeedback() {
		return m_playFeedback;
	}
	void KeyRepeatTimer::run() {
		// set next interval
		Abstract_Timer::start(m_repeat_msec);
		if (PDL_IsPlugin() && m_event.key.keysym.scancode && getPlayFeedback()) {
			PDL_ServiceCall("luna://com.palm.audio/systemsounds/playFeedback", "{\"name\":\"key\"}");
		}
		SDL_PushEvent(&m_event);
	}
	PDL_bool KeyRepeatTimer::playFeedbackCallback(PDL_ServiceParameters *params, void *context)
	{
		KeyRepeatTimer *keyRepeatTimer = (KeyRepeatTimer *)context;
		if (keyRepeatTimer == NULL)
		{
			syslog(LOG_DEBUG, "playFeedbackCallback context null");
			return PDL_TRUE;
		}

		if (PDL_ParamExists(params, "x_palm_virtualkeyboard_prefs"))
		{
			char result [256];
			char *search = NULL;
			PDL_GetParamString(params, "x_palm_virtualkeyboard_prefs", result, 256);

			search = strstr(result, "TapSounds\":");
			if (search == NULL)
				return PDL_TRUE;
			search += strlen("TapSounds\":");

			keyRepeatTimer->setPlayFeedback((*search == 't')); // ? true : false
		}
		else
		{
			syslog(LOG_DEBUG, "no x_palm_virtualkeyboard_prefs in response");
		}
		return PDL_TRUE;
	}


	RefreshDelayTimer::RefreshDelayTimer(SDLCore* core)
	: Abstract_Timer(core), m_delay_msec(0) {
		m_last_refresh.tv_sec = 0;
		m_last_refresh.tv_nsec = 0;
	}
	void RefreshDelayTimer::setDelay(unsigned int delay_msec) {
		m_delay_msec = delay_msec;
		if (running()) {
			timespec now, next;
			TimerCollection::getNow(now);
			next = m_last_refresh;
			TimerCollection::addMsec(next, m_delay_msec);
			if (next > now) {
				Abstract_Timer::start(next);
			} else {
				// done waiting
				Abstract_Timer::stop();
			}
		}
	}
	bool RefreshDelayTimer::update() {
		if (running()) return false; // not triggered yet

		timespec now, next;
		TimerCollection::getNow(now);
		next = m_last_refresh;
		TimerCollection::addMsec(next, m_delay_msec);
		if (next > now) {
			// delay didn't pass yet, start timer
			Abstract_Timer::start(next);
			return false;
		} else {
			// delay passed, allow update
			return true;
		}
	}
	void RefreshDelayTimer::run() {
		// loop got activated, job done
		Abstract_Timer::stop();
	}

} // end namespace SDL
