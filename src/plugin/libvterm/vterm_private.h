/*
Copyright (C) 2009 Bryan Christ

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

/*
This library is based on ROTE written by Bruno Takahashi C. de Oliveira
*/

#ifndef _VTERM_PRIVATE_H_
#define _VTERM_PRIVATE_H_

#include <unistd.h>

#include <sys/types.h>

#include <curses.h>
#include <glib.h>

#define ESEQ_BUF_SIZE 128  		         // size of escape sequence buffer.

#define STATE_ALT_CHARSET     (1<<1)
#define STATE_ESCAPE_MODE     (1<<2)
#define STATE_PIPE_ERR        (1<<3)
#define STATE_CHILD_EXITED    (1<<4)
#define STATE_CURSOR_INVIS    (1<<5)
#define STATE_SCROLL_SHORT    (1<<6)      // scrolling region is not full height

#define IS_MODE_ESCAPED(x)    (x->state & STATE_ESCAPE_MODE)
#define IS_MODE_ACS(x)        (x->state & STATE_ALT_CHARSET)

typedef struct _vterm_cell_s vterm_cell_t;

struct _vterm_s
{
	gint	         rows,cols;                    // terminal height & width
   WINDOW         *window;                      // curses window
   vterm_cell_t   **cells;
   gchar          ttyname[96];                  // populated with ttyname_r()
   guint          curattr;                      // current attribute set
	gint	         crow,ccol;			            // current cursor column & row
	gint	         scroll_min;					      // top of scrolling region
	gint	         scroll_max;					      // bottom of scrolling region
	gint	         saved_x,saved_y;			      // saved cursor coords
   short          colors;                       // color pair for default fg/bg
   gint           fg,bg;                        // current fg/bg colors
   gchar	         esbuf[ESEQ_BUF_SIZE]; 	      /* 0-terminated string. Does
                                                   NOT include the initial
                                                   escape (\x1B) character.   */
   gint 	         esbuf_len;    	               /* length of buffer. The
                                                   following property is
                                                   always kept:
                                                   esbuf[esbuf_len] == '\0' 	*/
   gint 	         pty_fd;                	      /* file descriptor for the pty
                                                   attached to this terminal. */
   pid_t          child_pid;                    // pid of the child process
	guint	         flags;						      // user options
   guint          state;                        // internal state control
   void           (*write) (vterm_t*,guint32);
};

struct _vterm_cell_s
{
   chtype         ch;                           // cell data
   guint          attr;                         // cell attributes
};

#define VTERM_CELL(vterm_ptr,x,y)               ((y*vterm_ptr->cols)+x)

#endif

