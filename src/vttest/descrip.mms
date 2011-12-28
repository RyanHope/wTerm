# $Id: descrip.mms,v 1.17 2010/08/28 12:21:13 tom Exp $
# VAX/VMS "mms" script for VTTEST

THIS = vttest

#### Start of system configuration section. ####

DEFINES = /Define=(STDC_HEADERS)
CFLAGS	= /Listing /Include=([]) $(DEFINES)

#### End of system configuration section. ####

C_SRC = \
	charsets.c \
	color.c \
	draw.c \
	esc.c \
	keyboard.c \
	main.c \
	mouse.c \
	nonvt100.c \
	printer.c \
	reports.c \
	reset.c \
	setup.c \
	sixel.c \
	status.c \
	tek4014.c \
	ttymodes.c \
	unix_io.c \
	utf8.c \
	vms_io.c \
	vt220.c \
	vt320.c \
	vt420.c \
	vt52.c \
	vt520.c \
	xterm.c
H_SRC = \
	vttest.h \
	draw.h \
	esc.h \
	ttymodes.h
OBJS = \
	charsets.obj, \
	color.obj, \
	draw.obj, \
	esc.obj, \
	keyboard.obj, \
	main.obj, \
	mouse.obj, \
	nonvt100.obj, \
	printer.obj, \
	reports.obj, \
	reset.obj, \
	setup.obj, \
	sixel.obj, \
	status.obj, \
	tek4014.obj, \
	utf8.obj, \
	vms_io.obj, \
	vt220.obj, \
	vt320.obj, \
	vt420.obj, \
	vt52.obj, \
	vt520.obj, \
	xterm.obj

SRC =	patchlev.h \
	CHANGES COPYING README BUGS \
	$(THIS).1 \
	$(C_SRC) $(H_SRC) \
	config.hin install.sh mkdirs.sh makefile.in configure.in

all : $(THIS).exe
	@ write sys$output "** produced $?"

$(THIS).exe : $(OBJS), vms_link.opt
	$(LINK)/exec=$(THIS) main.obj, vms_link/opt

vms_link.opt :
	@vmsbuild vms_link_opt

$(THIS).com :
	if "''f$search("vttest.com")'" .nes. "" then delete vttest.com;*
	copy nl: vttest.com
	open/append  test_script vttest.com
	write test_script "$ temp = f$environment(""procedure"")"
	write test_script "$ temp = temp -"
	write test_script "		- f$parse(temp,,,""version"",""syntax_only"") -"
	write test_script "		- f$parse(temp,,,""type"",""syntax_only"")"
	write test_script "$ vttest :== $ 'temp'.exe"
	write test_script "$ define/user_mode sys$input  sys$command"
	write test_script "$ define/user_mode sys$output sys$command"
	write test_script "$ vttest 'p1 'p2 'p3 'p4 'p5 'p6 'p7 'p8"
	close test_script
	write sys$output "** made vttest.com"

clean :
	- if f$search("*.obj").nes."" then dele/nolog *.obj;*
	- if f$search("*.lis").nes."" then dele/nolog *.lis;*
	- if f$search("*.log").nes."" then dele/nolog *.log;*
	- if f$search("*.map").nes."" then dele/nolog *.map;*

clobber : clean
	- if f$search("$(THIS).exe").nes."" then dele/nolog $(THIS).exe;*

$(OBJS) : vttest.h
