include Makefile.inc

PLUGIN		:=	wterm

PDLLIB		:=	libpdl
VTERMLIB	:=	libvterm
PDCURSESLIB	:=	pdcurses

CFLAGS		:=	$(MARCH_TUNE) -I. -Ilibpdl -Ilibvterm -Ipdcurses \
				-I$(ROOT)${PREFIX}/include -I$(ROOT)${PREFIX}/include/SDL \
				-I$(ROOT)${PREFIX}/include/glib-2.0 -I$(ROOT)${PREFIX}/lib/glib-2.0/include
								
LIBDIRS		:=	-L$(ROOT)${PREFIX}/lib -L$(ROOT)${PREFIX}/lib/pulse -Llibpdl -Llibnapp

LIBS		:=	-Wl,--allow-shlib-undefined -lgdbm -lpulse -lpulse-simple -lasound -lutil -lglib-2.0 -lpdl -lSDL

.PHONY		:	clean clobber

all: $(PLUGIN)

$(PLUGIN): $(OBJECTS) $(PDLLIB)/$(PDLLIB).so $(VTERMLIB)/$(VTERMLIB).a $(PDCURSESLIB)/$(PDCURSESLIB).a
	$(CC) $(CFLAGS) $(LIBDIRS) $(LIBS) wterm.c libvterm/libvterm.a pdcurses/libpdcurses.a -o $(PLUGIN)

.PHONY: $(PDLLIB)/$(PDLLIB).so
$(PDLLIB)/$(PDLLIB).so:
	cd $(PDLLIB); $(MAKE)
	
.PHONY: $(VTERMLIB)/$(VTERMLIB).a
$(VTERMLIB)/$(VTERMLIB).a:
	cd $(VTERMLIB); $(MAKE)

.PHONY: $(PDCURSESLIB)/$(PDCURSESLIB).a
$(PDCURSESLIB)/$(PDCURSESLIB).a:
	cd $(PDCURSESLIB); $(MAKE)

push: 

clobber: clobber-$(PDLLIB) clobber-$(PDCURSESLIB) clobber-$(VTERMLIB)
	rm -rf *.o $(PLUGIN)
	
clobber-$(PDLLIB):
	cd $(PDLLIB); $(MAKE) clean
	
clobber-$(VTERMLIB):
	cd $(VTERMLIB); $(MAKE) clean
	
clobber-$(PDCURSESLIB):
	cd $(PDCURSESLIB); $(MAKE) clean
	
clean: clobber

