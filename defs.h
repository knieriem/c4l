/*
 * can_read - can4linux CAN driver module
 *
 * Copyright (c) 2001 port GmbH Halle/Saale
  */

#include <linux/kernel.h>
#include <linux/interrupt.h>         /* tasklets */

#include <linux/slab.h>

#include <linux/poll.h>

#include <asm/io.h>

#include <linux/mm.h>
#include <linux/signal.h>
#include <linux/timer.h>

#include <asm/uaccess.h>

#define __lddk_copy_from_user(a,b,c) if(copy_from_user(a,b,c)){}
#define __lddk_copy_to_user(a,b,c) if(copy_to_user(a,b,c)){}

#include <linux/ioport.h>

extern	void	decusers(void);
extern	void	incusers(void);
extern	int	inuse(void);

#if CAN4LINUX_PCI
# define _BUS_TYPE "PCI-"
/* the only one supported: EMS CPC-PCI */
# define PCIvendorid 0x110a
# define PCIdeviceid 0x2104

#else
# define _BUS_TYPE "ISA-"
#endif

# ifdef  CAN_PORT_IO
#  define __CAN_TYPE__ _BUS_TYPE "PeliCAN-port I/O "
# else
# ifdef  CAN_INDEXED_PORT_IO
#  define __CAN_TYPE__ _BUS_TYPE "PeliCAN-indexed port I/O "
# else
#  define __CAN_TYPE__ _BUS_TYPE "PeliCAN-memory mapped "
# endif
# endif


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


/************************************************************************/
#include "debug.h"
/************************************************************************/
/* #include <can_Proto.h> */
extern int can_read (__LDDK_READ_PARAM); 
extern __LDDK_WRITE_TYPE can_write (__LDDK_WRITE_PARAM); 
extern __LDDK_SELECT_TYPE can_select ( __LDDK_SELECT_PARAM ); 
extern int can_ioctl ( __LDDK_IOCTL_PARAM ); 
extern int can_open ( __LDDK_OPEN_PARAM ); 
extern __LDDK_CLOSE_TYPE can_close (__LDDK_CLOSE_PARAM); 


/*---------- Default Outc value for some known boards
 * this depends on the transceiver configuration
 * 
 * the port board uses optocoupler configuration as denoted in the philips
 * application notes, so the OUTC value is 0xfa
 *
 */

#if   defined(ATCANMINI_BASIC) || defined(ATCANMINI_PELICAN)
# define CAN_OUTC_VAL           0xfa
# define IO_MODEL		'p'
#elif defined(IME_SLIMLINE)
# define CAN_OUTC_VAL           0xda
# define IO_MODEL		'm'
#elif defined(CPC_PCI)
# define CAN_OUTC_VAL           0xda
# define IO_MODEL		'm'
#elif defined(IXXAT_PCI03)
# define CAN_OUTC_VAL           0x5e
# define IO_MODEL		'm'
#elif defined(PCM3680)
# define CAN_OUTC_VAL           0x5e
# define IO_MODEL		'm'
#elif defined(TRM816)
# define CAN_OUTC_VAL           0x1a

    /* 0x0100 0010
       normal mode    : 0x02
       Tx1: float     : 0x00
       Tx0: push/pull : 0x18
       ---------------------
       OR ==>           0x1A */

# define IO_MODEL		'm'    /* unused */
#else 
# define CAN_OUTC_VAL           0x00
# define IO_MODEL		'm'
/* #error no CAN_OUTC_VAL */
#endif


#include "sja1000.h"

#include "can4linux.h"
/************************************************************************/
 extern u32 Can_pitapci_control[];


/* number of supported CAN channels */
#define MAX_CHANNELS 4

#define MAX_RX_BUFSIZE 2048
#define MAX_TX_BUFSIZE 128

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

        /* dynamically allocated buffer, see can_util.c:Can_FifoInit() */
	int size;
	char *free;
        canmsg_t *data;
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

 extern int Can_RequestIrq(int minor, int irq);

 extern wait_queue_head_t CanWait[];

 extern unsigned char *can_base[];
 extern unsigned int   can_range[];

extern int IRQ_requested[];
extern int Can_minors[];			/* used as IRQ dev_id */


#ifndef Can_MAJOR
#define Can_MAJOR 91
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
extern int CAN_Interrupt(int irq, void *unused);
extern int CAN_VendorInit(int);

extern void register_systables(void);
extern void unregister_systables(void);

/* can_82c200funcs.c */
extern int CAN_ShowStat (int board);

/* util.c */
extern int Can_FifoInit(int minor);
extern int Can_FifoCleanup(int minor);
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

#ifdef CAN_INDEXED_PORT_IO
extern canregs_t* regbase;
static inline unsigned Indexed_Inb(unsigned base,unsigned adr) {
  unsigned val;
  outb(adr,base);
  val=inb(base+1);
#ifdef IODEBUG
  printk("CANin: base: %x adr: %x, got: %x\n",base,adr,val);
#endif
  return val;
}
#endif
