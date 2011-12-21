include ../../../Makefile.inc

O = o

include libobjs.mif

PDCURSES_SDL_H	= pdcsdl.h

SFLAGS		= -I$(ROOT)${PREFIX}/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -DPDC_RGB
SLIBS		= -lSDL -lpthread

# If your system doesn't have these, remove the defines here
SFLAGS		+= -DHAVE_VSNPRINTF -DHAVE_VSSCANF


BUILD		= $(CC) $(CFLAGS) -I.

LINK		= $(CC)
LDFLAGS		= $(LIBCURSES) $(SLIBS)
LIBCURSES	= libpdcurses.a

.PHONY: libs clean

libs:	$(LIBCURSES)

clean:
	-rm -rf *.o trace $(LIBCURSES)

$(LIBCURSES) : $(LIBOBJS) $(PDCOBJS)
	$(AR) rv $@ $?
	$(RANLIB) $@

$(LIBOBJS) $(PDCOBJS) : $(PDCURSES_HEADERS)
$(PDCOBJS) : $(PDCURSES_SDL_H)
tui.o tuidemo.o : $(PDCURSES_CURSES_H)
terminfo.o: $(TERM_HEADER)
panel.o ptest: $(PANEL_HEADER)

$(LIBOBJS) : %.o: $(srcdir)/%.c
	$(BUILD) $(SFLAGS) -c $<

$(PDCOBJS) : %.o: %.c
	$(BUILD) $(SFLAGS) -c $<
