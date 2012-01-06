/**
 * This file is part of SDLTerminal.
 * Copyright (C) 2011 Vincent Ho <www.whimsicalvee.com>
 * Copyright (C) 2011-2012 Ryan Hope <rmh3093@gmail.com>
 *
 * SDLTerminal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SDLTerminal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with SDLTerminal.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TERMINAL_HPP__
#define TERMINAL_HPP__

#include <pthread.h>
#include <termios.h>

#ifndef TIOCGWINSZ
#include <sys/ioctl.h>
#endif

#include "extterminal.hpp"
#include "util/databuffer.hpp"

class Terminal : public ExtTerminal, public ExtTerminalContainer
{
private:
	int m_masterFD;
	int m_slaveFD;
	bool m_bDone;
	int m_nWritePriority;
	pid_t m_pid;
	char *m_slaveName;
	char *m_sUser;
	DataBuffer *m_dataBuffer;

	pthread_t m_readerThread;
	struct winsize m_winSize;

	int openPTYMaster();
	int openPTYSlave();
	int forkPTY();
	int setRaw();
	int setCBreak();
	int setTermMode();
	int setWindowSize();
	int setFlag(int fileDesc, int flag);
	bool isChild();

	int runReader();
	int startReaderThread();
	static void *readerThread(void *terminal);

	int sendCommand(const char *command);
	void flushOutputBuffer();

public:
	Terminal();
	virtual ~Terminal();

	void setWindowSize(int nWidth, int nHeight);
	int start();

	void insertData(const char *data, size_t size);

	const char *getUser();
	void setUser(const char *sUser);
};

#endif
