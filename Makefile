include Makefile.inc

APP			:= wterm
VENDOR		:= us.ryanhope
APPID 		:= ${VENDOR}.${APP}

VERSION		:= $(shell cat appinfo.json | grep version | cut -f 2 -d ":" | cut -f 2 -d "\"")
IPK			:= ${APPID}_${VERSION}_${ARCH}.ipk

.PHONY: install clean

${IPK}: wterm
	palm-package -X excludes.txt .
	mv ${APPID}_*.ipk ${IPK}
	ar q ${IPK} pmPostInstall.script
	ar q ${IPK} pmPreRemove.script
	
wterm:
	cd src/plugin; ${MAKE}
	mv src/plugin/wterm .

uninstall:
	- palm-install -r ${APPID}

install: ${IPK}
	palm-install ${IPK}
	palm-launch ${APPID}

clean:
	rm -rf *.ipk
	rm -rf wterm
	cd src/plugin; make clean
