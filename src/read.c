/*
 * can_read - can4linux CAN driver module
 *
 *
 * can4linux -- LINUX CAN device driver source
 * 
 * Copyright (c) 2001 port GmbH Halle/Saale
 * (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
 *          Claus Schroeter (clausi@chemie.fu-berlin.de)
 * derived from the the LDDK can4linux version
 *     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
 */

#include "defs.h"

/**
*
* \brief ssize_t read(int fd, void *buf, size_t count);
* the read system call
* \param fd The descriptor to read from.
* \param buf The destination data buffer (array of CAN canmsg_t).
* \param count The number of bytes to read.
*
* read() attempts to read up to \a count CAN messages from file
* descriptor fd into the buffer starting at buf.
* buf must be large enough to hold count times the size of 
* one CAN message structure \b canmsg_t.
* 
* \code
int got;
canmsg_t rx[80];			// receive buffer for read()

    got = read(can_fd, rx , 80 );
    if( got > 0) {
      ...
    } else {
	// read returned with error
	fprintf(stderr, "- Received got = %d\n", got);
	fflush(stderr);
    }


* \endcode
*
* \par ERRORS
*
* the following errors can occur
*
* \arg \c EINVAL \b buf points not to an large enough area,
*
* \returns
* On success, the number of bytes read is returned
* (zero indicates end of file).
* It is not an  error if this number is
* smaller than the number of bytes requested;
* this may happen for example
* because fewer bytes are actually available right now,
* or because read() was interrupted by a signal.
* On error, -1 is returned, and errno is set  appropriately.
*
* \internal
*/

int can_read( __LDDK_READ_PARAM )
{
  int retval=0;

  DBGin("can_read");
  {
	unsigned int minor = __LDDK_MINOR;
	msg_fifo_t *RxFifo = &Rx_Buf[minor];
	canmsg_t *addr; 
	int written = 0;

	addr = (canmsg_t *)buffer;
	
	if( !access_ok( VERIFY_WRITE, buffer, count * sizeof(canmsg_t) )) {
	   DBGout();
	   return -EINVAL;
	}
	while( written < count && RxFifo->status == BUF_OK ) {
	    if( RxFifo->tail == RxFifo->head ) {
		RxFifo->status = BUF_EMPTY;
		break;
	    }	

	    __lddk_copy_to_user( (canmsg_t *) &(addr[written]), 
			(canmsg_t *) &(RxFifo->data[RxFifo->tail]),
			sizeof(canmsg_t) );
	    written++;
	    RxFifo->tail = ++(RxFifo->tail) % RxFifo->size;
	}
	DBGout();
	return(written);
	
    } 
    DBGout();
    return retval;
}
