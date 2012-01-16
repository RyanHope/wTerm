include ../../Makefile.inc

CXXFLAGS	:=	$(MARCH_TUNE) -I. -I$(ROOT)/include -I$(ROOT)/include/SDL
LDFLAGS		:=	-L$(ROOT)/lib -Wl,--allow-shlib-undefined -lpdl -lSDL -lSDL_ttf -lSDL_image -lGLES_CM -lpthread

OBJECTS		:=	sdl/sdlfontgl.o sdl/sdlcore.o sdl/sdlterminal.o util/databuffer.o util/point.o \
				terminal/seqparser.o terminal/terminal.o terminal/terminalstate.o terminal/vtterminalstate.o terminalmain.o

wterm: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o $@
	
$(OBJECTS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

clean:
	- rm -f wterm
	- rm -f `find . -name "*.o"`
