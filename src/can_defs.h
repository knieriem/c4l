/*
 * can_read - can4linux CAN driver module
 *
 * Copyright (c) 2001 port GmbH Halle/Saale
 *------------------------------------------------------------------
 * $Header: /z2/cvsroot/products/0530/software/can4linux/src/can_defs.h,v 1.10 2002/12/01 17:24:22 oe Exp $
 *
 *--------------------------------------------------------------------------
 *
 *
 * modification history
 * --------------------
 * $Log: can_defs.h,v $
 * Revision 1.10  2002/12/01 17:24:22  oe
 * - data type of Can_timeout
 *
 * Revision 1.9  2002/12/01 17:01:01  oe
 * - corrected wrong index definitions into Sysctl array.
 *   (very strange, could bring the system into nirwana)
 *
 * Revision 1.8  2002/10/11 16:58:06  oe
 * - IOModel, Outc, VendOpt are now set at compile time
 * - deleted one misleading printk()
 *
 * Revision 1.7  2002/08/20 05:57:22  oe
 * - new write() handling, now not ovrwriting buffer content if buffer fill
 * - ioctl() get status returns buffer information
 *
 * Revision 1.6  2002/08/08 18:00:40  oe
 * adapted to new kernel: include linux/malloc.h --> linux/slab.h
 *
 * Revision 1.5  2002/01/10 19:13:19  oe
 * - application header file changed name can.h -> can4linux.h
 *
 * Revision 1.4  2001/09/14 14:58:09  oe
 * first free release
 *
 * Revision 1.3  2001/09/04 15:51:44  oe
 * changed struct file_operations can_fops
 *
 * Revision 1.2  2001/06/15 15:32:43  oe
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
* \file can_defs.h
* \author Name, port GmbH
* $Revision: 1.10 $
* $Date: 2002/12/01 17:24:22 $
*
* Module Desription 
* see Doxygen Doc for all possibilites
*
*
*
*/


#ifndef NOMODULE
#define __NO_VERSION__
#define MODULE
#endif
#ifdef __KERNEL__
#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

#ifdef __KERNEL__
#include <linux/module.h>


#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/errno.h>
#include <linux/major.h>

#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,12)
# include <linux/slab.h>
#else
# include <linux/malloc.h>
#endif

#include <linux/wrapper.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,1,0)
#include <linux/poll.h>
#endif

#include <asm/io.h>
#include <asm/segment.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/dma.h>

#include <linux/mm.h>
#include <linux/signal.h>
#include <linux/timer.h>


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,1,0)
#include <asm/uaccess.h>

#define __lddk_copy_from_user(a,b,c) copy_from_user(a,b,c)
#define __lddk_copy_to_user(a,b,c) copy_to_user(a,b,c)

#else

/* #define __lddk_copy_from_user(a,b,c) memcpy_fromio(a,b,c) */
/* #define __lddk_copy_to_user(a,b,c) memcpy_toio(a,b,c) */
#define __lddk_copy_from_user(a,b,c) memcpy_fromfs(a,b,c)
#define __lddk_copy_to_user(a,b,c) memcpy_tofs(a,b,c)


#include <linux/sched.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,4)
#include <linux/ioport.h>
#endif
#include <linux/ioport.h>

#endif

#if CAN4LINUX_PCI
# define _BUS_TYPE "PCI-"
/* the only one supported: EMS CPC-PCI */
# define PCI_VENDOR 0x110a
# define PCI_DEVICE 0x2104

#else
# define _BUS_TYPE "ISA-"
#endif

#if 1 
#ifdef CAN_PELICANMODE

# ifdef  CAN_PORT_IO
#  define __CAN_TYPE__ _BUS_TYPE "PeliCAN-port I/O "
# else
#  define __CAN_TYPE__ _BUS_TYPE "PeliCAN-memory mapped "
# endif

#else

# ifdef  CAN_PORT_IO
#  define __CAN_TYPE__ _BUS_TYPE "Philips-Basic-CAN port I/O "
# else
#  define __CAN_TYPE__ _BUS_TYPE "Philips-Basic-CAN memory mapped "
# endif

#endif
#endif

/* Length of the "version" strin entry in /proc/.../version */
#define PROC_VER_LENGTH 30 

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
/* kernels higher 2.3.x have a f****** new kernel interface ******************/
#define __LDDK_WRITE_TYPE	ssize_t
#define __LDDK_CLOSE_TYPE	int
#define __LDDK_READ_TYPE	ssize_t
#define __LDDK_OPEN_TYPE	int
#define __LDDK_IOCTL_TYPE	int
#define __LDDK_SELECT_TYPE	unsigned int

#define __LDDK_SEEK_PARAM 	struct file *file, loff_t off, size_t count
#define __LDDK_READ_PARAM 	struct file *file, char *buffer, size_t count, loff_t *loff
#define __LDDK_WRITE_PARAM 	struct file *file, const char *buffer, size_t count, loff_t *loff
#define __LDDK_READDIR_PARAM 	struct file *file, void *dirent, filldir_t count
#define __LDDK_SELECT_PARAM 	struct file *file, struct poll_table_struct *wait
#define __LDDK_IOCTL_PARAM 	struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg
#define __LDDK_MMAP_PARAM 	struct file *file, struct vm_area_struct * vma
#define __LDDK_OPEN_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_FLUSH_PARAM	struct file *file 
#define __LDDK_CLOSE_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_FSYNC_PARAM 	struct file *file, struct dentry *dentry, int datasync
#define __LDDK_FASYNC_PARAM 	struct file *file, int count 
#define __LDDK_CCHECK_PARAM 	kdev_t dev
#define __LDDK_REVAL_PARAM 	kdev_t dev

#define __LDDK_MINOR MINOR(file->f_dentry->d_inode->i_rdev)
#define __LDDK_INO_MINOR MINOR(inode->i_rdev)

#ifndef SLOW_DOWN_IO
# define SLOW_DOWN_IO __SLOW_DOWN_IO
#endif


#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0)
/* kernels higher 2.2.x have a f****** new kernel interface ******************/
#define __LDDK_READ_TYPE	ssize_t
#define __LDDK_WRITE_TYPE	ssize_t
#define __LDDK_SELECT_TYPE	unsigned int
#define __LDDK_IOCTL_TYPE	int
#define __LDDK_OPEN_TYPE	int
#define __LDDK_CLOSE_TYPE	int

#define __LDDK_SEEK_PARAM 	struct file *file, loff_t off, int count
#define __LDDK_READ_PARAM 	struct file *file, char *buffer, size_t count, loff_t *loff
#define __LDDK_WRITE_PARAM 	struct file *file, const char *buffer, size_t count, loff_t *loff
#define __LDDK_READDIR_PARAM 	struct file *file, struct dirent *dirent, int count
#define __LDDK_SELECT_PARAM 	struct file *file, poll_table *wait
#define __LDDK_IOCTL_PARAM 	struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg
#define __LDDK_MMAP_PARAM 	struct file *file, struct vm_area_struct * vma
#define __LDDK_OPEN_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_FLUSH_PARAM	struct file *file 
#define __LDDK_CLOSE_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_FSYNC_PARAM 	struct file *file
#define __LDDK_FASYNC_PARAM 	struct file *file, int count 
#define __LDDK_CCHECK_PARAM 	kdev_t dev
#define __LDDK_REVAL_PARAM 	kdev_t dev

#define __LDDK_MINOR MINOR(file->f_dentry->d_inode->i_rdev)
#define __LDDK_INO_MINOR MINOR(inode->i_rdev)

#ifndef SLOW_DOWN_IO
# define SLOW_DOWN_IO __SLOW_DOWN_IO
#endif

#else
/* kernels < 2.3.x **********************************************************/
 /* 2.0.x */
#define __LDDK_WRITE_TYPE	int
#define __LDDK_CLOSE_TYPE	void
#define __LDDK_READ_TYPE
#define __LDDK_OPEN_TYPE
#define __LDDK_IOCTL_TYPE
#define __LDDK_SELECT_TYPE	int

#define __LDDK_SEEK_PARAM 	struct inode *inode, struct file *file, off_t off, int count
#define __LDDK_READ_PARAM 	struct inode *inode, struct file *file, char *buffer, int count
#define __LDDK_WRITE_PARAM 	struct inode *inode, struct file *file, const char *buffer, int count
#define __LDDK_READDIR_PARAM 	struct inode *inode, struct file *file, struct dirent *dirent, int count
#define __LDDK_SELECT_PARAM 	struct inode *inode, struct file *filp, int sel_type, select_table * wait
#define __LDDK_IOCTL_PARAM 	struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg
#define __LDDK_MMAP_PARAM 	struct inode *inode, struct file *file, struct vm_area_struct * vma
#define __LDDK_OPEN_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_CLOSE_PARAM 	struct inode *inode, struct file *file 
#define __LDDK_FSYNC_PARAM 	struct inode *inode, struct file *file
#define __LDDK_FASYNC_PARAM 	struct inode *inode, struct file *file, int count 
#define __LDDK_CCHECK_PARAM 	kdev_t dev
#define __LDDK_REVAL_PARAM 	kdev_t dev

#define __LDDK_MINOR MINOR(inode->i_rdev)
#define __LDDK_INO_MINOR MINOR(inode->i_rdev)

#endif



/************************************************************************/
#include <can_debug.h>
/************************************************************************/
/* #include <can_Proto.h> */
#ifdef __KERNEL__
extern int can_read (__LDDK_READ_PARAM); 
extern __LDDK_WRITE_TYPE can_write (__LDDK_WRITE_PARAM); 
extern __LDDK_SELECT_TYPE can_select ( __LDDK_SELECT_PARAM ); 
extern int can_ioctl ( __LDDK_IOCTL_PARAM ); 
extern int can_open ( __LDDK_OPEN_PARAM ); 
extern __LDDK_CLOSE_TYPE can_close (__LDDK_CLOSE_PARAM); 
#endif 


/*---------- Default Outc value for some known boards
 * this depends on the transceiver configuration
 * 
 * the port board uses optocoupler configuration as denoted in the philips
 * application notes, so the OUTC value is 0xfa
 *
 */

#if   defined(ATCANMINI_BASIC) || defined(ATCANMINI_PELICAN)
# define CAN_OUTC_VAL           0xfa
# define IO_MODEL		"pppp "
# define VEND_OPT		"nnnn "
#elif defined(IME_SLIMLINE)
# define CAN_OUTC_VAL           0xda
# define IO_MODEL		"mmmm "
# define VEND_OPT		"nnnn "
#elif defined(CPC_PCI)
# define CAN_OUTC_VAL           0xda
# define IO_MODEL		"mmmm "
# define VEND_OPT		"nnnn "
#elif defined(IXXAT_PCI03)
# define CAN_OUTC_VAL           0x5e
# define IO_MODEL		"mmmm "
# define VEND_OPT		"ssss "
#else 
# define CAN_OUTC_VAL           0x00
# define IO_MODEL		"mmmm "
# define VEND_OPT		"nnnn "
/* #error no CAN_OUTC_VAL */
#endif


/************************************************************************/
#include <can_82c200.h>
/************************************************************************/
/************************************************************************/
#include <can4linux.h>
/************************************************************************/
 extern volatile int irq2minormap[];
 extern volatile int irq2pidmap[];
 extern u32 Can_pitapci_control[];

 #define MAX_BUFS 4
 /* #define MAX_BUFSIZE 64 */
 #define MAX_BUFSIZE 200
 /* #define MAX_BUFSIZE 4 */

 #define BUF_EMPTY    0
 #define BUF_OK       1
 #define BUF_FULL     BUF_OK
 #define BUF_OVERRUN  2
 #define BUF_UNDERRUN 3


 typedef struct {
	int head;
        int tail;
        int status;
	int active;
	char free[MAX_BUFSIZE];
        canmsg_t data[MAX_BUFSIZE];
 } msg_fifo_t;



#ifdef CAN_USE_FILTER
 #define MAX_ID_LENGTH 11
 #define MAX_ID_NUMBER (1<<11)

 typedef struct {
	unsigned    use;
	unsigned    signo[3];
	struct {
		unsigned    enable    : 1;
		unsigned    timestamp : 1;
		unsigned    signal    : 2;
		canmsg_t    *rtr_response;
	} filter[MAX_ID_NUMBER];
 } msg_filter_t;



 extern msg_filter_t Rx_Filter[];
#endif

 extern msg_fifo_t Tx_Buf[];
 extern msg_fifo_t Rx_Buf[];

 typedef void (*irq_handler_t)(int irq, void *unused, struct pt_regs *ptregs);

 extern int Can_RequestIrq(int minor, int irq, irq_handler_t handler);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,1)
 extern wait_queue_head_t CanWait[];
#else
 extern struct wait_queue *CanWait[];
#endif

 extern unsigned char *can_base[];
 extern unsigned int   can_range[];
#endif


/************************************************************************/



/************************************************************************/
#define LDDK_START_TIMER Can_StartTimer
#define LDDK_STOP_TIMER  Can_StopTimer
#define LDDK_TIMEDOUT    Can_Timeout

extern int Can_Timeout;


/************************************************************************/
#define LDDK_USE_SYSCTL 1
#ifdef __KERNEL__
#include <linux/sysctl.h>

extern ctl_table Can_sysctl_table[];
extern ctl_table Can_sys_table[];



 /* ------ Global Definitions for version */

extern char version[];
#define SYSCTL_VERSION 1
 
 /* ------ Global Definitions for IOModel */

extern char IOModel[];
#define SYSCTL_IOMODEL 2
 
 /* ------ Global Definitions for TxSpeed */

extern char TxSpeed[];
#define SYSCTL_TXSPEED 3
 
 /* ------ Global Definitions for VendOpt */

extern char VendOpt[];
#define SYSCTL_VENDOPT 4
 
 /* ------ Global Definitions for IRQ */

extern  int IRQ[];
#define SYSCTL_IRQ 5
 
 /* ------ Global Definitions for Base */

extern  int Base[];
#define SYSCTL_BASE 6
 
 /* ------ Global Definitions for Baud */

extern  int Baud[];
#define SYSCTL_BAUD 7
 
 /* ------ Global Definitions for AccCode */

extern  unsigned int AccCode[];
#define SYSCTL_ACCCODE 8
 
 /* ------ Global Definitions for AccMask */

extern  unsigned int AccMask[];
#define SYSCTL_ACCMASK 9
 
 /* ------ Global Definitions for Timeout */

extern  int Timeout[];
#define SYSCTL_TIMEOUT 10
 
 /* ------ Global Definitions for Outc */

extern  int Outc[];
#define SYSCTL_OUTC 11
 
 /* ------ Global Definitions for TxErr */

extern  int TxErr[];
#define SYSCTL_TXERR 12
 
 /* ------ Global Definitions for RxErr */

extern  int RxErr[];
#define SYSCTL_RXERR 13
 
 /* ------ Global Definitions for Overrun */

extern  int Overrun[];
#define SYSCTL_OVERRUN 14
 
 /* ------ Global Definitions for Chipset */

 /* ------ Global Definitions for dbgMask */

extern unsigned int dbgMask;
#define SYSCTL_DBGMASK 15

 /* ------ Global Definitions for Test  */
extern  int Cnt1[];
#define SYSCTL_CNT1 16
extern  int Cnt2[];
#define SYSCTL_CNT2 17
 
 
#endif
/************************************************************************/



#ifndef Can_MAJOR
#define Can_MAJOR 63
#endif

extern int Can_errno;

#ifdef USE_LDDK_RETURN
#define LDDK_RETURN(arg) DBGout();return(arg)
#else
#define LDDK_RETURN(arg) return(arg)
#endif


/************************************************************************/
/* function prototypes */
/************************************************************************/
extern int CAN_ChipReset(int);
extern int CAN_SetTiming(int, int);
extern int CAN_StartChip(int);
extern int CAN_StopChip(int);
extern int CAN_SetMask(int, unsigned int, unsigned int);
extern int CAN_SetOMode(int,int);
extern int CAN_SendMessage(int, canmsg_t *, int );
extern int CAN_GetMessage(int , canmsg_t *);
extern void CAN_Interrupt(int irq, void *unused, struct pt_regs *ptregs );
extern int CAN_VendorInit(int);

extern void register_systables(void);
extern void unregister_systables(void);

/* can_82c200funcs.c */
extern int CAN_ShowStat (int board);

/* util.c */
extern int Can_FifoInit(int minor);
extern int Can_FilterCleanup(int minor);
extern int Can_FilterInit(int minor);
extern int Can_FilterMessage(int minor, unsigned message, unsigned enable);
extern int Can_FilterOnOff(int minor, unsigned on);
extern int Can_FilterSigNo(int minor, unsigned signo, unsigned signal);
extern int Can_FilterSignal(int minor, unsigned id, unsigned signal);
extern int Can_FilterTimestamp(int minor, unsigned message, unsigned stamp);
extern int Can_FreeIrq(int minor, int irq );
extern int Can_WaitInit(int minor);
extern void Can_StartTimer(unsigned long v);
extern void Can_StopTimer(void);
extern void Can_TimerInterrupt(unsigned long unused);
extern void Can_dump(int minor);
extern void print_tty(const char *fmt, ...);

/* PCI support */
extern int pcimod_scan(void);

/************************************************************************/
/* hardware access functions or macros */
/************************************************************************/
#ifdef  CAN_PORT_IO
/* #error Intel port I/O access */
/* using port I/O with inb()/outb() for Intel architectures like 
   AT-CAN-MINI ISA board */

#ifdef IODEBUG
#  define CANout(bd,adr,v)	\
	(printk("Cout: (%x)=%x\n", (int)&((canregs_t *)Base[bd])->adr, v), \
		outb(v, (int) &((canregs_t *)Base[bd])->adr ))
#else
#  define CANout(bd,adr,v)	\
		(outb(v, (int) &((canregs_t *)Base[bd])->adr ))
#endif
#define CANin(bd,adr)		\
		(inb ((int) &((canregs_t *)Base[bd])->adr  ))
#define CANset(bd,adr,m)	\
	outb((inb((int) &((canregs_t *)Base[bd])->adr)) \
		| m ,(int) &((canregs_t *)Base[bd])->adr )
#define CANreset(bd,adr,m)	\
	outb((inb((int) &((canregs_t *)Base[bd])->adr)) \
		& ~m,(int) &((canregs_t *)Base[bd])->adr )
#define CANtest(bd,adr,m)	\
	(inb((int) &((canregs_t *)Base[bd])->adr  ) & m )

#else 	/* CAN_PORT_IO */
/* using memory acces with readb(), writeb() */
/* #error  memory I/O access */
/* #define can_base Base */
#ifdef IODEBUG
#  define CANout(bd,adr,v)	\
	(printk("Cout: (%x)=%x\n", (u32)&((canregs_t *)can_base[bd])->adr, v), \
		writeb(v, (u32) &((canregs_t *)can_base[bd])->adr ))

#define CANset(bd,adr,m)     do	{\
	unsigned char v;	\
        v = (readb((u32) &((canregs_t *)can_base[bd])->adr)); \
	printk("CANset %x |= %x\n", (v), (m)); \
	writeb( v | (m) , (u32) &((canregs_t *)can_base[bd])->adr ); \
	} while (0)

#define CANreset(bd,adr,m)	do {\
	unsigned char v; \
        v = (readb((u32) &((canregs_t *)can_base[bd])->adr)); \
	printk("CANreset %x &= ~%x\n", (v), (m)); \
	writeb( v & ~(m) , (u32) &((canregs_t *)can_base[bd])->adr ); \
	} while (0)

#else
#define CANout(bd,adr,v)	\
		(writeb(v, (u32) &((canregs_t *)can_base[bd])->adr ))
#define CANset(bd,adr,m)	\
	writeb((readb((u32) &((canregs_t *)can_base[bd])->adr)) \
		| (m) , (u32) &((canregs_t *)can_base[bd])->adr )
#define CANreset(bd,adr,m)	\
	writeb((readb((u32) &((canregs_t *)can_base[bd])->adr)) \
		& ~(m), (u32) &((canregs_t *)can_base[bd])->adr )
#endif
#define CANin(bd,adr)		\
		(readb ((u32) &((canregs_t *)can_base[bd])->adr  ))
#define CANtest(bd,adr,m)	\
	(readb((u32) &((canregs_t *)can_base[bd])->adr  ) & (m) )


#endif 		/* CAN_PORT_IO */

/*________________________E_O_F_____________________________________________*/


#if 0
/*
There is a special access mode for the 82527 CAN controller
called fast register access.
This mode is sometimes used instead of normal Mem-Read/Write
and substitutes  MEM_In/MEM_Out

*/

uint8 MEM_In (unsigned long  adr ) { 
#if IODEBUG
        int ret = readb(adr);
        printk("MIn: 0x%x=0x%x\n", adr, ret);
        return ret;
#else
	SLOW_DOWN_IO;	SLOW_DOWN_IO;
	return readb(adr);
#endif
}

uint8 fastMEM_In (unsigned long  adr ) {
        /* Fast access:
         * first read desired register,
         * after that read register 4
         */
	int ret = readb(adr);
        if((adr & 0x000000FF) == 0x02) {
        	return ret;
	}
	/* 250 ns delay, SLOW_DOWN_IO is empty on ELIMA */
	asm volatile("nop;nop;nop;nop");
	asm volatile("nop;nop;nop;nop");
	asm volatile("nop;nop;nop;nop");
	asm volatile("nop;nop;nop;nop");
	ret = readb((adr & 0xFFFFFF00) + 4);
#if IODEBUG
        printk("MfIn: 0x%x=0x%x\n", adr, ret);
#endif
        return ret;
}

void MEM_Out(uint8 v, unsigned long adr ) {
#if IODEBUG
        printk("MOut: 0x%x->0x%x\n", v, adr);
#endif
	SLOW_DOWN_IO;
        writeb(v, adr);
	SLOW_DOWN_IO; 
#if defined(CONFIG_PPC)
	/* 250 ns delay, SLOW_DOWN_IO is empty on ELIMA */
	asm volatile("nop;nop;nop;nop");
	asm volatile("nop;nop;nop;nop");
	asm volatile("nop;nop;nop;nop");
	asm volatile("nop;nop;nop;nop");
#endif
}

#endif







