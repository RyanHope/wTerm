include ../../Makefile.inc

CXXFLAGS	:=	$(MARCH_TUNE) -I. -I$(ROOT)/include -I$(ROOT)/include/SDL
LDFLAGS		:=	-L$(ROOT)/lib -lpthread -lpdl -lGLES_CM -lGLESv2 -lSDL -lSDL_ttf -lSDL_image


OBJECTS		:=	sdl/sdlcore.cpp sdl/sdlterminal.cpp \
				util/configmanager.cpp util/databuffer.cpp util/point.cpp \
				terminal/seqparser.cpp terminal/terminal.cpp terminal/terminalconfigmanager.cpp \
				terminal/terminalstate.cpp terminal/vtterminalstate.cpp terminalmain.cpp

wterm: 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	- rm -rf wterm