APP			:= wterm
VENDOR		:= us.ryanhope
APPID 		:= ${VENDOR}.${APP}

VERSION		:= $(shell cat appinfo.json | grep version | cut -f 2 -d ":" | cut -f 2 -d "\"")
IPK			:= ${APPID}_${VERSION}_all.ipk

.PHONY: install clean

${IPK}: wterm
	palm-package -X excludes.txt .
	ar q ${APPID}_*.ipk pmPostInstall.script
	ar q ${APPID}_*.ipk pmPreRemove.script
	
wterm:
	cd src/plugin; ${MAKE}
	mv src/plugin/wterm .

uninstall:
	- palm-install -r ${APPID}

install: package
	palm-install ${APPID}_*.ipk
	palm-launch ${APPID}

clean:
	rm -rf *.ipk
	rm -rf wterm
	cd src/plugin; make clean
