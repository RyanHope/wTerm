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
		CC=/srv/preware/build/toolchain/cs07q3armel/build/arm-2007q3/bin/arm-none-linux-gnueabi-gcc
		CXX=/srv/preware/build/toolchain/cs07q3armel/build/arm-2007q3/bin/arm-none-linux-gnueabi-g++
		ROOT=/srv/preware/build/staging/armv7/usr
	endif
else
	ARCH=i686
	HOST=i686-unknown-linux-gnu
	ifeq ($(shell uname -s),Darwin)
		CC=/opt/PalmPDK/i686-gcc/bin/i686-nptl-linux-gnu-gcc --sysroot=/opt/PalmPDK/i686-gcc/sys-root
		CXX=/opt/PalmPDK/i686-gcc/bin/i686-nptl-linux-gnu-g++ --sysroot=/opt/PalmPDK/i686-gcc/sys-root
		ROOT=/opt/PalmPDK/device
	else
		CC/srv/preware/build/toolchain/i686-unknown-linux-gnu/build/i686-unknown-linux-gnu/bin/i686-unknown-linux-gnu-g++
		CXX/srv/preware/build/toolchain/i686-unknown-linux-gnu/build/i686-unknown-linux-gnu/bin/i686-unknown-linux-gnu-g++
		ROOT=/srv/preware/build/staging/i686/usr
	endif
endif