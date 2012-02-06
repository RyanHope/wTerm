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

#include "sdlcore_p.hpp"

/* This is the async event handling part */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <algorithm>

namespace SDL {

bool TimerCollection::orderTimers(Abstract_Timer *a, Abstract_Timer *b) {
	return (a->m_nextEvent > b->m_nextEvent);
}

void TimerCollection::getNow(timespec &t) {
	if (0 != clock_gettime(CLOCK_MONOTONIC, &t)) {
		syslog(LOG_ERR, "clock_gettime failed: %s", strerror(errno));
		abort();
	}
}

void TimerCollection::addMsec(timespec &t, int msec) {
	int sec = msec / 1000;
	msec = msec - (1000 * sec);
	t.tv_sec += sec;
	t.tv_nsec += msec * 1000000;
	while (t.tv_nsec < 0) {
		--t.tv_sec;
		t.tv_nsec += 1000000000;
	}
	while (t.tv_nsec >= 1000000000) {
		++t.tv_sec;
		t.tv_nsec -= 1000000000;
	}
}


TimerCollection::TimerCollection() : m_changed(false) {
}
TimerCollection::~TimerCollection() {
	while (!m_heap.empty()) remove(m_heap.back());
}

bool TimerCollection::hasEvents() {
	return !m_heap.empty();
}

timespec TimerCollection::nextEvent() {
	if (!m_heap.empty()) return m_heap.front()->m_nextEvent;
	timespec t = {0,0};
	return t;
}

void TimerCollection::run() {
	timespec t;
	getNow(t);
	while (!m_heap.empty() && m_heap.front()->m_nextEvent <= t) {
		std::pop_heap(m_heap.begin(), m_heap.end(), orderTimers);
		Abstract_Timer *timer = m_heap.back();
		if (0 != timer->m_interval_msec) {
			// restart timer
			timer->m_nextEvent = t;
			addMsec(timer->m_nextEvent, timer->m_interval_msec);
			std::push_heap(m_heap.begin(), m_heap.end(), orderTimers);
		} else {
			// one shot timer, disable
			timer->m_running = false;
			m_heap.pop_back();
		}
		m_changed = true;

		timer->run();
	}
}

void TimerCollection::remove(Abstract_Timer *t) {
	if (!t->m_running) return;
	t->m_running = false;

	if (m_heap.empty()) {
		syslog(LOG_ERR, "cannot remove timer from empty collection");
		abort();
	}

	m_changed = true;

	if (m_heap.back() == t) {
		// fast path for calling stop() in run()
		m_heap.pop_back(); // popping from the back keep heap valid
		return;
	}
	Heap::iterator it = std::find(m_heap.begin(), m_heap.end(), t);
	if (it == m_heap.end()) {
		syslog(LOG_ERR, "remove: timer not in collection");
		abort();
	}
	m_heap.erase(it);
	std::make_heap(m_heap.begin(), m_heap.end(), orderTimers);
}

void TimerCollection::insert(Abstract_Timer *t) {
	if (t->m_running) return;
	t->m_running = true;

	m_changed = true;

	m_heap.push_back(t);
	std::push_heap(m_heap.begin(), m_heap.end(), orderTimers);
}

Abstract_Timer::Abstract_Timer(SDLCore* core) : m_core(core), m_running(false) {
}
Abstract_Timer::~Abstract_Timer() {
	stop();
}
void Abstract_Timer::setCore(SDLCore *core) {
	if (core == m_core) return;
	if (m_running) {
		stop();
		m_core = core;
		if (m_core) m_core->m_timers->insert(this);
	} else {
		m_core = core;
	}
}
SDLCore* Abstract_Timer::core() {
	return m_core;
}
void Abstract_Timer::start(unsigned int msec) {
	if (0 == msec || !m_core) return;
	if (m_running) stop(); // restart timer

	m_interval_msec = msec;

	TimerCollection::getNow(m_nextEvent);
	TimerCollection::addMsec(m_nextEvent, msec);

	m_core->m_timers->insert(this);
}
void Abstract_Timer::start(timespec nextEvent) {
	if (!m_core) return;
	if (m_running) stop(); // restart timer

	m_interval_msec = 0;
	m_nextEvent = nextEvent;

	m_core->m_timers->insert(this);
}
void Abstract_Timer::stop() {
	if (!m_core || !m_running) return;
	m_core->m_timers->remove(this);
}
bool Abstract_Timer::running() {
	return m_running;
}

IOCollection::IOCollection() : m_changed(false), m_changedComp(false) {
}

IOCollection::~IOCollection() {
	while (!m_list.empty()) remove(m_list.back());
}

void IOCollection::run() {
	int readyCount = poll(&m_pollfds[0], m_pollfds.size(), 0);
	if (readyCount <= 0) return;

restart: // restart if the arrays are resized (content may move)
	for (unsigned int i = 0, e = m_pollfds.size(); i < e; i++) {
		m_changedComp = false;
		short &revents(m_pollfds[i].revents);
		if (revents) {
			revents &= (POLLIN | POLLOUT);
			Abstract_IO *t = m_list[i];
			if (revents & POLLIN) {
				revents &= ~POLLIN; // mark as done
				if (t->m_read) t->read_ready();
				if (m_changedComp) goto restart;
			}
			if (revents & POLLOUT) {
				revents &= ~POLLOUT; // mark as done
				if (t->m_read) t->write_ready();
				if (m_changedComp) goto restart;
			}
		}
	}
}

void IOCollection::remove(Abstract_IO *t) {
	if (!t->m_registered) return;

	m_changedComp = true;
	m_changed = true;

	unsigned int lastNdx = m_list.size() - 1;

	m_list[t->m_colNdx] = m_list[lastNdx];
	m_pollfds[t->m_colNdx] = m_pollfds[lastNdx];

	m_list.pop_back();
	m_pollfds.pop_back();

	t->m_registered = false;
	t->m_colNdx = 0;
}

void IOCollection::insert(Abstract_IO *t) {
	if (t->m_registered) return;

	m_changedComp = true;
	m_changed = true;

	t->m_colNdx = m_list.size();
	m_list.push_back(t);

	pollfd p = {
		t->m_fd,
		(t->m_read ? POLLIN : 0) | (t->m_write ? POLLOUT : 0),
		0
	};
	m_pollfds.push_back(p);

	t->m_registered = true;
}

void IOCollection::update(Abstract_IO *t) {
	if (t->m_registered) {
		if (-1 == t->m_fd || (!t->m_read && !t->m_write)) {
			remove(t);
			return;
		}
	} else {
		if (-1 != t->m_fd && (t->m_read || t->m_write)) {
			insert(t);
		}
		return;
	}

	pollfd &p(m_pollfds[t->m_colNdx]);

	pollfd old = p;

	p.fd = t->m_fd;
	p.events = (t->m_read ? POLLIN : 0) | (t->m_write ? POLLOUT : 0);
	p.revents = 0;

	if (p.fd != old.fd || p.events != old.events) m_changed = true;
}

Abstract_IO::Abstract_IO(SDLCore* core)
: m_core(core), m_fd(-1), m_read(false), m_write(false), m_registered(false), m_colNdx(0) {
}
Abstract_IO::~Abstract_IO() {
	if (m_registered) m_core->m_iocollection->remove(this);
	m_fd = -1;
	m_write = m_read = false;
}
void Abstract_IO::setCore(SDLCore *core) {
	if (core == m_core) return;

	if (m_registered) m_core->m_iocollection->remove(this);

	m_core = core;

	if (m_core) m_core->m_iocollection->update(this);
}
SDLCore* Abstract_IO::core() {
	return m_core;
}

void Abstract_IO::read_ready() {
	stopRead();
}
void Abstract_IO::write_ready() {
	stopWrite();
}

void Abstract_IO::setFD(int fd) {
	m_fd = fd;
	if (m_core) m_core->m_iocollection->update(this);
}
int Abstract_IO::fd() {
	return m_fd;
}

void Abstract_IO::waitRead() {
	m_read = true;
	if (m_core && m_fd != -1) m_core->m_iocollection->update(this);
}
void Abstract_IO::stopRead() {
	m_read = false;
	if (m_core && m_fd != -1) m_core->m_iocollection->update(this);
}
void Abstract_IO::waitWrite() {
	m_write = true;
	if (m_core && m_fd != -1) m_core->m_iocollection->update(this);
}
void Abstract_IO::stopWrite() {
	m_write = false;
	if (m_core && m_fd != -1) m_core->m_iocollection->update(this);
}

bool Abstract_IO::reading() {
	return m_read;
}
bool Abstract_IO::writing() {
	return m_write;
}


AsyncQueue::AsyncQueue(SDL_Event event)
: m_eventPending(false), m_event(event) {
	pthread_mutex_init(&m_lock, NULL);
	m_mainThread = pthread_self();
}
AsyncQueue::~AsyncQueue() {
	runQueue();
	pthread_mutex_destroy(&m_lock);
}

void AsyncQueue::sendJob(Abstract_AsyncJob *job) {
	bool pending;

	pthread_mutex_lock(&m_lock);

	pending = m_eventPending;
	m_eventPending = true;
	m_jobs.push_back(job);

	pthread_mutex_unlock(&m_lock);

	if (!pending) SDL_PushEvent(&m_event);
}

void AsyncQueue::runQueue() {
	List jobs;

	pthread_mutex_lock(&m_lock);

	m_eventPending = false;
	std::swap(jobs, m_jobs);

	pthread_mutex_unlock(&m_lock);

	for (List::iterator i = jobs.begin(), e = jobs.end(); i != e; ++i) {
		(*i)->run();
		delete *i;
	}
}

Abstract_AsyncJob::Abstract_AsyncJob() {
}
Abstract_AsyncJob::~Abstract_AsyncJob() {
}

void Abstract_AsyncJob::send(SDLCore *core) {
	core->m_asyncqueue->sendJob(this);
}


static void nonBlock(int fd) {
	int val = fcntl(fd, F_GETFL, 0);

	if (val < 0) {
		syslog(LOG_ERR, "Cannot get flags from file descriptor.");
		abort();
	}

	val |= O_NONBLOCK;

	if (fcntl(fd, F_SETFL, val) < 0) {
		syslog(LOG_ERR, "Cannot set flags on file descriptor.");
		abort();
	}
}

ListenThread::ListenThread(SDL_Event event) : m_event(event), m_thread(0), m_hasTimeout(false), m_stop(false), m_waiting(false) {
	pthread_mutex_init(&m_lock, NULL);
	pthread_cond_init(&m_cond, NULL);

	pipe(m_threadPipe);
	nonBlock(m_threadPipe[0]);

	pollfd poll_pipe = { m_threadPipe[0], POLLIN, 0 };
	m_pollfds.push_back(poll_pipe);

	m_timeout.tv_sec = 0;
	m_timeout.tv_nsec = 0;
}

ListenThread::~ListenThread() {
	stop();

	close(m_threadPipe[0]);
	close(m_threadPipe[1]);

	pthread_cond_destroy(&m_cond);
	pthread_mutex_destroy(&m_lock);
}

void ListenThread::setEvent(SDL_Event event) {
	pthread_mutex_lock(&m_lock);

	m_event = event;

	pthread_mutex_unlock(&m_lock);
}

const SDL_Event& ListenThread::event() {
	return m_event;
}

void* listen_thread(void* ptr) {
	syslog(LOG_DEBUG, "running SDL::ListenThread");
	static_cast<ListenThread*>(ptr)->run_thread();
	syslog(LOG_DEBUG, "finished SDL::ListenThread");
	return NULL;
}

void ListenThread::run() {
	if (m_thread) return;

	m_stop = false;
	pthread_create(&m_thread, NULL, listen_thread, this);
}

void ListenThread::stop() {
	if (!m_thread) return;

	pthread_mutex_lock(&m_lock);
	m_stop = true;
	m_waiting = false;

	wakeup();
	pthread_mutex_unlock(&m_lock);

	pthread_join(m_thread, NULL);
	m_thread = 0;
}

void ListenThread::waitFor(bool hasTimeout, timespec timeout, const std::vector<pollfd> &pollfds) {
	pthread_mutex_lock(&m_lock);

	m_hasTimeout = hasTimeout;
	m_timeout = timeout;
	m_pollfds = pollfds;

	pollfd poll_pipe = { m_threadPipe[0], POLLIN, 0 };
	m_pollfds.push_back(poll_pipe);

	m_waiting = true;

	wakeup();
	pthread_mutex_unlock(&m_lock);
}

void ListenThread::wakeup() {
	static const char buf[1] = { ' ' };
	write(m_threadPipe[1], buf, 1);
	pthread_cond_signal(&m_cond);
}

void ListenThread::run_thread() {
	bool noevent = true;
	static char buf[64];

	for ( ;; ) {
		pthread_mutex_lock(&m_lock);

		if (!noevent) {
			m_waiting = false;
			SDL_PushEvent(&m_event);
		}

		while (!m_waiting) {
			if (m_stop) {
				pthread_mutex_unlock(&m_lock);
				return;
			}
			pthread_cond_wait(&m_cond, &m_lock);
		}

		if (m_stop) {
			pthread_mutex_unlock(&m_lock);
			return;
		}

		std::vector<pollfd> pollfds(m_pollfds);
		bool hasTimeout(m_hasTimeout);
		timespec timeout(m_timeout);

		pthread_mutex_unlock(&m_lock);

		int nTimeout = -1;
		if (hasTimeout) {
			timespec now;
			TimerCollection::getNow(now);
			nTimeout = (timeout.tv_sec - now.tv_sec)*1000 + (timeout.tv_nsec - now.tv_nsec)/1000000;
			if (nTimeout < 0) nTimeout = 0;
		}
//		syslog(LOG_DEBUG, "polling: %i fds, timeout %i", pollfds.size(), nTimeout);

		int n = poll(&pollfds[0], pollfds.size(), nTimeout);
		int threadPipe_revents = pollfds[pollfds.size()-1].revents;
		noevent = (1 == n && (threadPipe_revents & POLLIN));

		// clear threadPipe from wakeup signals
		if (threadPipe_revents & POLLIN) {
			while (0 < read(m_threadPipe[0], buf, sizeof(buf)) || EINTR == errno) ;
		}
	}
}

} // end namspace SDL
