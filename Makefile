include Makefile.inc

APP			:= wterm
VENDOR		:= us.ryanhope
APPID 		:= ${VENDOR}.${APP}

VERSION		:= $(shell cat appinfo.json | grep version | cut -f 2 -d ":" | cut -f 2 -d "\"")
IPK			:= ${APPID}_${VERSION}_${ARCH}.ipk

.PHONY: install clean

${IPK}: clean wterm bin/vttest 
	palm-package -X excludes.txt .
	- mv ${APPID}_*.ipk ${IPK}
	ar q ${IPK} pmPostInstall.script
	ar q ${IPK} pmPreRemove.script

bin:
	- mkdir -p bin

wterm: bin
	- cd src/plugin; ${MAKE}
	- cp src/plugin/wterm wterm

bin/vttest: bin
	cd src/vttest; CC=${CC} ./configure --host=${HOST}; ${MAKE}
	cp src/vttest/vttest bin/vttest

uninstall:
	- palm-install -r ${APPID}

install: ${IPK}
	palm-install ${IPK}
	palm-launch ${APPID}

clean:
	- rm -rf *.ipk
	- rm -rf bin
	- rm -rf wterm
	- cd src/plugin; make clean
	- cd src/vttest; make clean
