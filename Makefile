#
#
# can4linux -- LINUX CAN device driver Makefile
# 
# Copyright (c) 2001/2/3 port GmbH Halle/Saale
# (c) 2001/2/3 Heinz-Jürgen Oertel (oe@port.de)
#
# to compile the can4linux device driver,
# please select some configuration variables.
# eg. the target hardware                       TARGET=
# with or without compiled debug-support        DEBUG= 
#
# Modified and extended to support the esd elctronic system
# design gmbh PC/104-CAN Card (www.esd-electronics.com)
# by Jean-Jacques Tchouto (tchouto@fokus.fraunhofer.de) 
#
# SSV TRM/816 support by Sven Geggus <geggus@iitb.fraunhofer.de>
#

# if available, call this script to get values for KVERSION and INCLUDE
ifeq ($(wildcard linux-info.sh),linux-info.sh)
LINUX_INFO := 1
li_dummy   := $(shell $(SHELL) linux-info.sh)
endif

# Used release tag for this software version
VERSION=3
REL=1
RELEASE=CAN4LINUX-$(VERSION)_$(REL)
DVERSION=$(VERSION).$(REL)

# be prepared for RTLinux
LINUXTARGET=LINUXOS
#LINUXTARGET=RTLinux

ifndef LINUX_INFO
KVERSION= $(shell uname -r)
else
KVERSION= $(shell $(SHELL) linux-info.sh --kversion)
endif
CONFIG := $(shell uname -n)

CTAGS =	ctags --c-types=dtvf 
CTAGS =	elvtags -tsevl

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
## 
## Supported TARGETS= IME_SLIMLINE ATCANMINI_PELICAN CPC_PCI IXXAT_PCI03 
##        PCM3680 PC104_200
## 
## compile DigiTec FC-CAN as ATCANMINI_PELICAN
##
#TARGET=IXXAT_PCI03
#TARGET=IME_SLIMLINE
#TARGET=PCM3680
#TARGET=PC104_200
TARGET=ATCANMINI_PELICAN
#TARGET=CPC_PCI
#TARGET=TRM816


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
# CAN_PORT_IO		- use port I/O instead of memory I/O
# CAN_INDEXED_PORT_IO   - CAN registers adressed by a pair of registers
#			  one is selcting the register the other one does i/O
#			  used eg. on Elan CPUs
# CAN4LINUX_PCI
# IODEBUG               - all register write accesses are logged
# CONFIG_TIME_MEASURE=1 - enable Time measurement at parallel port
#



ifeq "$(TARGET)" "CPC_PCI"
# CPC-PCI PeliCAN  PCI (only with SJA1000) ------------------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR)\
	-DCAN4LINUX_PCI \
	-DCAN_SYSCLK=8

	#-DIODEBUG

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "ATCANMINI_PELICAN"
# AT-CAN-MINI PeliCAN ISA (only with SJA1000) --------------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_PORT_IO \
	-DCAN_SYSCLK=8
	#-DCONFIG_TIME_MEASURE=1

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "IXXAT_PCI03"
# IXXAT PC-I 03 board ISA (only with SJA1000) ---------------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_SYSCLK=8

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "PCM3680"
# Advantech PCM3680 board ISA (only with SJA1000) ----------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_SYSCLK=8

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "TRM816"
# TRM816 Onboard CAN-Controller (only with SJA1000) --------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_INDEXED_PORT_IO \
	-DCAN_SYSCLK=10

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "PC104_200"
# ESD PC104-200 PC104 board (with SJA1000) ----------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_PORT_IO -DPC104 \
	-DCAN_SYSCLK=8

TARGET_MATCHED = true
endif

ifeq "$(TARGET)" "IME_SLIMLINE"
# I+ME  PcSlimline ISA (only with SJA1000) -----------------------------------
DEFS =  -D$(TARGET) -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=$(CAN_MAJOR) \
	-DCAN_SYSCLK=8

TARGET_MATCHED = true
endif

ifeq "$(LINUXTARGET)" "LINUXOS"
#use normal Linux OS
LIBS   =
CAN_MODULE = Can.o
PROJECTHOME=$(shell pwd)
INSTALLHOME=/usr/src
endif
 


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
	@echo "select one of: ATCANMINI_PELICAN, CPC_PCI, IME_SLIMLINE, IXXAT_PCI03, PCM3680, PC104_200, TRM816"
else
###########################################################################
# select the compiler toolchain
###########################################################################
TOOLS=powerpc-linux-gcc 
TOOLS=arm-uclinux-
TOOLS=/usr/local/armtools_glibc/bin/arm-uclinux-
TOOLS=

ECHO		= /bin/echo

COMPILE	= $(ECHO) "--- Compiling "$<" for $(TARGET) on $(LINUXTARGET) ..." ; \
	  $(TOOLS)gcc
DEPEND	= $(ECHO) "--- Checking dependencies..." ; $(TOOLS)$(CPP)
RLINK	= $(ECHO) "--- Linking (relocatable) "$@"..." ;\
	  $(TOOLS)ld -r
LINK	= $(ECHO) "--- Linking "$@"..." ; $(TOOLS)gcc
YACC	= $(ECHO) --- Running bison on $<...; bison -d -y
LEX	= $(ECHO) --- Running flex on $<...; flex 

CC	= $(COMPILE)

all: $(CAN_MODULE)


# !! should be for all Kernels > 2.2.17 ???
# for each kernel ther is a set of kernel specific headers in 
# /lib/modules/`uname -r`/build/include
#
ifeq "$(LINUXTARGET)" "LINUXOS"
ifeq "$(findstring 2.4., $(KVERSION))" ""
 INCLUDES = -Isrc
 TEST = Nein
else
  ifndef LINUX_INFO
 INCLUDES = -Isrc -I/lib/modules/`uname -r`/build/include
  else
 INCLUDES = -Isrc -I$(shell $(SHELL) linux-info.sh -I)
  endif
 #INCLUDES = -Isrc -I/home/geg/kernel/linux-2.4.22-586/include
 TEST = Ja
endif
endif

# That are the finally used flags for compiling the sources
CFLAGS = -Wall -D__KERNEL__ -DLINUX -O2 -Wstrict-prototypes -fomit-frame-pointer $(DEFS) $(OPTIONS) $(INCLUDES) -DVERSION=\"$(DVERSION)_$(TARGET)\"

VPATH=src
# all the files to be compiled into object code
OBJS	=	\
	    core.o\
	    open.o\
	    read.o\
	    write.o\
	    ioctl.o\
	    select.o\
	    close.o\
	    debug.o\
	    error.o\
	    util.o\
	    sysctl.o\

# include Chip specific object files
ifeq "$(TARGET)" "CPC_PCI"
OBJS += sja1000funcs.o
endif
ifeq "$(TARGET)" "ATCANMINI_PELICAN"
OBJS += sja1000funcs.o
endif
ifeq "$(TARGET)" "IXXAT_PCI03"
OBJS += sja1000funcs.o
endif
ifeq "$(TARGET)" "PCM3680"
OBJS += sja1000funcs.o
endif
ifeq "$(TARGET)" "TRM816"
OBJS += sja1000funcs.o
endif
ifeq "$(TARGET)" "PC104_200"
OBJS += mcf5282funcs.o
endif
ifeq "$(TARGET)" "IME_SLIMLINE"
OBJS += sja1000funcs.o
endif


$(CAN_MODULE):  $(addprefix $(OBJDIR)/,$(OBJS)) $(OBJDIR)
	@$(RLINK) -o $@ $(addprefix $(OBJDIR)/,$(OBJS))

$(OBJDIR)/core.o: core.c can4linux.h defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/open.o: open.c can4linux.h defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/read.o: read.c can4linux.h defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/write.o: write.c can4linux.h defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/ioctl.o: ioctl.c can4linux.h defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/select.o: select.c can4linux.h defs.h
	@$(COMPILE) -c $(CFLAGS)  $(INCLUDES) -o $@ $<
$(OBJDIR)/close.o: close.c can4linux.h defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/sja1000funcs.o: sja1000funcs.c can4linux.h defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/util.o: util.c can4linux.h defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/sysctl.o: sysctl.c can4linux.h defs.h $(OBJDIR)/vcs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -I obj -o $@ $<
$(OBJDIR)/error.o: error.c can4linux.h defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(OBJDIR)/debug.o: debug.c can4linux.h defs.h
	@$(COMPILE) -c $(CFLAGS) $(INCLUDES) -o $@ $<

$(OBJDIR)/vcs.h:
	id=`hg id -i`; \
	hg log -l1 --template '#define VCS_REV_STRING "{date|shortdate} '$$id'"\n' > $@

clean:
	-rm -f tags
	-rm -f obj/vcs.h
	-rm -f obj/*.o
	-rm -f Can.o
	(cd examples;make clean)

distclean: clean
	cd examples; make clean
	cd trm816; make clean

endif
