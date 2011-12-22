include ../../Makefile.inc

CXXFLAGS	:=	$(MARCH_TUNE)
ifeq ($(shell uname -s),Darwin)
CXXFLAGS	+=	-I. -I/opt/PalmPDK/include -I/opt/PalmPDK/include/SDL -L/opt/PalmPDK/device/lib
else
CXXFLAGS	:=	-I. -I$(ROOT)/include -I$(ROOT)/include/SDL -L$(ROOT)/lib
endif
LDFLAGS		:=	-Wl,--allow-shlib-undefined -lSDL -lSDL_ttf -lSDL_image -lGLES_CM -lpdl -lpthread

OBJECTS		:=	sdl/sdlcore.cpp sdl/sdlterminal.cpp \
				util/configmanager.cpp util/databuffer.cpp util/point.cpp \
				terminal/seqparser.cpp terminal/terminal.cpp terminal/terminalconfigmanager.cpp \
				terminal/terminalstate.cpp terminal/vtterminalstate.cpp terminalmain.cpp

wterm: 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	- rm -rf wterm