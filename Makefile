ifneq ($(M),)
include $(M)/Make.module
else

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
LINUXSRC := $(shell $(SHELL) linux-info.sh --srcdir)
KVERSION := $(shell $(SHELL) linux-info.sh --kversion)
INCLUDES := -I$(shell $(SHELL) linux-info.sh -I)

# Used release tag for this software version
VERSION=3
REL=1
RELEASE=CAN4LINUX-$(VERSION)_$(REL)
DVERSION=$(VERSION).$(REL)

# be prepared for RTLinux
LINUXTARGET=LINUXOS
#LINUXTARGET=RTLinux


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


TARGET=ATCANMINI_PELICAN

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

ifneq ($DEBUG),NODEBUG)
DEFAULT_DBG_MASK = some
endif
DEFAULT_DBG_MASK = none

ifneq ($(findstring 2.4., $(KVERSION)),)
ifeq "$(LINUXTARGET)" "LINUXOS"
#use normal Linux OS
CAN_MODULE = can.o
endif
 
TOOLS=

ECHO		= /bin/echo

COMPILE	= $(ECHO) "--- Compiling "$<" for $(TARGET) on $(LINUXTARGET) ..." ; \
	  $(TOOLS)gcc
RLINK	= $(ECHO) "--- Linking (relocatable) "$@"..." ;\
	  $(TOOLS)ld -r
LINK	= $(ECHO) "--- Linking "$@"..." ; $(TOOLS)gcc

CC	= $(COMPILE)

all: $(CAN_MODULE)

# That are the finally used flags for compiling the sources
CFLAGS = -O2 -Wall -Wstrict-prototypes -fomit-frame-pointer\
	-D__KERNEL__ -DMODULE\
	-DLINUX\
	 $(DEFS) $(OPTIONS) $(INCLUDES)\
	-DVERSION=\"$(DVERSION)_$(TARGET)\"

M=.
include Make.module

OBJS += linux_2.4.o
else
export \
	TARGET\
	DEBUG\
	CAN_MAJOR\
	DVERSION\

all:
	make -C $(LINUXSRC) 'M=$(CURDIR)' modules

endif

$(CAN_MODULE):  $(OBJS)
	@$(RLINK) -o $@ $(OBJS)

$(OBJS): can4linux.h defs.h ,,sysctl.h


%.o: %.c
	@$(COMPILE) -o $@ -c $(CFLAGS) $(INCLUDES) $<

clean:
	-rm -f ,,sysctl.[ch]
	-rm -f *.o
	-rm -f can.ko can.o
	(cd examples;make clean)

distclean: clean
	cd examples; make clean
	cd trm816; make clean

endif
