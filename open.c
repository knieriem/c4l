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
 */

#include "defs.h"

int Can_isopen[MAX_CHANNELS] = { 0 };   /* device minor already opened */

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
    incusers();
    {

	int lasterr;	

	unsigned int minor = __LDDK_MINOR;

	/* Can_dump(minor); */

/* release_mem_region(0xd8000, 0x200 ); */
	if( minor > 3 )
	{
	    printk("CAN: Illegal minor number %d\n", minor);
	    decusers();
	    DBGout();
	    return -EINVAL;
	}
	/* check if device is already open, should be used only by one process */

#ifdef UNSAFE
	/* if(Can_isopen[minor]) { */
	    /* decusers(); */
	    /* DBGout(); */
	    /* return -ENXIO; */
	/* } */
	++Can_isopen[minor];		/* flag device in use */
#else
	if(Can_isopen[minor] == 1) {
	    decusers();
	    DBGout();
	    return -ENXIO;
	}
	Can_isopen[minor] = 1;		/* flag device in use */
#endif
	if( Base[minor] == 0x00) {
	    /* No device available */
	    printk("CAN[%d]: no device available\n", minor);
	    decusers();
	    DBGout();
	    return -ENXIO;
	}

	/* the following does all the board specific things
	   also memory remapping if necessary */
	if( (lasterr = CAN_VendorInit(minor)) < 0 ){
	    decusers();
	    DBGout();
	    return lasterr;
	}

	/* Can_dump(minor); */

/* controller_available(curr + 0x400, 4); */
	Can_WaitInit(minor);	/* initialize wait queue for select() */

	if (Can_FifoInit(minor) < 0)
	{
		decusers();
		DBGout();
		return -ENOMEM;
	}

#if CAN_USE_FILTER
	Can_FilterInit(minor);
#endif

	if( CAN_ChipReset(minor) < 0 ) {
	    decusers();
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
