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

#ifndef _SDL_SDLGENERIC_HPP_
#define _SDL_SDLGENERIC_HPP_

#include "sdlcore.hpp"

namespace SDL {

	class DelayedRepeatTimer : protected Abstract_Timer {
	public:
		DelayedRepeatTimer(SDLCore* core = 0);

		void start();
		void stop();

		void setDelay(unsigned int delay_msec);
		void setRepeat(unsigned int repeat_msec);

		virtual void triggered() = 0;

	private:
		virtual void run();

	private:
		unsigned int m_delay_msec;
		unsigned int m_repeat_msec;
	};


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


} // end of namespace SDL

#endif
