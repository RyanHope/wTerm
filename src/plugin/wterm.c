/*
Copyright (C) 2010 Sumedha Widyadharma <asmw@asmw.de>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <SDL.h>
#include <curses.h>
#include <stdio.h>
#include <signal.h>
#include <locale.h>
#include <pdcsdl.h>
#include <vterm.h>
#include <vterm_write.h>
#include <syslog.h>

#include <stdlib.h>
#include <pty.h>
#include <string.h>
#include <pwd.h>
#include <utmp.h>
#include <termios.h>
#include <fcntl.h>

#include <PDL.h>

int screen_w, screen_h;
WINDOW *term_win;
vterm_t *vterm;

PDL_bool plugin_write(PDL_JSParameters *params) {

	const char *buffer = PDL_GetJSParamString(params, 0);
	syslog(LOG_ALERT, "%s", buffer);
	int i = 0;

	for (;buffer[i];i++)
		vterm_write_pipe(vterm,buffer[i]);

	return PDL_TRUE;
}

int main() {

	openlog("us.ryanhope.wterm.plugin", LOG_PID, LOG_USER);

	int fd = open("/dev/ptmx", O_RDWR);
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);
	close(fd);
	syslog(LOG_ALERT, "OPEN: %d", fd);
	syslog(LOG_ALERT, "IOCTL: %d", ioctl(fd, TIOCSCTTY, 0/1));

	int      i, j, ch;
	char        *locale;
	ssize_t  bytes;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		exit(1);
	}
	PDL_Init(0);

	PDL_RegisterJSHandler("write", plugin_write);

	PDL_JSRegistrationComplete();
	PDL_CallJS("ready", NULL, 0);

	pdc_screen = SDL_SetVideoMode(0, 0, 0, SDL_SWSURFACE);

	locale=setlocale(LC_ALL,"");

	atexit(SDL_Quit);
	initscr();
	noecho();
	start_color();
	scrollok(stdscr, TRUE);
	raw();
	nodelay(stdscr, TRUE);       /* prevents getch() from blocking; rather
	 * it will return ERR when there is no
	 * keypress available */

	keypad(stdscr, TRUE);        /* necessary to use rote_vt_keypress */
	getmaxyx(stdscr, screen_h, screen_w);
	syslog(LOG_ALERT, "Curses max dimensions: %ux%u\n",screen_w, screen_h);

	/* initialize the color pairs the way rote_vt_draw expects it. You might
	 * initialize them differently, but in that case you would need
	 * to supply a custom conversion function for rote_vt_draw to
	 * call when setting attributes. The idea of this "default" mapping
	 * is to map (fg,bg) to the color pair bg * 8 + 7 - fg. This way,
	 * the pair (white,black) ends up mapped to 0, which means that
	 * it does not need a color pair (since it is the default). Since
	 * there are only 63 available color pairs (and 64 possible fg/bg
	 * combinations), we really have to save 1 pair by assigning no pair
	 * to the combination white/black. */
	for (i = 0; i < 8; i++) for (j = 0; j < 8; j++)
		if (i != 7 || j != 0)
			init_pair(j*8+7-i, i, j);

	/* paint the screen blue */
	for (i = 0; i < screen_h; i++) for (j = 0; j < screen_w; j++) addch(' ');
	refresh();

	/* create a window with a frame */
	term_win = newwin(screen_h,screen_w,0,0);
	wattrset(term_win, COLOR_PAIR(0*8+7-7)); /* black over white */
	wrefresh(term_win);

	/* create the terminal and have it run bash */

	vterm=vterm_create(screen_w,screen_h,0);

	syslog(LOG_ALERT, "TTY NAME: %s", vterm_get_ttyname(vterm));

	vterm_set_colors(vterm,COLOR_WHITE,COLOR_BLACK);
	vterm_wnd_set(vterm,term_win);

	/* keep reading keypresses from the user and passing them to the terminal;
	 * also, redraw the terminal to the window at each iteration */
	ch = '\0';
	while (TRUE) {
		ch = getch();
		if (ch != ERR) {
			syslog(LOG_ALERT, "%x\n", ch);
			vterm_write_pipe(vterm,ch);
		}

		bytes=vterm_read_pipe(vterm);
		if(bytes > 0)
		{
			vterm_wnd_update(vterm);
			touchwin(term_win);
			wrefresh(term_win);
			refresh();
		}

		if(bytes==-1) break;
	}

	endwin();
	return 0;
}
