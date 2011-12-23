ifdef DEVICE
	ARCH=armv7
	ifeq ($(shell uname -s),Darwin)
		MARCH_TUNE=-O2 -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp
		CXX=/opt/PalmPDK/arm-gcc/bin/arm-none-linux-gnueabi-g++
	else
		MARCH_TUNE=-O2 -march=armv7-a -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp
		CXX=/srv/preware/build/toolchain/cs07q3armel/build/arm-2007q3/bin/arm-none-linux-gnueabi-g++
	endif
	ROOT=/srv/preware/build/staging/armv7/usr
else
	ARCH=i686
	ifeq ($(shell uname -s),Darwin)
		CXX=/opt/PalmPDK/i686-gcc/bin/i686-nptl-linux-gnu-g++ --sysroot=/opt/PalmPDK/i686-gcc/sys-root
	else
		CXX/srv/preware/build/toolchain/i686-unknown-linux-gnu/build/i686-unknown-linux-gnu/bin/i686-unknown-linux-gnu-g++
	endif
	ROOT=/srv/preware/build/staging/i686/usr
endif