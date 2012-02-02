include ../../Makefile.inc

CXXFLAGS    := $(MARCH_TUNE) -I. -I$(ROOT)/include -I$(ROOT)/include/SDL
LDFLAGS     := -L$(ROOT)/lib -Wl,--allow-shlib-undefined -lpdl -lSDL -lSDL_ttf -lSDL_image -lGLESv2 -lpthread -lrt

OBJECTS     := sdl/sdlfontgl.o sdl/sdlcore.o sdl/wterm.o util/point.o util/utf8.o util/glutils.o util/utils.o \
               terminal/seqparser.o terminal/terminal.o terminal/terminalstate.o terminal/vtterminalstate.o terminal/screenbuffer.o \
               terminalmain.o

all: wterm setup

wterm: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

setup:
	$(CC) $(CXXFLAGS) $(LDFLAGS) $@.c -o $@

$(OBJECTS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

clean:
	- rm -f wterm setup
	- rm -f `find . -name "*.o"`
