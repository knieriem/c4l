#
# Driver Configurataion
#
# 
# -D CAN_USE_FILTER
#
# -D CAN_RTR_CONFIG
#
# common debug support enabled (loggong to syslogd)
# -DDEBUG=1
# or disabled (not compiled into driver)
# -DDEBUG=0
#
# how to access the hardware
# -DCAN_PORT_IO
#
# debugging I/O acces to the hardware, prints address and value
# -DIODEBUG
#

PATH=/bin:/usr/bin:/usr/local/armtools_glibc/bin/ 
export PATH
TOOLS=arm-uclinux-
TOOLS=/usr/local/armtools_glibc/bin/arm-uclinux-
TOOLS=
DEBUG=DEBUG=1
DEFS = -D$(DEBUG) -DDEFAULT_DEBUG -DCan_MAJOR=63 \
	-DCAN_PORT_IO -DCAN_PELICANMODE

ifeq "$(shell uname -r)" "2.4.4-4GB"
 INCLUDES = -I. -I/lib/modules/`uname -r`/build/include
else
 INCLUDES = -I.
endif

CFLAGS = -Wall -D__KERNEL__ -DLINUX -O2 -Wstrict-prototypes -fomit-frame-pointer $(DEFS) $(OPTIONS) $(INCLUDES)

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

BOLD		= "\033[1m"
NBOLD		= "\033[0m"

ECHO		= /bin/echo -e

COMPILE	= $(ECHO) "--- Compiling "$(BOLD)$<$(NBOLD)"..." ; \
	  $(TOOLS)gcc
DEPEND	= $(ECHO) "--- Checking dependencies..." ; $(TOOLS)$(CPP)
RLINK	= $(ECHO) "--- Linking (relocatable) "$(BOLD)$@$(NBOLD)"..." ;\
	  $(TOOLS)ld -r
LINK	= $(ECHO) "--- Linking "$(BOLD)$@$(NBOLD)"..." ; $(TOOLS)gcc
YACC	= $(ECHO) --- Running bison on $(BOLD)$<$(NBOLD)...; bison -d -y
LEX	= $(ECHO) --- Running flex on $(BOLD)$<$(NBOLD)...; flex 

CC	= @$(COMPILE)
#CC	= $(COMPILE) $(CFLAGS)


all:	Can.o

Can.o: $(OBJS)
	@$(RLINK) -o Can.o $(OBJS)


load:
	$(ECHO) ">>> " Loading Driver Module to Kernel
	/sbin/insmod Can.o 

config:
	../utils/cansetup  ../etc/heinz.conf
unload:
	$(ECHO) ">>> " Removing Driver Module from Kernel
	-/sbin/rmmod Can

clean:
	-rm *.o


ctags:
	ctags -i +d *.c *.h

dist:
	tar zcvf can4linux2.0.tgz Readme Makefile *.c *.h
