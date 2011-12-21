APPID = us.ryanhope.wterm

.PHONY: package install clean

package: wterm
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
