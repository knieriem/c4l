/*
 * can_core - can4linux CAN driver module
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
 * $Header: /z2/cvsroot/products/0530/software/can4linux/src/can_core.c,v 1.8 2002/10/25 10:39:25 oe Exp $
 *
 *--------------------------------------------------------------------------
 *
 *
 * modification history
 * --------------------
 * $Log: can_core.c,v $
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
* $Revision: 1.8 $
* $Date: 2002/10/25 10:39:25 $
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
/proc/sys/Can/Chipset:  82c200   82c200  82c200  none
/proc/sys/Can/IOModel:  pppp
/proc/sys/Can/IRQ:      5     7       3       5
/proc/sys/Can/Outc:     250   250     250     0
/proc/sys/Can/Overrun:  0     0       0       0
/proc/sys/Can/RxErr:    0     0       0       0
/proc/sys/Can/Timeout:  100   100     100     100
/proc/sys/Can/TxErr:    0     0       0       0
/proc/sys/Can/TxSpeed:  ffff
/proc/sys/Can/VendOpt:  nnnn
/proc/sys/Can/dbgMask:  0
/proc/sys/Can/version:  2.3
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
name of the used CAN chip for this board.
Read only for this version.
\par IOModel
one letter for each port.
Set the CAN register access model.
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
\par Overrun
counter for overrun conditions in the CAN controller
\par RxErr
counter for CAN controller rx error conditions
\par Timeout
time out value for waiting for a successful transmission
\par TxErr
counter for CAN controller tx error conditions
\par TxSpeed
flag for \b f fast (use tx interrupts) or \b s slow 
\par VendOpt
vendor specific flag for each board. 
Two vendor specific ports are now supported
's' for the STZP(IXXAT) PC I-03
'a' for the  Advantech Pcm-3680
board
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

char kernel_version[] = UTS_RELEASE;

#define LDDK_USE_REGISTER 1



#ifdef LDDK_USE_REGISTER
    int Can_major = Can_MAJOR; 
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

#ifdef ADNP1486

/* Check whether we run on a ADNP1486 (0 if yes) */
static int
test_adnp1486 (void)
{
#define BIOSID_BASE  0x000fe100

  char *biosid, rc = -1;

  biosid = (char*) ioremap (BIOSID_BASE, 16);

  if (biosid)
    {
      int count;
      char mismatch;

      char biosid[] = "ADNP1486";

      for (count = 0, mismatch = 0; count < strlen (biosid); count++)
        {
          if (readb (biosid + count) != biosid[count])
            mismatch = 1;
        }

      if (!mismatch) rc = 0;         /* else this is a ADNP/1486-3V */

      iounmap ((void *) biosid);
    }

  return (rc);
}
#endif

int init_module(void)
{
#if LDDK_USE_BLKREQUEST
  extern int Can_request ();
#endif

  DBGin("init_module");
#ifdef LDDK_USE_REGISTER
      if( register_chrdev(Can_major,"Can",&can_fops) ) {
	  printk("can't get Major %d\n",Can_major);
      return(-EIO);
    }
  

#endif
    {
	printk(__CAN_TYPE__ "CAN Driver " VERSION " (c) " __DATE__  "\n");
	printk(" C.Schroeter (clausi@chemie.fu-berlin.de)\n"); 
	printk(" H.J. Oertel (oe@port.de), H.D. Stich\n");
	    
    }

#if CAN4LINUX_PCI
    /* make some syctl entries read only
     * IRQ number
     * Base address
     * and access mode
     * are fixed and provided by the PCI BIOS
     */
    Can_sysctl_table[SYSCTL_IRQ - 1].mode = 0444;
    Can_sysctl_table[SYSCTL_BASE - 1].mode = 0444;
    Can_sysctl_table[SYSCTL_IOMODEL - 1].mode = 0444;
    /* Can_sysctl_table[SYSCTL_OUTC - 1].mode = 0444; */
    /* printk("CAN pci test loaded\n"); */
    /* dbgMask = 0; */
    pcimod_scan();
#endif

#ifdef ADNP1486
    /* Do we run on a SSV ADNP1486? */
    if (test_adnp1486 () == -1)
      return -EINVAL;
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

#if 0
printk(" Can_pitapci_control[0] = %x\n", Can_pitapci_control[0]);
printk(" Can_pitapci_control[1] = %x\n", Can_pitapci_control[1]);
printk(" Can_pitapci_control[2] = %x\n", Can_pitapci_control[2]);
printk(" Can_pitapci_control[3] = %x\n", Can_pitapci_control[3]);
#endif
    DBGout();
    return 0;
}

void cleanup_module(void)
{
    DBGin("cleanup_module");
    if (MOD_IN_USE) {
      printk("Can : device busy, remove delayed\n");
    }

    /* printk("CAN: removed successfully \n"); */
	

#if LDDK_USE_BLKREQUEST
    blk_dev[Can_major].request_fn = NULL ;
#endif
#ifdef LDDK_USE_REGISTER
    if( unregister_chrdev(Can_major, "Can") != 0 ){
        printk("can't unregister Can, device busy \n");
    } else {
        printk("Can successfully removed\n");
    }

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
