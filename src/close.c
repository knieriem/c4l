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
 *------------------------------------------------------------------
 * $Header: /z2/cvsroot/products/0530/software/can4linux/src/can_close.c,v 1.7 2003/08/27 13:06:26 oe Exp $
 *
 *--------------------------------------------------------------------------
 *
 *
 * modification history
 * --------------------
 * $Log: can_close.c,v $
 * Revision 1.7  2003/08/27 13:06:26  oe
 * - Version 3.0
 *
 * Revision 1.6  2003/07/05 14:28:55  oe
 * - all changes for the new 3.0: try to eliminate hw depedencies at run-time.
 *   configure for HW at compile time
 *
 * Revision 1.5  2002/08/08 17:57:24  oe
 * - at close() use release_mem_region() release_region()
 *
 * Revision 1.4  2001/09/14 14:58:09  oe
 * first free release
 *
 * Revision 1.3  2001/09/04 15:50:57  oe
 * - changed function type
 * - added return value to "int" functions
 *
 * Revision 1.2  2001/06/15 15:31:35  oe
 * - added PCI support EMS CPC-PCI
 *
 * Revision 1.1.1.1  2001/06/11 18:30:54  oe
 * minimal version can4linux embedded, compile time Konfigurierbar
 *
 * Revision 1.1  2001/06/07 08:29:24  oe
 * Initial revision
 *
 *
 *
 *
 *--------------------------------------------------------------------------
 */


/**
* \file can_close.c
* \author Heinz-Jürgen Oertel, port GmbH
* $Revision: 1.7 $
* $Date: 2003/08/27 13:06:26 $
*
*
*/
/*
*
*/
#include <defs.h>

extern int Can_isopen[];   		/* device minor already opened */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

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
    DBGin("can_close");
    {
	unsigned int minor = __LDDK_MINOR;

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

	Can_FifoCleanup(minor);

	Can_FreeIrq(minor, IRQ[minor]);
#if !defined(CONFIG_PPC)
	/* printk("CAN module %d has been closed\n",minor); */
#endif

	if(Can_isopen[minor] > 0) {
	    --Can_isopen[minor];		/* flag device as free */
	    MOD_DEC_USE_COUNT;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,2,0)
	    return 0;
#endif
	}
	
    }
    DBGout();
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,2,0)
    return -EBADF;
#endif
}
