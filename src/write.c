/*
 * can_write - can4linux CAN driver module
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

\brief size_t write(int fd, const char *buf, size_t count);
write CAN messages to the network
\param fd The descriptor to write to.
\param buf The data buffer to write (array of CAN canmsg_t).
\param count The number of bytes to write.

write  writes  up to \a count CAN messages to the CAN controller
referenced by the file descriptor fd from the buffer
starting at buf.



\par Errors

the following errors can occur

\li \c EBADF  fd is not a valid file descriptor or is not open
              for writing.

\li \c EINVAL fd is attached to an object which is unsuitable for
              writing.

\li \c EFAULT buf is outside your accessible address space.

\li \c EINTR  The call was interrupted by a signal before any
              data was written.



\returns
On success, the number of CAN messages written are returned
(zero indicates nothing was written).
On error, -1 is returned, and errno is set appropriately.

\internal
*/

__LDDK_WRITE_TYPE can_write( __LDDK_WRITE_PARAM )
{
unsigned int minor = __LDDK_MINOR;
msg_fifo_t *TxFifo = &Tx_Buf[minor];
canmsg_t *addr;
canmsg_t tx;
unsigned long flags;
int written        = 0;

    DBGin("can_write");
#ifdef DEBUG_COUNTER
    Cnt1[minor] = Cnt1[minor] + 1;
#endif /* DEBUG_COUNTER */

    DBGprint(DBG_DATA,(" -- write %d msg", count));
    /* printk("w[%d/%d]", minor, TxFifo->active); */
    addr = (canmsg_t *)buffer;

    if(!access_ok(VERIFY_READ, (canmsg_t *)addr, count * sizeof(canmsg_t))) { 
	    DBGout();return -EINVAL;
    }
    while( written < count ) {
	/* enter critical section */
	save_flags(flags);cli();
	/* there are data to write to the network */
	if(TxFifo->free[TxFifo->head] == BUF_FULL) {
	    DBGprint(DBG_DATA,("ret buffer full"));
	    /* there is already one message at this place */;
	    DBGout();
	    /* return -ENOSPC; */
	    return written;
	}
	if( TxFifo->active ) {
	    /* more than one data and actual data in queue */
	    __lddk_copy_from_user(	/* copy one message to FIFO */
		    (canmsg_t *) &(TxFifo->data[TxFifo->head]), 
		    (canmsg_t *) &addr[written],
		    sizeof(canmsg_t) );
	    DBGprint(DBG_DATA,("fifo active: id %ld/%lx; head %d/tail %d",
TxFifo->data[TxFifo->head].id,
TxFifo->data[TxFifo->head].id,
TxFifo->head,
TxFifo->tail
));
	    TxFifo->free[TxFifo->head] = BUF_FULL; /* now this entry is FULL */
	    TxFifo->head = ++(TxFifo->head) % TxFifo->size;
	} else {
	    __lddk_copy_from_user(
		    (canmsg_t *) &tx, 
		    (canmsg_t *) &addr[written],
		    sizeof(canmsg_t) );
	    DBGprint(DBG_DATA,("fifo in-active: id %ld/%lx; head %d/tail %d",
		tx.id, tx.id,
TxFifo->head,
TxFifo->tail
));
	  /* f - fast -- use interrupts */
	  if( count >= 1 ) {
	    /* !!! CHIP abh. !!! */
	    TxFifo->active = 1;
	  }
	  CAN_SendMessage( minor, &tx);  /* Send, no wait */
	}
        written++;
	/* leave critical section */
	restore_flags(flags);
    }
    /* printk("W[%d/%d]", minor, TxFifo->active); */
    DBGout();
    return written;
}
