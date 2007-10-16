/*
 * can_open - can4linux CAN driver module
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
 * $Header: /z2/cvsroot/products/0530/software/can4linux/src/can_open.c,v 1.7 2003/08/27 13:06:27 oe Exp $
 *
 *--------------------------------------------------------------------------
 *
 *
 * modification history
 * --------------------
 * $Log: can_open.c,v $
 * Revision 1.7  2003/08/27 13:06:27  oe
 * - Version 3.0
 *
 * Revision 1.6  2003/07/05 14:28:55  oe
 * - all changes for the new 3.0: try to eliminate hw depedencies at run-time.
 *   configure for HW at compile time
 *
 * Revision 1.5  2002/08/08 18:03:41  oe
 * *** empty log message ***
 *
 * Revision 1.4  2001/09/14 14:58:09  oe
 * first free release
 *
 * Revision 1.3  2001/09/04 15:51:44  oe
 * changed struct file_operations can_fops
 *
 * Revision 1.2  2001/06/15 15:32:44  oe
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
* \file can_open.c
* \author Heinz-Jürgen Oertel, port GmbH
* $Revision: 1.7 $
* $Date: 2003/08/27 13:06:27 $
*
*
*/


/* header of standard C - libraries */
/* #include <linux/module.h>			 */

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
int Can_isopen[MAX_CHANNELS] = { 0 };   /* device minor already opened */

/* local defined variables
---------------------------------------------------------------------------*/
/* static char _rcsid[] = "$Id: can_open.c,v 1.7 2003/08/27 13:06:27 oe Exp $"; */


/***************************************************************************/
/**
*
* \brief int open(const char *pathname, int flags);
* opens the CAN device for following operations
* \param pathname device pathname, usual /dev/can?
* \param flags is one of \c O_RDONLY, \c O_WRONLY or \c O_RDWR which request
*       opening  the  file  read-only,  write-only  or read/write,
*       respectively.
*
*
* The open call is used to "open" the device.
* Doing a first initialization according the to values in the /proc/sys/Can
* file system.
* Additional an ISR function is assigned to the IRQ.
*
* The CLK OUT pin is configured for creating the same frequency
* like the chips input frequency fclk (XTAL). 
*
* If Vendor Option \a VendOpt is set to 's' the driver performs
* an hardware reset befor initializing the chip.
*
* \returns
* open return the new file descriptor,
* or -1 if an error occurred (in which case, errno is set appropriately).
*
* \par ERRORS
* the following errors can occur
* \arg \c ENXIO  the file is a device special file
* and no corresponding device exists.
* \arg \c EINVAL illegal \b minor device number
* \arg \c EINVAL wrong IO-model format in /proc/sys/Can/IOmodel
* \arg \c EBUSY  IRQ for hardware is not available
* \arg \c EBUSY  I/O region for hardware is not available

*/
int can_open( __LDDK_OPEN_PARAM )
{
int retval = 0;

    DBGin("can_open");
    MOD_INC_USE_COUNT;
    {

	int lasterr;	

	unsigned int minor = __LDDK_MINOR;

	/* Can_dump(minor); */

/* release_mem_region(0xd8000, 0x200 ); */
	if( minor > 3 )
	{
	    printk("CAN: Illegal minor number %d\n", minor);
	    MOD_DEC_USE_COUNT;
	    DBGout();
	    return -EINVAL;
	}
	/* check if device is already open, should be used only by one process */

#ifdef UNSAFE
	/* if(Can_isopen[minor]) { */
	    /* MOD_DEC_USE_COUNT; */
	    /* DBGout(); */
	    /* return -ENXIO; */
	/* } */
	++Can_isopen[minor];		/* flag device in use */
#else
	if(Can_isopen[minor] == 1) {
	    MOD_DEC_USE_COUNT;
	    DBGout();
	    return -ENXIO;
	}
	Can_isopen[minor] = 1;		/* flag device in use */
#endif
	if( Base[minor] == 0x00) {
	    /* No device available */
	    printk("CAN[%d]: no device available\n", minor);
	    MOD_DEC_USE_COUNT;
	    DBGout();
	    return -ENXIO;
	}

	/* the following does all the board specific things
	   also memory remapping if necessary */
	if( (lasterr = CAN_VendorInit(minor)) < 0 ){
	    MOD_DEC_USE_COUNT;
	    DBGout();
	    return lasterr;
	}

	/* Can_dump(minor); */

/* controller_available(curr + 0x400, 4); */
	Can_WaitInit(minor);	/* initialize wait queue for select() */

	if (Can_FifoInit(minor) < 0)
	{
		MOD_DEC_USE_COUNT;
		DBGout();
		return -ENOMEM;
	}

#if CAN_USE_FILTER
	Can_FilterInit(minor);
#endif

	if( CAN_ChipReset(minor) < 0 ) {
	    MOD_DEC_USE_COUNT;
	    DBGout();
	    return -EINVAL;
	}
	CAN_StartChip(minor);
#if DEBUG
    CAN_ShowStat(minor);
#endif
	/* Can_dump(minor); */
    }
    DBGout();
    return retval;
}

