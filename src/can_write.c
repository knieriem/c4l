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
 *------------------------------------------------------------------
 * $Header: /z2/cvsroot/products/0530/software/can4linux/src/can_write.c,v 1.7 2003/07/05 14:28:55 oe Exp $
 *
 *--------------------------------------------------------------------------
 *
 *
 * modification history
 * --------------------
 * $Log: can_write.c,v $
 * Revision 1.7  2003/07/05 14:28:55  oe
 * - all changes for the new 3.0: try to eliminate hw depedencies at run-time.
 *   configure for HW at compile time
 *
 * Revision 1.6  2002/08/20 05:57:22  oe
 * - new write() handling, now not ovrwriting buffer content if buffer fill
 * - ioctl() get status returns buffer information
 *
 * Revision 1.5  2002/08/08 18:05:57  oe
 * *** empty log message ***
 *
 * Revision 1.4  2001/09/14 14:58:09  oe
 * first free release
 *
 * Revision 1.3  2001/09/04 15:51:44  oe
 * changed struct file_operations can_fops
 *
 * Revision 1.2  2001/06/15 15:32:49  oe
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
* \file can_write.c
* \author Heinz-Jürgen Oertel, port GmbH
* $Revision: 1.7 $
* $Date: 2003/07/05 14:28:55 $
*
*/

#include <can_defs.h>

/* \fn size_t can_write( __LDDK_WRITE_PARAM) */
/***************************************************************************/
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

DEBUG_TTY(1, "write: %d", count);
    DBGprint(DBG_DATA,(" -- write %d msg", count));
    /* printk("w[%d/%d]", minor, TxFifo->active); */
    addr = (canmsg_t *)buffer;

    if(verify_area(VERIFY_READ, (canmsg_t *)addr, count * sizeof(canmsg_t))) { 
	    DBGout();return -EINVAL;
    }
    while( written < count ) {
        int check;

	/* enter critical section */
	save_flags(flags);cli();

        /* check mode; mh 2003-11-18 */
        check = CANin(minor,canmode);
        if (check & CAN_RESET_REQUEST) {
		CAN_StopChip(minor);
		printk("CAN: ## check: force reset **\n");
		CAN_StartChip(minor);
        }

	/* there are data to write to the network */
	if(TxFifo->free[TxFifo->head] == BUF_FULL) {
DEBUG_TTY(1, "ret buffer full");
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
DEBUG_TTY(1, " fifo active: id %d/%x; head %d/tail %d",
TxFifo->data[TxFifo->head].id,
TxFifo->data[TxFifo->head].id,
TxFifo->head,
TxFifo->tail
);
	    TxFifo->free[TxFifo->head] = BUF_FULL; /* now this entry is FULL */
	    TxFifo->head = ++(TxFifo->head) % TxFifo->size;
	} else {
	    __lddk_copy_from_user(
		    (canmsg_t *) &tx, 
		    (canmsg_t *) &addr[written],
		    sizeof(canmsg_t) );
DEBUG_TTY(1, " fifo in-active: id %d/%x; head %d/tail %d",
		tx.id, tx.id,
TxFifo->head,
TxFifo->tail

);
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

