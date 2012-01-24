
BUILD_ROOT=/srv/preware/build

ifdef NATIVE

CC=gcc
CXX=g++
ROOT=/media/ext3fs/dev
MARCH_TUNE=-O2 -march=armv7-a -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp

else
ifdef DEVICE
	ARCH=armv7
	HOST=arm-none-linux-gnueabi
	ifeq ($(shell uname -s),Darwin)
		MARCH_TUNE=-O2 -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp
		CC=/opt/PalmPDK/arm-gcc/bin/arm-none-linux-gnueabi-gcc
		CXX=/opt/PalmPDK/arm-gcc/bin/arm-none-linux-gnueabi-g++
		ROOT=/opt/PalmPDK/device
	else
		MARCH_TUNE=-O2 -march=armv7-a -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp
		CC=$(BUILD_ROOT)/toolchain/cs09q1armel/build/arm-2009q1/bin/arm-none-linux-gnueabi-gcc
		CXX=$(BUILD_ROOT)/toolchain/cs09q1armel/build/arm-2009q1/bin/arm-none-linux-gnueabi-g++
		ROOT=$(BUILD_ROOT)/staging/armv7/usr
	endif
else
	ARCH=i686
	HOST=i686-unknown-linux-gnu
	ifeq ($(shell uname -s),Darwin)
		CC=/opt/PalmPDK/i686-gcc/bin/i686-nptl-linux-gnu-gcc --sysroot=/opt/PalmPDK/i686-gcc/sys-root
		CXX=/opt/PalmPDK/i686-gcc/bin/i686-nptl-linux-gnu-g++ --sysroot=/opt/PalmPDK/i686-gcc/sys-root
		ROOT=/opt/PalmPDK/device
	else
		CC=$(BUILD_ROOT)/toolchain/i686-unknown-linux-gnu/build/i686-unknown-linux-gnu/bin/i686-unknown-linux-gnu-g++
		CXX=$(BUILD_ROOT)/toolchain/i686-unknown-linux-gnu/build/i686-unknown-linux-gnu/bin/i686-unknown-linux-gnu-g++
		ROOT=$(BUILD_ROOT)/staging/i686/usr
	endif
endif
endif

ifeq ($(DEBUG),1)
	MARCH_TUNE+=-g
else ifeq ($(DEBUG),2)
	MARCH_TUNE+=-g -Wall
else ifeq ($(DEBUG),3)
	MARCH_TUNE+=-g -Wall -pedantic
endif

ifdef LOGLEVEL
	MARCH_TUNE+=-DLOGLEVEL=${LOGLEVEL}
else
	MARCH_TUNE+=-DLOGLEVEL=0
endif
