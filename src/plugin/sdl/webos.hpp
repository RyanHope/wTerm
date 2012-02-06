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

#ifndef _SDL_WEBOS_HPP_
#define _SDL_WEBOS_HPP_

#include "sdlcore.hpp"
#include "sdlgeneric.hpp"
#include "wterm.hpp"

namespace Webos {

	class FakeKeyboardEvents : public SDL::DelayedRepeatTimer {
	public:
		FakeKeyboardEvents(SDL::SDLCore* core = 0);

		void send(const SDL_Event &event, bool sound);

		void setPlayFeedback(bool playFeedback);
		bool getPlayFeedback();

		virtual void triggered();

		void playSound();

	private:
		SDL_Event m_event;
		bool m_eventSound;
		bool m_playFeedback;
	};

	class Adapter {
	public:
		Adapter(WTerm *wterm);

		void setFontSize(unsigned int ptsize);
		void fakeKeyEvent(SDL_Event &event, bool sound);
		void stopKeyRepeat();

	private:
		WTerm *m_wterm;
		FakeKeyboardEvents m_fakeKeyEvents;
	};

} // end of namespace Webos

#endif
