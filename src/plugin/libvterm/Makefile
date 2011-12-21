include ../Makefile.inc


DEFS = -D_REENTRANT -DFIONREAD
INCLUDES = -I../pdcurses -I$(ROOT)${PREFIX}/include/glib-2.0 -I$(ROOT)${PREFIX}/lib/glib-2.0/include

makefile: all

all: libvterm.a

libvterm.a:
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFS) -c -fPIC *.c
	$(AR) rv $@ *.o

clean:
	rm -f *.o
	rm -f *.a