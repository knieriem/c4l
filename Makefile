#
#
# can4linux -- LINUX CAN device driver Makefile
# 
# Copyright (c) 2001/2 port GmbH Halle/Saale
# (c) 2001/2 Heinz-Jürgen Oertel (oe@port.de)
#
# to compile the can4linux device driver,
# please select some configuration variables.
# eg. the target hardware                       TARGET=
# with or without compiled debug-support        DEBUG= 
#

# Used release tag for this software version
VERSION=2
REL=6
RELEASE=CAN4LINUX-$(VERSION)_$(REL)
DVERSION=$(VERSION).$(REL)


KVERSION= $(shell uname -r)
CONFIG := $(shell uname -n)

TITLE = CAN driver can4linux

#
# The CAN driver major device number
# development starts with major=63
#  (LOCAL/EXPERIMENTAL USE)
# The new linux/Documentation/devices.txt defines major=91
CAN_MAJOR=	63
CAN_MAJOR=	91

# definitions for the hardware target
#########################################################################
# Only AT-CAN-MINI can be compiled for 82c200 or PeliCAN mode.
# All other Targets are assuming to have an SJA1000
# CPC_PCI  implies PeliCAN
#
#TARGET=IXXAT_PCI03
#TARGET=ATCANMINI_BASIC
#TARGET=IME_SLIMLINE
#TARGET=ATCANMINI_PELICAN
#TARGET=CPC_PCI
TARGET=ADNP1486

TARGET_MATCHED = false
# location of the compiled objects and the final driver module 
OBJDIR = obj

# Debugging Code within the driver
# to use the Debugging option
# and the Debugging control via /proc/sys/Can/DbgMask
# the Makefile in subdir Can must called with the DEBUG set to
# DEBUG=1
# else
# NODEBUG
# doesn't compile any debug code into the driver
DEBUG=NODEBUG
DEBUG=DEBUG=1

# all definitions for compiling the sources
# CAN_PELICANMODE       - use the SJA1000 PeliCAN mode instead 82c200 mode
# CAN_PORT_IO		- use port I/O instead of memeory I/O
# CAN4LINUX_PCI
# IODEBUG               - all register write accesses are logged
# CONFIG_TIME_MEASURE=1 - enable Time measurement at parallel port
#



ifeq "$(TARGET)" "ADNP1486"
# SSV ADNP 1486 (with 82C200 or SJA1000)
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_PORT_IO_INDIR -DCAN_PELICANMODE \

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "CPC_PCI"
# CPC-PCI PeliCAN  PCI (only with SJA1000)
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR)\
	-DCAN_PELICANMODE \
	-DCAN4LINUX_PCI

	#-DIODEBUG

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "ATCANMINI_PELICAN"
# AT-CAN-MINI PeliCAN ISA (only with SJA1000)
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_PORT_IO -DCAN_PELICANMODE \
	-DCONFIG_TIME_MEASURE=1

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "ATCANMINI_BASIC"
# AT-CAN-MINI  Basic CAN ISA (with 82C200 or SJA1000)
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_PORT_IO \

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "IXXAT_PCI03"
# IXXAT PC-I 03 board ISA (only with SJA1000)
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_PELICANMODE \

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "IME_SLIMLINE"
# I+ME  PcSlimline ISA (only with SJA1000)
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_PELICANMODE \

TARGET_MATCHED = true
endif


LIBS   =
CAN_MODULE = Can.o
PROJECTHOME=$(shell pwd)
INSTALLHOME=/usr/src
 


# TARGET=ELIMA selcts the powerpc-linux crosscompiler

# which sets defines like PPC, __PPC__, __PPC

ifeq "$(TARGET)" "ELIMA"
CC:=		powerpc-linux-gcc 
CFLAGS=		-v -s -O9 -fforce-addr -fforce-mem -ffast-math	\
		-fomit-frame-pointer -funroll-loops		\
		-DLOOPS=300000 -DTIMES -DHZ=100
DDLFLAGS = -I./ddllib -p powerpc-linux-

TARGET_MATCHED = true
else
CC:=		gcc
CFLAGS =  
endif

###########################################################################
ifneq "$(TARGET_MATCHED)" "true"
.DEFAULT: all ; @$(MAKE) all
all:	
	@echo "You didn't select a supported TARGET"
	@echo "select one of: ATCANMINI_BASIC, ATCANMINI_PELICAN, CPC_PCI, IME_SLIMLINE, IXXAT_PCI03"
else
###########################################################################
# select the compiler toolchain
###########################################################################
TOOLS=powerpc-linux-gcc 
TOOLS=arm-uclinux-
TOOLS=/usr/local/armtools_glibc/bin/arm-uclinux-
TOOLS=

BOLD		= "\033[1m"
NBOLD		= "\033[0m"

ECHO		= /bin/echo -e

COMPILE	= $(ECHO) "--- Compiling "$(BOLD)$<$(NBOLD)" for $(TARGET) ..." ; \
	  $(TOOLS)gcc
DEPEND	= $(ECHO) "--- Checking dependencies..." ; $(TOOLS)$(CPP)
RLINK	= $(ECHO) "--- Linking (relocatable) "$(BOLD)$@$(NBOLD)"..." ;\
	  $(TOOLS)ld -r
LINK	= $(ECHO) "--- Linking "$(BOLD)$@$(NBOLD)"..." ; $(TOOLS)gcc
YACC	= $(ECHO) --- Running bison on $(BOLD)$<$(NBOLD)...; bison -d -y
LEX	= $(ECHO) --- Running flex on $(BOLD)$<$(NBOLD)...; flex 

CC	= @$(COMPILE)

all: $(CAN_MODULE) #examples 


# !! should be for all Kernels > 2.2.17 ???
# for each kernel ther is a set of kernel specific headers in 
# /lib/modules/`uname -r`/build/include
#
ifeq "$(findstring 2.4., $(KVERSION))" ""
 INCLUDES = -Isrc
 TEST = Nein
else
 INCLUDES = -Isrc -I/lib/modules/`uname -r`/build/include
 TEST = Ja
endif

# That are the finally used flags for compiling the sources
CFLAGS = -Wall -D__KERNEL__ -DLINUX -O2 -Wstrict-prototypes -fomit-frame-pointer $(DEFS) $(OPTIONS) $(INCLUDES) -DVERSION=\"$(DVERSION)_$(TARGET)\"

VPATH=src
# all the files to be compiled into object code
OBJS	=	\
	    can_core.o		\
	    can_open.o		\
	    can_read.o		\
	    can_write.o		\
	    can_ioctl.o		\
	    can_select.o	\
	    can_close.o		\
	    can_82c200funcs.o	\
	    Can_debug.o		\
	    Can_error.o		\
	    can_util.o		\
	    can_sysctl.o	\




$(CAN_MODULE):  $(addprefix $(OBJDIR)/,$(OBJS)) $(OBJDIR)
	@$(RLINK) -o $@ $(addprefix $(OBJDIR)/,$(OBJS))

$(OBJDIR)/can_core.o: can_core.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_open.o: can_open.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_read.o: can_read.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_write.o: can_write.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_ioctl.o: can_ioctl.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_select.o: can_select.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS)  $(INCLUDES) -o $@ $<
$(OBJDIR)/can_close.o: can_close.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_82c200funcs.o: can_82c200funcs.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_util.o: can_util.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/can_sysctl.o: can_sysctl.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/Can_error.o: Can_error.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/Can_debug.o: Can_debug.c can4linux.h can_defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<


# load host specific CAN configuration
load:
	$(ECHO) ">>> " Loading Driver Module to Kernel
	/sbin/insmod $(CAN_MODULE) 
	@echo "Loading etc/$(CONFIG).conf CAN configuration"
	utils/cansetup etc/$(CONFIG).conf
	echo 0 >/proc/sys/Can/dbgMask

# load host configuration and set dbgMask
dload:
	$(ECHO) ">>> " Loading Driver Module to Kernel
	/sbin/insmod $(CAN_MODULE) 
	@echo "Loading etc/$(CONFIG).conf CAN configuration"
	utils/cansetup etc/$(CONFIG).conf
	echo 7 >/proc/sys/Can/dbgMask
	echo "125 125 125 125" >/proc/sys/Can/Baud


# unload the CAN driver module
unload:
	$(ECHO) ">>> " Removing Driver Module from Kernel
	-/sbin/rmmod $(CAN_MODULE:.o=)


.PHONY:examples
examples:
	(cd examples;make)

clean:
	-rm tags
	-rm obj/*.o
	-rm Can.o
	(cd examples;make clean)

distclean: clean
	cd examples; make clean


inodes:
	-mknod /dev/can0 c $(CAN_MAJOR) 0
	-mknod /dev/can1 c $(CAN_MAJOR) 1
	-mknod /dev/can2 c $(CAN_MAJOR) 2
	-mknod /dev/can3 c $(CAN_MAJOR) 3
	chmod 666 /dev/can[0-3]



ctags:
	ctags --c-types=dtvf src/*.[ch]

############################################################################
#              V e r s i o n  C o n t r o l
#
#
############################################################################
# commit changes of all files to the cvs repository
commit:
	cvs commit -F commitfile

# tag all files in the current module
tag:
	cvs tag $(RELEASE)

#### HTML Manual section. #################################
man:    port_footer.html
	doxygen

showman:
	netscape -raise -remote 'openURL(file:$(PROJECTHOME)/man/html/index.html)'

# Standardfooter für manual pages sollte irgendwo im pms 00340
# stehen, dito das port.gif bild
# 
port_footer.html:       Makefile
	cat ft.html | sed 's/TITLE/$(TITLE)/' \
		    | sed 's/DATE/$(shell date)/' \
		    > $@


archive:	distclean
	tar zcvf can4linux.$(VERSION).$(REL).tgz Makefile Doxyfile README \
	        INSTALL.t2 INSTALL.pdf CHANGELOG \
		etc \
		man \
		src \
		obj \
		examples \
		utils \
		ft.html \
		debug


install:
	-mkdir $(INSTALLHOME)/can4linux
	-mkdir $(INSTALLHOME)/can4linux/obj
	(cd $(INSTALLHOME)/can4linux; ln -s $(PROJECTHOME)/etc .)
	(cd $(INSTALLHOME)/can4linux; ln -s $(PROJECTHOME)/examples .)
	(cd $(INSTALLHOME)/can4linux; ln -s $(PROJECTHOME)/src .)
	(cd $(INSTALLHOME)/can4linux; ln -s $(PROJECTHOME)/utils .)
	cp Makefile $(INSTALLHOME)/can4linux
	(cd $(INSTALLHOME)/can4linux; make)

endif
# Help Text
## 
## make targets:
##  Can.o      - compile the driver sources to Can.o
##  examples   - compile examples in examples directory
##  man        - manual pages using Doxygen
##  archive    - create a *tgz
## 
## only as super user:
##  inodes     - create device entries  in /dev
##  load       - load the driver and use actual "host"-configuration
##  dload      - load with debugMask set to "debug"
##  unload     - unload the CAN driver module
## 
##  istall     - install the driver in /usr/src with references to
##               this actual source tree


.PHONY:help
help:
	@echo -e "\nMakefile for the can4linux CAN driver module"
	@echo "Actual Release Tag is set to RELEASE=$(RELEASE)."
	@sed -n '/^##/s/^## //p' Makefile

