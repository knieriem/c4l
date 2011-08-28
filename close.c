/*
 * can_close - can4linux CAN driver module
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
#include ",,sysctl.h"

extern int Can_isopen[];   		/* device minor already opened */

/***************************************************************************/
/**
*
* \brief int close(int fd);
* close a file descriptor
* \param fd The descriptor to close.
*
* \b close closes a file descriptor, so that it no longer
*  refers to any device and may be reused.
* \returns
* close returns zero on success, or -1 if an error occurred.
* \par ERRORS
*
* the following errors can occur
*
* \arg \c BADF \b fd isn't a valid open file descriptor 
*

*/
__LDDK_CLOSE_TYPE can_close ( __LDDK_CLOSE_PARAM )
{
	Dev *dev = filedev(file);
	int minor = dev->minor;

    DBGin("can_close");
    {
	CAN_StopChip(minor);

        /* since Vx.y (2.4?) macros defined in ioport.h,
           called is  __release_region()  */
#if defined(CAN_PORT_IO) 
	release_region(Base[minor], can_range[minor] );
#else
#if defined(CAN_INDEXED_PORT_IO)
	release_region(Base[minor],2);
#else
# ifndef CAN4LINUX_PCI
	release_mem_region(Base[minor], can_range[minor] );
# endif
#endif
#endif

#ifdef CAN_USE_FILTER
	Can_FilterCleanup(minor);
#endif

	Can_FifoCleanup(dev);

	Can_FreeIrq(dev, IRQ[minor]);
#if !defined(CONFIG_PPC)
	/* printk("CAN module %d has been closed\n",minor); */
#endif

	if(Can_isopen[minor] > 0) {
	    --Can_isopen[minor];		/* flag device as free */
	    decusers();
	    return 0;
	}
	
    }
    DBGout();
    return -EBADF;
}
