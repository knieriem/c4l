/*
 * can_core - can4linux CAN driver module
set tagprg="global -t $1"
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
 * $Header: /z2/cvsroot/products/0530/software/can4linux/src/can_core.c,v 1.10 2003/08/27 13:06:26 oe Exp $
 *
 *--------------------------------------------------------------------------
 *
 *
 * modification history
 * --------------------
 * $Log: can_core.c,v $
 * Revision 1.10  2003/08/27 13:06:26  oe
 * - Version 3.0
 *
 * Revision 1.9  2003/07/05 14:28:55  oe
 * - all changes for the new 3.0: try to eliminate hw depedencies at run-time.
 *   configure for HW at compile time
 *
 * Revision 1.8  2002/10/25 10:39:25  oe
 * - vendor specific handling for Advantech board added by "R.R.Robotica" <rrrobot@tin.it>
 *
 * Revision 1.7  2002/10/11 16:58:06  oe
 * - IOModel, Outc, VendOpt are now set at compile time
 * - deleted one misleading printk()
 *
 * Revision 1.6  2002/08/08 17:59:38  oe
 * *** empty log message ***
 *
 * Revision 1.5  2001/11/20 16:43:29  oe
 * *** empty log message ***
 *
 * Revision 1.4  2001/09/14 14:58:09  oe
 * first free release
 *
 * Revision 1.3  2001/09/04 15:51:44  oe
 * changed struct file_operations can_fops
 *
 * Revision 1.2  2001/06/15 15:32:10  oe
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
* \file can_core.c
* \author Heinz-Jürgen Oertel, port GmbH
* $Revision: 1.10 $
* $Date: 2003/08/27 13:06:26 $
*
* Contains the code for module initialization.
* The functions herein are never called directly by the user
* but when the driver module is loaded into the kernel space
* or unloaded.
*
* The driver is highly configurable using the \b sysctl interface.
* For a description see main page.

*/


/****************************************************************************/
/**
* \mainpage  can4linx - CAN network device driver
*
The LINUX CAN driver can be used to control the CAN bus
connected with the  AT-CAN-MINI  PC interface card.
This project was done in cooperation with the  LINUX LLP Project
to control laboratory or automation devices via CAN.

The full featured can4linux supports many different interface boards.
It is possible to use different kinds of boards at the same time.
Up to four boards can be placed in one computer.
With this feature it is possible to use /dev/can0 and
/dev/can2 for two boards AT-CAN-MINI with SJA1000
and /dev/can1 and /dev/can3 with two CPC-XT equipped with Intel 82527.
In all these configurations
the programmer sees the same driver interface with
open(), close(), read(), write() and ioctl() calls
( can_open(), can_close(), can_read(), can_write(), can_ioctl() ).

The driver itself is highly configurable
using the /proc interface of the LINUX kernel. 

The following listing shows a typical configuration with three boards: 

\code
$ grep . /proc/sys/Can/\*
/proc/sys/Can/AccCode:  -1       -1      -1      -1
/proc/sys/Can/AccMask:  -1       -1      -1      -1
/proc/sys/Can/Base:     800      672     832     896
/proc/sys/Can/Baud:     125      125     125     250
/proc/sys/Can/Chipset:  SJA1000
/proc/sys/Can/IOModel:  pppp
/proc/sys/Can/IRQ:      5     7       3       5
/proc/sys/Can/Outc:     250   250     250     0
/proc/sys/Can/Overrun:  0     0       0       0
/proc/sys/Can/RxErr:    0     0       0       0
/proc/sys/Can/Timeout:  100   100     100     100
/proc/sys/Can/TxErr:    0     0       0       0
/proc/sys/Can/dbgMask:  0
/proc/sys/Can/version:  3.0_ATCANMINI_PELICAN
\endcode


This above mentioned full flexibility
is not needed in embedded applications.
For this applications, a stripped-down version exists.
It uses the same programming interface
but does the most configurations at compile time.
That means especially that only one CAN controller support with
a special register access method is compiled into the driver.
Actually the only CAN controller supported by this version
is the Philips SJA 1000 in both the compatibility mode 
\b BasicCAN and the Philips \PeliCAN mode (compile time selectable).

The version of can4linux currently available at the uClinux CVS tree
is also supporting the Motorola FlexCAN module as ist is implemented
on Motorolas ColdFire 5282 CPU.



The following sections are describing the \e sysctl entries.

\par AccCode/AccMask
contents of the message acceptance mask and acceptance code registers
of 82x200/SJA1000 compatible CAN controllers (see can_ioctl()).

\par Base
CAN controllers base address for each board.
Depending of the \e IOModel entry that can be a memory or I/O address.
(read-only for PCI boards)
\par Baud
used bit rate for this board
\par Chipset
name of the supported CAN chip used with this boards
Read only for this version.
\par IOModel
one letter for each port. Readonly.
Read the CAN register access model.
The following models are currently supported:
\li m - memory access, the registers are directly mapped into memory
\li f - fast register access, special mode for the 82527
     uses memory locations for register addresses 
     (ELIMA)
\li p - port I/O,  80x86 specific I/O address range
     (AT-CAN-MINI)
\li b - special mode for the B&R CAN card,
     two special I/O addresses for register addressing and access
Since version 2.4 set at compile time.
\par IRQ
used IRQ numbers, one value for each board.
(read-only for PCI boards)
\par Outc
value of the output control register of the CAN controller
Since version 2.4 set at compile time.
A board specific value is used when the module the first time is loaded.
This board specific value can be reloded by writing the value 0
to \e Outc .
\par Overrun
counter for overrun conditions in the CAN controller
\par RxErr
counter for CAN controller rx error conditions
\par Timeout
time out value for waiting for a successful transmission

\par TxErr
counter for CAN controller tx error conditions

\par dbgMask
if compiled with debugging support, writing a value greater then 0 
enables debugging to \b syslogd

\par version
read only entry containing the drivers version number


Please see also at can_ioctl() for some additional descriptions.

For initially writing these sysctl entries after loading the driver
(or at any time) a shell script utility does exist.
It uses a board configuration file that is written over \e /proc/sys/Can .
\code
utils/cansetup port.conf
\endcode
or, like used in the Makefile:
\code
CONFIG := $(shell uname -n)

# load host specific CAN configuration
load:
	@echo "Loading etc/$(CONFIG).conf CAN configuration"
	utils/cansetup etc/$(CONFIG).conf
	echo 0 >/proc/sys/Can/dbgMask
\endcode
Example *.conf files are located in the \e etc/ directory.

\note
This documentation was created using the wonderful tool
\b Doxygen http://www.doxygen.org/index.html .
Die Dokumentation wurde unter Verwendung von
\b Doxygen http://www.doxygen.org/index.html
erstellt

*/

#include <can_defs.h>
#include <linux/version.h>


#ifndef DOXYGEN_SHOULD_SKIP_THIS



MODULE_LICENSE("GPL");
MODULE_AUTHOR("H.-J.Oertel <oe@port.de>");
MODULE_DESCRIPTION("CAN fieldbus driver");


char kernel_version[] = UTS_RELEASE;

int IRQ_requested[MAX_CHANNELS];
int Can_minors[MAX_CHANNELS];			/* used as IRQ dev_id */
#define LDDK_USE_REGISTER 1
#ifdef LDDK_USE_REGISTER
    int Can_major = Can_MAJOR; 
#ifdef CONFIG_DEVFS_FS
devfs_handle_t can_dev_handle[MAX_CHANNELS];
static char devname[MAX_CHANNELS];
#endif
#endif

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

struct file_operations can_fops = { 
#if LINUX_VERSION_CODE >= 0x020201 
    llseek:	NULL, 
#else
    lseek:	NULL, 
#endif
    read:	can_read,
    write:	can_write,
    readdir:	NULL, 
#if LINUX_VERSION_CODE >= 0x020203 
    poll:	can_select,
#else
    select:	can_select,
#endif 
    ioctl:	can_ioctl,
    mmap:	NULL, 
    open:	can_open,
#if LINUX_VERSION_CODE >= 0x020203
    flush:	NULL, /* flush call */
#endif 
    release:	can_close,
    fsync:	NULL,
    fasync:	NULL,
};


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef CAN_INDEXED_PORT_IO
canregs_t* regbase=0;
#endif

int init_module(void)
{
int i;
#if LDDK_USE_BLKREQUEST
  extern int Can_request ();
#endif

  DBGin("init_module");
#ifdef LDDK_USE_REGISTER
#ifdef CONFIG_DEVFS_FS
    /* If we have devfs, create /dev/canX to put files in there */
    for (i=0; i < MAX_CHANNELS; i++) {
      sprintf(devname, "can%i", i);
      can_dev_handle[i]=devfs_register(NULL, devname,
		     0,
		     Can_major, i, S_IFCHR | S_IRUGO | S_IWUGO,
		     &can_fops,
		     NULL);

    }
    devfs_register_chrdev(Can_major, "Can", &can_fops);
#else /* no devfs, do it the "classic" way  */
      if( register_chrdev(Can_major, "Can", &can_fops) ) {
	  printk("can't get Major %d\n", Can_major);
      return(-EIO);
    }
#endif
#endif
    {
	printk(__CAN_TYPE__ "CAN Driver " VERSION " (c) " __DATE__  "\n");
	printk(" H.J. Oertel (oe@port.de)\n");
	printk(" C.Schroeter (clausi@chemie.fu-berlin.de), H.D. Stich\n"); 
	    
    }

    /*
    initialize the variables layed down in /proc/sys/Can
    */
    for (i = 0; i < MAX_CHANNELS; i++) {
	IOModel[i]       = IO_MODEL;
	Baud[i]          = 125;
	AccCode[i]       = AccMask[i] =  0xffffffff;
	Timeout[i]       = 100;
	Outc[i]          = CAN_OUTC_VAL;
	IRQ_requested[i] = 0;
	Can_minors[i]    = i;		/* used as IRQ dev_id */

    }
    IOModel[i] = '\0';


#if CAN4LINUX_PCI
    /* make some syctl entries read only
     * IRQ number
     * Base address
     * and access mode
     * are fixed and provided by the PCI BIOS
     */
    Can_sysctl_table[SYSCTL_IRQ - 1].mode = 0444;
    Can_sysctl_table[SYSCTL_BASE - 1].mode = 0444;
    /* printk("CAN pci test loaded\n"); */
    /* dbgMask = 0; */
    pcimod_scan();
#endif

#if LDDK_USE_PROCINFO
    register_procinfo();
#endif
#if LDDK_USE_SYSCTL
    register_systables();
#endif

#if LDDK_USE_BLKREQUEST
    blk_dev[Can_major].request_fn = Can_request ;
#endif

    DBGout();
    return 0;
}

void cleanup_module(void)
{
#ifdef CONFIG_DEVFS_FS
  int i;
#endif
  DBGin("cleanup_module");
  if (MOD_IN_USE) {
    printk("Can : device busy, remove delayed\n");
  }

  /* printk("CAN: removed successfully \n"); */
	

#if LDDK_USE_BLKREQUEST
    blk_dev[Can_major].request_fn = NULL ;
#endif
#ifdef LDDK_USE_REGISTER
#ifndef CONFIG_DEVFS_FS
    if( unregister_chrdev(Can_major, "Can") != 0 ){
        printk("can't unregister Can, device busy \n");
    } else {
        printk("Can successfully removed\n");
    }
#else
    for (i = 0; i < MAX_CHANNELS; i++) {
      devfs_unregister(can_dev_handle[i]);
    }
    devfs_unregister_chrdev(Can_major, "Can");
#endif
#endif
#if LDDK_USE_PROCINFO
    unregister_procinfo();
#endif
#if LDDK_USE_SYSCTL
    unregister_systables();
#endif
    DBGout();
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

