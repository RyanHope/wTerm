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

#include <syslog.h>

#include <PDL.h>

#include "webos.hpp"
#include "sdlgeneric.hpp"

#define HP_SYM         17
#define HP_ORANGE      129

namespace Webos {

	static PDL_bool playFeedbackCallback(PDL_ServiceParameters *params, void *context) {
		FakeKeyboardEvents *keyRepeatTimer = (FakeKeyboardEvents *)context;
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

	FakeKeyboardEvents::FakeKeyboardEvents(SDL::SDLCore* core)
	: DelayedRepeatTimer(core), m_playFeedback(false) {
		if (PDL_IsPlugin()) {
			PDL_Err err = PDL_ServiceCallWithCallback("luna://com.palm.systemservice/getPreferences","{\"keys\":[\"x_palm_virtualkeyboard_prefs\"],\"subscribe\":true}", playFeedbackCallback, this, PDL_FALSE);
			if (err != PDL_NOERROR) {
				syslog(LOG_ERR, "Failed to register playFeedbackCallback: %s", PDL_GetError());
			}
		}
	}

	void FakeKeyboardEvents::send(const SDL_Event &event, bool sound) {
		m_event = event;
		m_eventSound = sound;
		playSound();
		SDL_PushEvent(&m_event);
		if (m_event.type == SDL_KEYUP) {
			stop();
		} else {
			switch (event.key.keysym.sym) {
			case SDLK_NUMLOCK:
			case SDLK_CAPSLOCK:
			case HP_SYM:
			case SDLK_LCTRL:
			case SDLK_RCTRL:
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
			case SDLK_WORLD_30:
			case SDLK_LALT:
			case SDLK_RALT:
			case HP_ORANGE:
			case SDLK_LMETA:
			case SDLK_RMETA:
			case SDLK_MODE:
				stop();
				break;
			default:
				start();
				break;
			}
		}
	}

	void FakeKeyboardEvents::setPlayFeedback(bool playFeedback) {
		m_playFeedback = playFeedback;
	}

	bool FakeKeyboardEvents::getPlayFeedback() {
		return m_playFeedback;
	}

	void FakeKeyboardEvents::triggered() {
		playSound();
		SDL_PushEvent(&m_event);
	}

	void FakeKeyboardEvents::playSound() {
		if (m_eventSound && m_playFeedback) {
			PDL_ServiceCall("luna://com.palm.audio/systemsounds/playFeedback", "{\"name\":\"key\"}");
		}
	}



	Adapter::Adapter(WTerm *wterm) : m_wterm(wterm), m_fakeKeyEvents(wterm) {
		m_fakeKeyEvents.setDelay(500);
		m_fakeKeyEvents.setRepeat(35);
	}

	class WA_Job_SetFontSize : public SDL::Abstract_AsyncJob {
	public:
		WA_Job_SetFontSize(WTerm *wterm, unsigned int ptsize)
		: wterm(wterm), ptsize(ptsize) { }
		void run() {
			wterm->setFontSize(ptsize);
		}
		WTerm *wterm;
		unsigned int ptsize;
	};
	void Adapter::setFontSize(unsigned int ptsize) {
		WA_Job_SetFontSize *job = new WA_Job_SetFontSize(m_wterm, ptsize);
		job->send(m_wterm);
	}

	class WA_Job_FakeKeyEvent : public SDL::Abstract_AsyncJob {
	public:
		WA_Job_FakeKeyEvent(FakeKeyboardEvents *f, SDL_Event &event, bool sound)
		: f(f), m_event(event), sound(sound) { }
		void run() {
			f->send(m_event, sound);
		}
		FakeKeyboardEvents *f;
		SDL_Event m_event;
		bool sound;
	};
	void Adapter::fakeKeyEvent(SDL_Event &event, bool sound) {
		WA_Job_FakeKeyEvent *job = new WA_Job_FakeKeyEvent(&m_fakeKeyEvents, event, sound);
		job->send(m_wterm);
	}

	void Adapter::stopKeyRepeat() {
		SDL_Event e;
		e.type = SDL_KEYUP;
		e.key.state = SDL_RELEASED;
		e.key.keysym.mod = (SDLMod)0;
		e.key.keysym.unicode = (SDLKey)0;
		e.key.keysym.scancode = 0;
		fakeKeyEvent(e, false);
	}

} // end of namespace Webos
