include Makefile.inc

APP			:= wterm
VENDOR		:= us.ryanhope
APPID 		:= $(VENDOR).$(APP)

VERSION		:= $(shell cat appinfo.json | grep version | cut -f 2 -d ":" | cut -f 2 -d "\"")
IPK			:= $(APPID)_$(VERSION)_$(ARCH).ipk

.PHONY: install clean

ipk/$(IPK): wterm bin/vttest
	- rm -rf ipk
	- mkdir -p ipk
	- palm-package -X excludes.txt .
	- mv $(APPID)_*.ipk ipk/$(IPK)
	- ar q ipk/$(IPK) pmPostInstall.script
	- ar q ipk/$(IPK) pmPreRemove.script

bin:
	- mkdir -p bin

wterm: bin
	- cd src/plugin; $(MAKE)
	- mv src/plugin/wterm wterm

bin/vttest: bin
	- cd src/vttest; $(MAKE)
	- mv src/vttest/vttest bin/vttest

uninstall:
	- palm-install -r $(APPID)

install: ipk/$(IPK)
	- palm-install ipk/$(IPK)
	
test: install
	- palm-launch $(APPID)

clean:
	- rm -rf ipk
	- rm -rf bin
	- rm -rf wterm
	- cd src/plugin; make clean
	- cd src/vttest; make clean
