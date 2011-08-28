

CC=powerpc-linux-gcc
CC=gcc

CFLAGS = -Wall -I..
CFLAGS = -Wall -I.. -DUSE_RT_SCHEDULING


all: ctest baud can_send acceptance noiser receive transmit \
	can_verify





# simple applications, using the simple build-in make rule
ctest:		ctest.o
can_send:	can_send.o
can_verify:	can_verify.o
baud:		baud.o
acceptance:	acceptance.o
noiser:		noiser.o
receive:	receive.o
transmit:	transmit.o
transmit2:	transmit2.o



# create a shared object file for TCL
# (tested with SWIG1.1-883, must be installed)
# Test with canLtwo.tcl
canLtwo.so:	canLtwo.c canLtwo.i ../src/can4linux.h
	swig -tcl canLtwo.i
	$(CC) $(CFLAGS) -c -fpic -I/usr/local/include \
	    canLtwo.c canLtwo_wrap.c
	$(CC) -shared canLtwo.o canLtwo_wrap.o -o $@

clean:
	-rm -f *.o can_send receive transmit noiser \
		ctest acceptance baud can_verify \
		#canLtwo.so
