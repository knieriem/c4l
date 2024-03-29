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


static void copyusermsg(canmsg_t *dst, void *src)
{
	    __lddk_copy_from_user(dst, src, sizeof(canmsg_t));
}

__LDDK_WRITE_TYPE can_write( __LDDK_WRITE_PARAM )
{
	Dev *dev = filedev(file);
	MsgQ *txq;
	canmsg_t *addr;
	int written;

	txq = &dev->txq;
	DBGin("can_write");
#ifdef DEBUG_COUNTER
	Cnt1[dev->minor] = Cnt1[dev->minor] + 1;
#endif /* DEBUG_COUNTER */

	DBGprint(DBG_DATA,(" -- write %d msg", count));
	addr = (canmsg_t *)buffer;

	if (!access_ok(VERIFY_READ, (canmsg_t *)addr, count * sizeof(canmsg_t))) { 
		DBGout();return -EINVAL;
	}
	for (written=0; written < count; written++) {
		if (!qproduce(txq, copyusermsg, &addr[written])) {
//printk("full %d\n", dev->minor);
			DBGprint(DBG_DATA,("tx buffer full"));
			break;
		}
		if (txq->idle) {
			txq->idle = 0;
//printk("idle %d\n", dev->minor);
			qconsume(txq, sendcanmsg, dev);
		}
	}
	DBGout();
	return written;
}
