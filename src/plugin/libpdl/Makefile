include ../Makefile.inc

INCLUDES	=	-I. -I$(ROOT)${PREFIX}/include/SDL
					
CFLAGS		=	-Os
				
LIBRARY		= 	libpdl.so

OBJECTS		= 	libpdl.o

.PHONY		: 	clean-objects clean


all: $(LIBRARY)

fresh: clean all

$(LIBRARY): $(OBJECTS)
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(LIBRARY) $(OBJECTS) -o $(LIBRARY) $(INCLUDES) $(LIBS)

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $<  -o $@ -I. $(INCLUDES) $(LIBS)
	
clean-objects:
	rm -rf $(OBJECTS)
	
clean: clean-objects
	rm -rf $(LIBRARY) *.o
