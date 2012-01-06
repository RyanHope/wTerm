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

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stropts.h>
#include <termios.h>
#include <unistd.h>
#include <syslog.h>

#include "terminal.hpp"

struct termios terminal_global_state;
int terminal_global_slave_fd = -1;

void terminal_restore_settings()
{
	if (terminal_global_slave_fd >= 0)
	{
		if (tcsetattr(terminal_global_slave_fd, TCSAFLUSH, &terminal_global_state) < 0)
		{
			syslog(LOG_ERR, "Cannot restore initial terminal settings.");
		}
	}
}

void terminal_save_settings(int slaveFD)
{
	terminal_global_slave_fd = slaveFD;

	if (terminal_global_slave_fd >= 0)
	{
		if (tcgetattr(terminal_global_slave_fd, &terminal_global_state) < 0)
		{
			terminal_global_slave_fd = -1;
			syslog(LOG_ERR, "Cannot save initial terminal settings.");
		}
	}
}

Terminal::Terminal()
{
	m_masterFD = -1;
	m_slaveFD = -1;
	m_bDone = false;
	m_dataBuffer = new DataBuffer();
	m_nWritePriority = 0;
	m_sUser = NULL;

	setUser("root");
	memset(&m_winSize, 0, sizeof(m_winSize));
}

Terminal::~Terminal()
{
	if (isReady())
	{
		m_bDone = true;
		pthread_join(m_readerThread, NULL);
	}

	terminal_restore_settings();
	terminal_global_slave_fd = -1;

	if (m_masterFD >= 0)
	{
		close(m_masterFD);
	}

	if (m_slaveFD >= 0)
	{
		close(m_slaveFD);
	}

	free(m_sUser);
	delete m_dataBuffer;
}

int Terminal::openPTYMaster()
{
	syslog(LOG_INFO, "Initializing master.");

	m_masterFD = posix_openpt(O_RDWR | O_NOCTTY);

	if (m_masterFD < 0)
	{
		syslog(LOG_ERR, "Cannot get master pseudo-terminal.");
		return -1;
	}

	if (grantpt(m_masterFD) < 0 || unlockpt(m_masterFD) < 0)
	{
		syslog(LOG_ERR, "Cannot grant access permission for master pseudo-terminal.");
		return -2;
	}

	m_slaveName = ptsname(m_masterFD);

	if (m_slaveName == NULL)
	{
		syslog(LOG_ERR, "Cannot get name of slave pseudo-terminal.");
		return -3;
	}

	if (setFlag(m_masterFD, O_NONBLOCK) != 0)
	{
		syslog(LOG_ERR, "Couldn't set FD to non-blocking.");
		return -4;
	}

	syslog(LOG_INFO, "Initialized master with FD %d.", m_masterFD);

	return 0;
}

int Terminal::openPTYSlave()
{
	syslog(LOG_INFO, "Initializing slave with name '%s'.", m_slaveName);

	m_slaveFD = open(m_slaveName, O_RDWR);

	if (m_slaveFD == -1)
	{
		syslog(LOG_ERR, "Cannot open slave pseudo-terminal.");
		return -1;
	}

	if (ioctl(m_slaveFD, I_FIND, "ptem") == 0)
	{
		if (ioctl(m_slaveFD, I_PUSH, "ptem") < 0 || ioctl(m_slaveFD, I_PUSH, "ldterm") < 0 || ioctl(m_slaveFD, I_PUSH, "ttcompat") < 0)
		{
			syslog(LOG_ERR, "Cannot push modules for slave pseudo-terminal.");
			return -2;
		}
	}

	syslog(LOG_INFO, "Initialized slave with name '%s' with FD %d.", m_slaveName, m_slaveFD);

	return 0;
}

int Terminal::forkPTY()
{
	if (openPTYMaster() != 0)
	{
		syslog(LOG_ERR, "Cannot open master PTY.");
		return -1;
	}

	if (openPTYSlave() != 0)
	{
		syslog(LOG_ERR, "Cannot open slave PTY.");
		return -2;
	}

	//Save terminal settings to be restored at exit.
	terminal_save_settings(m_slaveFD);

	m_pid = fork();

	if (m_pid < 0)
	{
		syslog(LOG_ERR, "Cannot fork process.");
		return -3;
	}

	if (m_pid == 0)
	{
		syslog(LOG_INFO, "Initializing child terminal process.");

		//Child process.
		if (setsid() < 0)
		{
			syslog(LOG_ERR, "Cannot set child session.");
			return -4;
		}

		for (int i = 0, max = sysconf(_SC_OPEN_MAX); i < max ; i++)
		{
			if(i != m_slaveFD)
			{
				close(i);
				
				if (i == m_masterFD)
				{
					m_masterFD = -1;
				}
			}
		}

		//Acquire controlling of terminal.
		if (openPTYSlave() != 0)
		{
			syslog(LOG_ERR, "Cannot acquire slave as controlling terminal.");
			return -5;
		}

		if (dup2(m_slaveFD, STDIN_FILENO) != STDIN_FILENO)
		{
			syslog(LOG_ERR, "Cannot duplicate slave into stdin.");
			return -7;
		}

		if (dup2(m_slaveFD, STDOUT_FILENO) != STDOUT_FILENO)
		{
			syslog(LOG_ERR, "Cannot duplicate slave into stdout.");
			return -8;
		}

		if (dup2(m_slaveFD, STDERR_FILENO) != STDERR_FILENO)
		{
			syslog(LOG_ERR, "Cannot duplicate slave into stderr.");
			return -9;
		}

		// THIS BREAK WHEN SYSLOG IS USED ~ PTM
		//Logger::getInstance()->info("Initialized child terminal process.");
	}
	else
	{
		syslog(LOG_INFO, "Created child terminal process with PID %d.", m_pid);
	}

	if (atexit(terminal_restore_settings) != 0)
	{
		syslog(LOG_INFO, "Cannot set terminal restore state function at exit.");
		return -10;
	}

	return 0;
}

int Terminal::setFlag(int fileDesc, int flags)
{
	int val = fcntl(fileDesc, F_GETFL, 0);

	if (val < 0)
	{
		syslog(LOG_ERR, "Cannot get flags from file descriptor.");
		return -1;
	}

	val |= flags;

	if (fcntl(fileDesc, F_SETFL, val) < 0)
	{
		syslog(LOG_ERR, "Cannot set flags on file descriptor.");
		return -1;
	}

	return 0;
}

int Terminal::sendCommand(const char *command)
{
	int cmdCharIndex = 0;
	char dataBuffer[4];
	struct timeval timeout;
	fd_set writeFDSet;
	bool bDone = false;

	unsigned bytes = strlen(command);
	const char * buf = command;

	while(bytes)
	{
		FD_ZERO(&writeFDSet);
		FD_SET(m_masterFD, &writeFDSet);

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		if (select(m_masterFD + 1, NULL, &writeFDSet, NULL, &timeout) < 1)
			continue;

		int res = write(m_masterFD, buf, bytes);

		if (res < 0)
		{
			// If we have written all we can, loop back to select()
			if (errno == EWOULDBLOCK)
				continue;

			// If write failed, bail
			syslog(LOG_WARNING, "Error writing command to terminal!");
			return -1;
		}

		// Otherwise, adjust buffer/bytes by amount written and continue
		buf += res;
		bytes -= res;
	}

	return 0;
}

bool Terminal::isChild()
{
	return (m_pid == 0);
}

int Terminal::start()
{
	syslog(LOG_INFO, "Starting pseudo terminal.");

	int result = forkPTY();

	if (result == 0)
	{
		if (isChild())
		{
			const char *argv[5];

			argv[0] = "/bin/login";
			argv[1] = "-p";
			argv[2] = "-f";
			argv[3] = getUser();
			argv[4] = NULL;

			setWindowSize();
			setTermMode();

			if (execvp(((char **)argv)[0], ((char **)argv)) < 0)
			{
				syslog(LOG_ERR, "Cannot execute child shell.");
			}

			_exit(0);
		}

		sleep(1);

		if (startReaderThread() == 0)
		{
			syslog(LOG_INFO, "Started pseudo terminal.");
			setReady(true);
		}
		else
		{
			syslog(LOG_ERR, "Cannot create reader thread.");
			result = -1;
		}
	}

	return result;
}

int Terminal::runReader()
{
	char dataBuffer[256];
	size_t dataBufferSize = sizeof(dataBuffer);
	ssize_t readResult;
	struct timeval timeout;
	fd_set readFDSet;

	// Loop until m_bDone is set (elsewhere)
	while (!m_bDone)
	{
		FD_ZERO(&readFDSet);
		FD_SET (m_masterFD, &readFDSet);

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		if (select(m_masterFD + 1, &readFDSet, NULL, NULL, &timeout) < 1)
			continue;

		// Read all available data:
		while(true)
		{
			readResult = read(m_masterFD, dataBuffer, dataBufferSize);

			if (readResult < 0)
			{
				// If we've read all we can, leave this loop
				if (errno == EWOULDBLOCK)
					break;

				syslog(LOG_ERR, "Cannot read pseudo terminal.");
				return -1;
			}

			// If EOF, we're done
			if (readResult == 0)
				break;
			
			// We got data!  Add to our buffer
			m_dataBuffer->append(dataBuffer, readResult);

			// Flush buffer if it gets too large
			if (m_dataBuffer->size() > (1 << 20))
				flushOutputBuffer();
		}

		// Flush since we've read all that's available.
		flushOutputBuffer();
	}

	// Indicate we exited neatly.
	return 0;
}

void Terminal::flushOutputBuffer()
{
	// Not thread-safe, meant to only be called from
	// the reader thread.
	char *tmp;
	int nTmpSize;

	//Transfer buffer to terminal state then flush buffer.
	if (m_dataBuffer->size() > 0 && getExtTerminal() != NULL)
	{
		if (getExtTerminal()->isReady())
		{
			//Add a null terminating character.
			nTmpSize = (m_dataBuffer->size() + 1) * sizeof(char);
			tmp = (char *)malloc(nTmpSize);
			memset(tmp, 0, nTmpSize);

			m_dataBuffer->copy(tmp, m_dataBuffer->size());
			m_dataBuffer->clear();

			getExtTerminal()->insertData(tmp, nTmpSize);

			free(tmp);
		}
	}
}

int Terminal::startReaderThread()
{
	return pthread_create(&m_readerThread, NULL, readerThread, this);
}

void *Terminal::readerThread(void *terminal)
{
	((Terminal *)terminal)->runReader();
	pthread_exit(NULL);
	return NULL;
}

/**
 * Sets the window size when the TTY is initialized. Must be called before starting
 * the terminal to take effect.
 */
void Terminal::setWindowSize(int nWidth, int nHeight)
{
	if (nWidth < 0)
	{
		nWidth = 0;
	}

	if (nHeight < 0)
	{
		nHeight = 0;
	}

	m_winSize.ws_col = nWidth;
	m_winSize.ws_row = nHeight;

	setWindowSize();
}

/**
 * Sets window size on the slave device. Should only be called on the child TTY process.
 */
int Terminal::setWindowSize()
{
	if (m_winSize.ws_col == 0 && m_winSize.ws_row == 0)
	{
		//No change.
		return 0;
	}

	if (m_winSize.ws_col <= 0 || m_winSize.ws_row <= 0)
	{
		syslog(LOG_WARNING, "Invalid window size.");
		return -1;
	}

	if (ioctl(m_slaveFD, TIOCSWINSZ, &m_winSize) < 0)
	{
		syslog(LOG_ERR, "Cannot set window size.");
		return -1;
	}

	return 0;
}

int Terminal::setCBreak()
{
	struct termios settings;

	if (tcgetattr(m_slaveFD, &settings) < 0)
	{
		syslog(LOG_ERR, "Cannot read terminal settings.");
		return -1;
	}

	settings.c_lflag &= ~ICANON;
	settings.c_cc[VMIN] = 1;
	settings.c_cc[VTIME] = 0;

	if (tcsetattr(m_slaveFD, TCSANOW, &settings) < 0)
	{
		syslog(LOG_ERR, "Cannot write CBreak terminal settings.");
		return -2;
	}

	return 0;
}

int Terminal::setTermMode()
{
	struct termios settings;

	if (tcgetattr(m_slaveFD, &settings) < 0)
	{
		syslog(LOG_ERR, "Cannot read terminal settings.");
		return -1;
	}

	settings.c_lflag = (ISIG | ICANON | ECHO | ECHOE | ECHOK);

#ifdef ECHOKE
	settings.c_lflag |= (ECHOKE | IEXTEN);
#endif

#ifdef ECHOCTL
	settings.c_lflag |= (ECHOCTL | IEXTEN);
#endif

	settings.c_iflag = (ICRNL | IXON);
	settings.c_cflag = (CS8 | CREAD | PARENB | HUPCL);

#ifdef TAB3
	settings.c_oflag = (OPOST | ONLCR | TAB3);
#else
	#ifdef ONLCR
		settings.c_oflag = (OPOST | ONLCR);
	#else
		settings.c_oflag = OPOST;
	#endif
#endif

	settings.c_cc[VMIN] = 1;
	settings.c_cc[VTIME] = 0;

	if (tcsetattr(m_slaveFD, TCSANOW, &settings) < 0)
	{
		syslog(LOG_ERR, "Cannot write default terminal settings.");
		return -2;
	}

	return 0;
}

int Terminal::setRaw()
{
	struct termios settings;

	if (tcgetattr(m_slaveFD, &settings) < 0)
	{
		syslog(LOG_ERR, "Cannot read terminal settings.");
		return -1;
	}

	settings.c_lflag &= ~(ICANON | IEXTEN | ISIG);
	settings.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	settings.c_cflag &= ~(CSIZE | PARENB);
	settings.c_cflag |= CS8;
	settings.c_oflag &= ~(OPOST);
	settings.c_cc[VMIN] = 1;
	settings.c_cc[VTIME] = 0;

	if (tcsetattr(m_slaveFD, TCSANOW, &settings) < 0)
	{
		syslog(LOG_ERR, "Cannot write Raw terminal settings.");
		return -2;
	}

	return 0;
}

void Terminal::insertData(const char *data, size_t size)
{
	if (size > 0)
	{
		/*
		printf("Send command: ");
		for (int i=0; i<strlen(data); i++)
		{
			printf("[%d]'%c'", data[i], data[i]);
		}
		printf("\n");
		*/

		sendCommand(data);
	}
}

const char *Terminal::getUser()
{
	return m_sUser;
}

void Terminal::setUser(const char *sUser)
{
	if (sUser != NULL)
	{
		size_t userSize = (strlen(sUser) + 1) * sizeof(char);

		m_sUser = (char *)realloc(m_sUser, userSize);
		memcpy(m_sUser, sUser, userSize);
	}
}
