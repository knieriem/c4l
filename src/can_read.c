/*
 * can_read - can4linux CAN driver module
 *
 *
 * can4linux -- LINUX CAN device driver source
 * 
 * Copyright (c) 2001 port GmbH Halle/Saale
 * (c) 2001 Heinz-J�rgen Oertel (oe@port.de)
 *          Claus Schroeter (clausi@chemie.fu-berlin.de)
 * derived from the the LDDK can4linux version
 *     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
 *------------------------------------------------------------------
 * $Header: /z2/cvsroot/products/0530/software/can4linux/src/can_read.c,v 1.5 2003/08/27 13:06:27 oe Exp $
 *
 *--------------------------------------------------------------------------
 *
 *
 * modification history
 * --------------------
 * $Log: can_read.c,v $
 * Revision 1.5  2003/08/27 13:06:27  oe
 * - Version 3.0
 *
 * Revision 1.4  2001/09/14 14:58:09  oe
 * first free release
 *
 * Revision 1.3  2001/09/04 15:51:44  oe
 * changed struct file_operations can_fops
 *
 * Revision 1.2  2001/06/15 15:32:45  oe
 * - added PCI support EMS CPC-PCI
 *
 * Revision 1.1.1.1  2001/06/11 18:30:54  oe
 * minimal version can4linux embedded, compile time Konfigurierbar
 *
 *
 *
 *
 *--------------------------------------------------------------------------
 */


/**
* \file can_read.c
* \author Heinz-J�rgen Oertel, port GmbH
* $Revision: 1.5 $
* $Date: 2003/08/27 13:06:27 $
*
* Module Description 
* see Doxygen Doc for all possibilities
*
*
*
*/


/* header of standard C - libraries */

/* header of common types */

/* shared common header */

/* header of project specific types */

/* project headers */
#include <can_defs.h>

/* local header */

/* constant definitions
---------------------------------------------------------------------------*/

/* local defined data types
---------------------------------------------------------------------------*/

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
/* static char _rcsid[] = "$Id: can_read.c,v 1.5 2003/08/27 13:06:27 oe Exp $"; */



/***************************************************************************/
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
	
	if( verify_area( VERIFY_WRITE, buffer, count * sizeof(canmsg_t) )) {
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
