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

#ifndef SDLCORE_P_HPP__
#define SDLCORE_P_HPP__

#include "sdlcore.hpp"

#include <vector>
#include <deque>

#include <poll.h>
#include <pthread.h>
#include <time.h>


inline bool operator<(const timespec &a, const timespec &b) {
	return (a.tv_sec < b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_nsec < b.tv_nsec));
}
inline bool operator<=(const timespec &a, const timespec &b) {
	return (a.tv_sec < b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_nsec <= b.tv_nsec));
}
inline bool operator>(const timespec &a, const timespec &b) {
	return (b < a);
}
inline bool operator>=(const timespec &a, const timespec &b) {
	return (b >= a);
}

namespace SDL {

class TimerCollection {
public:
	TimerCollection();
	~TimerCollection();

	const static int MAX_WAIT_SEC = 600;

	typedef std::vector<Abstract_Timer*> Heap;
	Heap m_heap;
	bool m_changed;

	bool hasEvents();
	timespec nextEvent();

	static bool orderTimers(Abstract_Timer *a, Abstract_Timer *b);
	static void getNow(timespec &t);
	static void addMsec(timespec &t, int msec);
	static int diffMsec(const timespec &a, const timespec &b); // "a - b"

	void run();

	void remove(Abstract_Timer *t);
	void insert(Abstract_Timer *t);
};

class IOCollection {
public:
	IOCollection();
	~IOCollection();

	typedef std::vector<Abstract_IO*> List;
	List m_list;

	std::vector<pollfd> m_pollfds;

	bool m_changed, m_changedComp;

	void run();

	void remove(Abstract_IO *t);
	void insert(Abstract_IO *t);

	void update(Abstract_IO *t);
};

class AsyncQueue {
public:
	AsyncQueue(SDL_Event event);
	~AsyncQueue();

	void sendJob(Abstract_AsyncJob *job);
	void runQueue();

	pthread_mutex_t m_lock;
	pthread_t m_mainThread;

	typedef std::deque<Abstract_AsyncJob*> List;

	List m_jobs;
	bool m_eventPending;
	SDL_Event m_event;
};

class ListenThread {
public:
	ListenThread(SDL_Event event);
	~ListenThread();

	// event that thread sends to SDL to signal activity
	void setEvent(SDL_Event event);
	const SDL_Event& event();

	void run();
	void stop();

	// after each "wakeup" the thread needs an update
	void waitFor(bool hasTimeout, timespec timeout, const std::vector<pollfd> &pollfds);

private:
	friend void* listen_thread(void* ptr);

	void wakeup();
	void run_thread();

	SDL_Event m_event;

	pthread_mutex_t m_lock;
	pthread_cond_t m_cond;

	pthread_t m_thread;

	int m_threadPipe[2]; // wakeup poll in thread

	std::vector<pollfd> m_pollfds;
	bool m_hasTimeout;
	timespec m_timeout;
	bool m_stop, m_waiting;
};

} // end namespace SDL

#endif
