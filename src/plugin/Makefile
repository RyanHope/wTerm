include ../../Makefile.inc

CXXFLAGS    := $(MARCH_TUNE) -I. -I$(ROOT)/include -I$(ROOT)/include/SDL -DVERSION=\"$(shell git describe --tags --always)\"
LDFLAGS     := -L$(ROOT)/lib -Wl,--allow-shlib-undefined -lpdl -lSDL -lSDL_ttf -lSDL_image -lGLESv2 -lpthread -lrt

OBJECTS     := sdl/sdlfontgl.o sdl/sdlcore.o sdl/sdlcore_events.o sdl/sdlgeneric.o sdl/wterm.o sdl/webos.o \
               util/point.o util/utf8.o util/glutils.o util/utils.o \
               terminal/seqparser.o terminal/terminal.o terminal/terminalstate.o terminal/vtterminalstate.o terminal/screenbuffer.o \
               main_terminal.o main_setup.o main.o

all: wterm

wterm: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJECTS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

clean:
	- rm -f wterm
	- rm -f `find . -name "*.o"`
