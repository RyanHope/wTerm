ifdef DEVICE
	ARCH=armv7
	MARCH_TUNE=-O2 -march=armv7-a -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp
	ifeq ($(shell uname -s),Darwin)
		CC=/opt/PalmPDK/arm-gcc/bin/arm-none-linux-gnueabi-gcc
		AR=/opt/PalmPDK/arm-gcc/bin/arm-none-linux-gnueabi-ar
	else
		CC=/srv/preware/build/toolchain/cs07q3armel/build/arm-2007q3/bin/arm-none-linux-gnueabi-gcc
		AR=/srv/preware/build/toolchain/cs07q3armel/build/arm-2007q3/bin/arm-none-linux-gnueabi-ar
		ROOT=/srv/preware/build/staging/armv7
		PREFIX=/usr
	endif
else
	ARCH=i686
	ifeq ($(shell uname -s),Darwin)
		export CC=/opt/PalmPDK/i686-gcc/bin/i686-nptl-linux-gnu-gcc --sysroot=/opt/PalmPDK/i686-gcc/sys-root
	else
		CC=/srv/preware/build/toolchain/i686-unknown-linux-gnu/build/i686-unknown-linux-gnu/bin/i686-unknown-linux-gnu-gcc
	endif
endif