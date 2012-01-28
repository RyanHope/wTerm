include ../../Makefile.inc

CFLAGS      := $(MARCH_TUNE) -I. -I$(ROOT)/include -I$(ROOT)/include/ncurses -DHAVE_CONFIG_H
LDFLAGS     := -L$(ROOT)/lib -lncurses

OBJECTS     := cmatrix.o

cmatrix: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	- rm -f cmatrix
	- rm -f `find . -name "*.o"`