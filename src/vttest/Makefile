include ../../Makefile.inc

CFLAGS	:=	$(MARCH_TUNE) -I. -I$(ROOT)/include -DHAVE_CONFIG_H
LDFLAGS		:=	-L$(ROOT)/lib

OBJECTS		:=		charsets.o color.o draw.o esc.o keyboard.o main.o mouse.o \
								nonvt100.o printer.o reports.o reset.o setup.o sixel.o \
								status.o tek4014.o ttymodes.o unix_io.o utf8.o vt220.o \
								vt320.o vt420.o vt52.o vt520.o xterm.o

vttest: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@
	
$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	- rm -f vttest
	- rm -f `find . -name "*.o"`