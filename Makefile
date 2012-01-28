include Makefile.inc

APP			:= wterm
VENDOR		:= us.ryanhope
APPID 		:= $(VENDOR).$(APP)

VERSION		:= $(shell cat appinfo.json | grep version | cut -f 2 -d ":" | cut -f 2 -d "\"")
IPK			:= $(APPID)_$(VERSION)_$(ARCH).ipk

.PHONY: wterm package install clean

package: clean-package ipk/$(IPK)

ipk/$(IPK): wterm bin/vttest bin/cmatrix clean-package
	- mkdir -p ipk
	- palm-package -X excludes.txt .
	- mv $(APPID)_*.ipk ipk/$(IPK)
	- ar q ipk/$(IPK) pmPostInstall.script
	- ar q ipk/$(IPK) pmPreRemove.script

clean-package:
	- rm -rf ipk

wterm:
	$(MAKE) -C src/plugin
	mv src/plugin/wterm wterm
	mv src/plugin/setup setup

bin/vttest:
	$(MAKE) -C src/vttest
	mv src/vttest/vttest bin/vttest

bin/cmatrix:
	$(MAKE) -C src/cmatrix
	mv src/cmatrix/cmatrix bin/cmatrix

uninstall:
	- palm-install -r $(APPID)

install: package
	palm-install ipk/$(IPK)

test: install
	palm-launch $(APPID)

clean:
	- rm -rf ipk
	- rm -rf bin/*
	- rm -rf wterm
	- rm -rf setup
	- $(MAKE) -C src/plugin clean
	- $(MAKE) -C src/vttest clean
	- $(MAKE) -C src/cmatrix clean
