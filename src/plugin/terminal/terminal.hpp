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

#ifndef TERMINAL_HPP__
#define TERMINAL_HPP__

#include <pthread.h>
#include <termios.h>

#ifndef TIOCGWINSZ
#include <sys/ioctl.h>
#endif

#include "extterminal.hpp"

#include <string>
#include <vector>

class Terminal : public ExtTerminal, public ExtTerminalContainer
{
private:
	bool m_bDone;
	int m_nWritePriority;
	pid_t m_pid;
	char *m_slaveName;
	char *m_sUser;

	std::vector<std::string> m_exec;

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
	void newLogin();

	void spawn();

	int runReader();
	int startReaderThread();
	static void *readerThread(void *terminal);

	int sendCommand(const char *command);
	void flushOutputBuffer();

public:
	Terminal();
	virtual ~Terminal();

	int m_masterFD;
	int m_slaveFD;

	char *path;

	void setWindowSize(int nWidth, int nHeight);
	int start();

	void insertData(const char *data, int len);
	void insertData(const char *data);

	void setExec(const char *exec);
};

#endif
