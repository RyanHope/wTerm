include Makefile.inc

APP         := wterm
VENDOR      := us.ryanhope
APPID       := $(VENDOR).$(APP)

VERSION     := $(shell cat appinfo.json | grep version | cut -f 2 -d ":" | cut -f 2 -d "\"")
IPK         := $(APPID)_$(VERSION)_$(ARCH).ipk

.PHONY: package install clean clean-package bins src/plugin/wterm src/vttest/vttest src/cmatrix/cmatrix

package: clean-package ipk/$(IPK) wterm

ipk/$(IPK): wterm bin/vttest bin/cmatrix clean-package
	mkdir -p ipk
	palm-package -X excludes.txt .
	mv $(APPID)_*.ipk ipk/$(IPK)
	ar q ipk/$(IPK) pmPostInstall.script
	ar q ipk/$(IPK) pmPreRemove.script

clean-package:
	- rm -rf ipk

bins: wterm bin/vttest bin/cmatrix

src/plugin/wterm:
	$(MAKE) -C src/plugin wterm

wterm: src/plugin/wterm
	cp src/plugin/wterm wterm

src/vttest/vttest:
	$(MAKE) -C src/vttest

bin/vttest: src/vttest/vttest
	mkdir -p bin
	cp src/vttest/vttest bin/vttest

src/cmatrix/cmatrix:
	$(MAKE) -C src/cmatrix

bin/cmatrix: src/cmatrix/cmatrix
	mkdir -p bin
	cp src/cmatrix/cmatrix bin/cmatrix

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
	- $(MAKE) -C src/plugin clean
	- $(MAKE) -C src/vttest clean
	- $(MAKE) -C src/cmatrix clean

resetFirstUse:
	novacom -- run file://usr/bin/luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"us.ryanhope.wterm","params":{"resetFirstUse": true}}'