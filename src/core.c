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

#include "defs.h"
#include <linux/version.h>

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

struct file_operations can_fops = { 
    llseek:	NULL, 
    read:	can_read,
    write:	can_write,
    readdir:	NULL, 
    poll:	can_select,
    ioctl:	can_ioctl,
    mmap:	NULL, 
    open:	can_open,
    flush:	NULL, /* flush call */
    release:	can_close,
    fsync:	NULL,
    fasync:	NULL,
};


#ifdef CAN_INDEXED_PORT_IO
canregs_t* regbase=0;
#endif

int core_init(void)
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

void core_cleanup(void)
{
#if defined(LDDK_USE_REGISTER) && defined(CONFIG_DEVFS_FS)
  int i;
#endif
  DBGin("cleanup_module");
  if (inuse()) {
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
