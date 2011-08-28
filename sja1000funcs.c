/* can_sja1000funcs
*
* can4linux -- LINUX CAN device driver source
* 
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * 
* Copyright (c) 2002 port GmbH Halle/Saale
* (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
*/

#include "defs.h"
#include <asm/dma.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include "io.h"
#include ",,sysctl.h"

/* int	CAN_Open = 0; */

/* timing values */
u8 CanTiming[10][2]={
	{CAN_TIM0_10K,  CAN_TIM1_10K},
	{CAN_TIM0_20K,  CAN_TIM1_20K},
	{CAN_TIM0_50K,  CAN_TIM1_50K},
	{CAN_TIM0_100K, CAN_TIM1_100K},
	{CAN_TIM0_125K, CAN_TIM1_125K},
	{CAN_TIM0_250K, CAN_TIM1_250K},
	{CAN_TIM0_500K, CAN_TIM1_500K},
	{CAN_TIM0_800K, CAN_TIM1_800K},
	{CAN_TIM0_1000K,CAN_TIM1_1000K}};




/* Board reset
   means the following procedure:
  set Reset Request
  wait for Rest request is changing - used to see if board is available
  initialize board (with valuse from /proc/sys/Can)
    set output control register
    set timings
    set acceptance mask
*/


#ifdef DEBUG
int CAN_ShowStat (int board)
{
    if (dbgMask && (dbgMask & DBG_DATA)) {
    printk(" MODE 0x%x,", CANin(board, canmode));
    printk(" STAT 0x%x,", CANin(board, canstat));
    printk(" IRQE 0x%x,", CANin(board, canirq_enable));
    /* printk(" INT 0x%x\n", CANin(board, canirq)); */
    printk("\n");
    }
    return 0;
}
#endif

/* can_GetStat - read back as many status information as possible 
*
* Because the CAN protocol itself describes different kind of information
* already, and the driver has some generic information
* (e.g about it's buffers)
* we can define a more or less hardware independent format.
*
*
* exception:
* ERROR WARNING LIMIT REGISTER (EWLR)
* The SJA1000 defines a EWLR, reaching this Error Warning Level
* an Error Warning interrupt can be generated.
* The default value (after hardware reset) is 96. In reset
* mode this register appears to the CPU as a read/write
* memory. In operating mode it is read only.
* Note, that a content change of the EWLR is only possible,
* if the reset mode was entered previously. An error status
* change (see status register; Table 14) and an error
* warning interrupt forced by the new register content will not
* occur until the reset mode is cancelled again.
*/

int can_GetStat(
	struct inode *inode,
	CanStatusPar_t *stat
	)
{
unsigned int board = MINOR(inode->i_rdev);	
msg_fifo_t *Fifo;
unsigned long flags;

    stat->type = CAN_TYPE_SJA1000;

    stat->baud = Baud[board];
    /* printk(" STAT ST %d\n", CANin(board, canstat)); */
    stat->status = CANin(board, canstat);
    /* printk(" STAT EWL %d\n", CANin(board, errorwarninglimit)); */
    stat->error_warning_limit = CANin(board, errorwarninglimit);
    stat->rx_errors = CANin(board, rxerror);
    stat->tx_errors = CANin(board, txerror);
    stat->error_code= CANin(board, errorcode);

    /* Disable CAN Interrupts */
    /* !!!!!!!!!!!!!!!!!!!!! */
    save_flags(flags); cli();
    Fifo = &Rx_Buf[board];
    stat->rx_buffer_size = Fifo->size;		/**< size of rx buffer  */
    /* number of messages */
    stat->rx_buffer_used =
    	(Fifo->head < Fifo->tail)
    	? (Fifo->size - Fifo->tail + Fifo->head) : (Fifo->head - Fifo->tail);
    Fifo = &Tx_Buf[board];
    stat->tx_buffer_size = Fifo->size;		/* size of tx buffer  */
    /* number of messages */
    stat->tx_buffer_used = 
    	(Fifo->head < Fifo->tail)
    	? (Fifo->size - Fifo->tail + Fifo->head) : (Fifo->head - Fifo->tail);
    /* Enable CAN Interrupts */
    /* !!!!!!!!!!!!!!!!!!!!! */
    restore_flags(flags);
    return 0;
}

int CAN_ChipReset (int board)
{
uint8 status;
/* int i; */

    DBGin("CAN_ChipReset");
    DBGprint(DBG_DATA,(" INT 0x%x", CANin(board, canirq)));

    CANout(board, canmode, CAN_RESET_REQUEST);



    /* for(i = 0; i < 100; i++) SLOW_DOWN_IO; */
    udelay(10);

    status = CANin(board, canstat);

    DBGprint(DBG_DATA,("status=0x%x mode=0x%x", status,
	    CANin(board, canmode) ));
    if( ! (CANin(board, canmode) & CAN_RESET_REQUEST ) ){
	    printk("ERROR: no board present!");
	    decusers();
	    DBGout();return -1;
    }

    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x", board, CANin(board, canmode)));
    /* select mode: Basic or PeliCAN */
    CANout(board, canclk, CAN_MODE_PELICAN + CAN_MODE_CLK);
    CANout(board, canmode, CAN_RESET_REQUEST + CAN_MODE_DEF);
    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x", board, CANin(board, canmode)));
    
    /* Board specific output control */
    if (Outc[board] == 0) {
	Outc[board] = CAN_OUTC_VAL; 
    }
    CANout(board, canoutc, Outc[board]);
    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x", board, CANin(board, canmode)));

    CAN_SetTiming(board, Baud[board]    );
    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x", board, CANin(board, canmode)));
    CAN_SetMask  (board, AccCode[board], AccMask[board] );
    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x", board, CANin(board, canmode)));

    /* Can_dump(board); */
    DBGout();
    return 0;
}


int CAN_SetTiming (int board, int baud)
{
int i = 5;
int custom=0;
    DBGin("CAN_SetTiming");
    DBGprint(DBG_DATA, ("baud[%d]=%d", board, baud));
    switch(baud)
    {
	case   10: i = 0; break;
	case   20: i = 1; break;
	case   50: i = 2; break;
	case  100: i = 3; break;
	case  125: i = 4; break;
	case  250: i = 5; break;
	case  500: i = 6; break;
	case  800: i = 7; break;
	case 1000: i = 8; break;
	default  : 
		custom=1;
    }
    /* select mode: Basic or PeliCAN */
    CANout(board, canclk, CAN_MODE_PELICAN + CAN_MODE_CLK);
    if( custom ) {
       CANout(board, cantim0, (uint8) (baud >> 8) & 0xff);
       CANout(board, cantim1, (uint8) (baud & 0xff ));
       printk(" custom bit timing BT0=0x%x BT1=0x%x ",
       		CANin(board, cantim0), CANin(board, cantim1));
    } else {
       CANout(board,cantim0, (uint8) CanTiming[i][0]);
       CANout(board,cantim1, (uint8) CanTiming[i][1]);
    }
    DBGprint(DBG_DATA,("tim0=0x%x tim1=0x%x",
    		CANin(board, cantim0), CANin(board, cantim1)) );

    DBGout();
    return 0;
}


int CAN_StartChip (int board)
{
/* int i; */
    RxErr[board] = TxErr[board] = 0L;
    DBGin("CAN_StartChip");
    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x", board, CANin(board, canmode)));
/*
    CANout( board,cancmd, (CAN_RELEASE_RECEIVE_BUFFER 
			  | CAN_CLEAR_OVERRUN_STATUS) ); 
*/
    /* for(i=0;i<100;i++) SLOW_DOWN_IO; */
    udelay(10);
    /* clear interrupts */
    CANin(board, canirq);

    /* Interrupts on Rx, TX, any Status change and data overrun */
    CANout(board, canirq_enable, (
		  CAN_OVERRUN_INT_ENABLE
		+ CAN_ERROR_WARN_INT_ENABLE
		+ CAN_TRANSMIT_INT_ENABLE
		+ CAN_RECEIVE_INT_ENABLE ));

    CANreset( board, canmode, CAN_RESET_REQUEST );
    DBGprint(DBG_DATA,("start mode=0x%x", CANin(board, canmode)));

    DBGout();
    return 0;
}


int CAN_StopChip (int board)
{
    DBGin("CAN_StopChip");
    CANset(board, canmode, CAN_RESET_REQUEST );
    DBGout();
    return 0;
}

/* set value of the output control register */
int CAN_SetOMode (int board, int arg)
{

    DBGin("CAN_SetOMode");
	DBGprint(DBG_DATA,("[%d] outc=0x%x", board, arg));
	CANout(board, canoutc, arg);

    DBGout();
    return 0;
}


int CAN_SetMask (int board, unsigned int code, unsigned int mask)
{
#ifdef CPC_PCI
# define R_OFF 4 /* offset 4 for the EMS CPC-PCI card */
#else
# define R_OFF 1
#endif

    DBGin("CAN_SetMask");
    DBGprint(DBG_DATA,("[%d] acc=0x%x mask=0x%x",  board, code, mask));
    CANout(board, frameinfo,
			(unsigned char)((unsigned int)(code >> 24) & 0xff));	
    CANout(board, frame.extframe.canid1,
			(unsigned char)((unsigned int)(code >> 16) & 0xff));	
    CANout(board, frame.extframe.canid2,
			(unsigned char)((unsigned int)(code >>  8) & 0xff));	
    CANout(board, frame.extframe.canid3,
    			(unsigned char)((unsigned int)(code >>  0 ) & 0xff));	

    CANout(board, frame.extframe.canid4,
			(unsigned char)((unsigned int)(mask >> 24) & 0xff));
    CANout(board, frame.extframe.canxdata[0 * R_OFF],
			(unsigned char)((unsigned int)(mask >> 16) & 0xff));
    CANout(board, frame.extframe.canxdata[1 * R_OFF],
			(unsigned char)((unsigned int)(mask >>  8) & 0xff));
    CANout(board, frame.extframe.canxdata[2 * R_OFF],
			(unsigned char)((unsigned int)(mask >>  0) & 0xff));

    DBGout();
    return 0;
}


int CAN_SendMessage (int board, canmsg_t *tx)
{
int i = 0;
int ext;			/* message format to send */
uint8 tx2reg, stat;

    DBGin("CAN_SendMessage");

    while ( ! ((stat=CANin(board, canstat)) & CAN_TRANSMIT_BUFFER_ACCESS )) {
	    if( current->need_resched ) schedule();
    }

    DBGprint(DBG_DATA,(
    		"CAN[%d]: tx.flags=%d tx.id=0x%lx tx.length=%d stat=0x%x",
		board, tx->flags, tx->id, tx->length, stat));


    /* delay message sending for some micro-seconds, if requested by
       the user */
    if (tx->timestamp.tv_usec) {
	if (tx->timestamp.tv_usec > 1000)
	    mdelay (tx->timestamp.tv_usec / 1000);
	else
	    udelay (tx->timestamp.tv_usec);
    }


    tx->length %= 9;			/* limit CAN message length to 8 */
    ext = (tx->flags & MSG_EXT);	/* read message format */

    /* fill the frame info and identifier fields */
    tx2reg = tx->length;
    if( tx->flags & MSG_RTR) {
	    tx2reg |= CAN_RTR;
    }
    if(ext) {
    DBGprint(DBG_DATA, ("---> send ext message"));
	CANout(board, frameinfo, CAN_EFF + tx2reg);
	CANout(board, frame.extframe.canid1, (uint8)(tx->id >> 21));
	CANout(board, frame.extframe.canid2, (uint8)(tx->id >> 13));
	CANout(board, frame.extframe.canid3, (uint8)(tx->id >> 5));
	CANout(board, frame.extframe.canid4, (uint8)(tx->id << 3) & 0xff);

    } else {
	DBGprint(DBG_DATA, ("---> send std message"));
	CANout(board, frameinfo, CAN_SFF + tx2reg);
	CANout(board, frame.stdframe.canid1, (uint8)((tx->id) >> 3) );
	CANout(board, frame.stdframe.canid2, (uint8)(tx->id << 5 ) & 0xe0);
    }

    /* - fill data ---------------------------------------------------- */
    if(ext) {
	for( i=0; i < tx->length ; i++) {
	    CANout( board, frame.extframe.canxdata[R_OFF * i], tx->data[i]);
	    	/* SLOW_DOWN_IO; SLOW_DOWN_IO; */
	}
    } else {
	for( i=0; i < tx->length ; i++) {
	    CANout( board, frame.stdframe.candata[R_OFF * i], tx->data[i]);
	    	/* SLOW_DOWN_IO; SLOW_DOWN_IO; */
	}
    }
    /* - /end --------------------------------------------------------- */
    CANout(board, cancmd, CAN_TRANSMISSION_REQUEST );

  DBGout();return i;
}

int CAN_VendorInit (int minor)
{
    DBGin("CAN_VendorInit");

/* 1. Vendor specific part ------------------------------------------------ */
#if defined(IXXAT_PCI03) || defined (PCM3680)
    can_range[minor] = 0x200; /*stpz board or Advantech Pcm-3680 */
#else
    can_range[minor] = CAN_RANGE;
#endif
    
/* End: 1. Vendor specific part ------------------------------------------- */


#if !defined(CAN4LINUX_PCI)
    /* Request the controllers address space */
#if defined(CAN_PORT_IO) 
    /* It's port I/O */
    if(NULL == request_region(Base[minor], can_range[minor], "CAN-IO")) {
	return -EBUSY;
    }
#else
#if defined(CAN_INDEXED_PORT_IO)
    /* It's indexed port I/O */
    if(NULL == request_region(Base[minor], 2, "CAN-IO")) {
      return -EBUSY;
    } 
#else
    /* It's Memory I/O */
    if(NULL == request_mem_region(Base[minor], can_range[minor], "CAN-IO")) {
	return -EBUSY;
    }
#endif
#endif
#endif 	/* !defined(CAN4LINUX_PCI) */

#if CAN4LINUX_PCI
	/* PCI scan has already remapped the address */
	can_base[minor] = (unsigned char *)Base[minor];
#else
	can_base[minor] = ioremap(Base[minor], 0x200);
#endif

    /* now the virtual address can be used for the register address macros */


/* 2. Vendor specific part ------------------------------------------------ */

/* ixxat/stpz board */
#if defined(IXXAT_PCI03)
	if( Base[minor] & 0x200 ) {
		printk("Resetting IXXAT PC I-03 [contr 1]\n");
		/* perform HW reset 2. contr*/
		writeb(0xff, can_base[minor] + 0x300);
	} else {
		printk("Resetting IXXAT PC I-03 [contr 0]\n");
		/* perform HW reset 1. contr*/
		writeb(0xff, can_base[minor] + 0x100);
	}
#endif
/* Advantech Pcm-3680 */
#if defined (PCM3680)
	if( Base[minor] & 0x200 ) {
		printk("Resetting Advantech Pcm-3680 [contr 1]\n");
		/* perform HW reset 2. contr*/
		writeb(0xff, can_base[minor] + 0x300);
	} else {
		printk("Resetting Advantech Pcm-3680 [contr 0]\n");
		/* perform HW reset 1. contr*/
		writeb(0xff, can_base[minor] + 0x100);
	}
        mdelay(100);
#endif


/* End: 2. Vendor specific part ------------------------------------------- */

    if( IRQ[minor] > 0 ) {
        if( Can_RequestIrq( minor, IRQ[minor]) ) {
	     printk("Can[%d]: Can't request IRQ %d \n", minor, IRQ[minor]);
	     DBGout(); return -EBUSY;
        }
    }

    DBGout(); return 0;
}



/* called from within CAN_Interrupt on transmit interrupt, and
   from within the delayed_send tasklet */
static void
send_tx_queue_msg (int minor)
{
    int i;
    unsigned int id;
    int ext;

    msg_fifo_t	 *TxFifo = &Tx_Buf[minor];

    uint8 tx2reg;

    tx2reg = (TxFifo->data[TxFifo->tail]).length;
    if( (TxFifo->data[TxFifo->tail]).flags & MSG_RTR) {
	tx2reg |= CAN_RTR;
    }
    ext = (TxFifo->data[TxFifo->tail]).flags & MSG_EXT;
    id = (TxFifo->data[TxFifo->tail]).id;
    if(ext) {
	DBGprint(DBG_DATA, ("---> send ext message"));
	CANout(minor, frameinfo, CAN_EFF + tx2reg);
	CANout(minor, frame.extframe.canid1, (uint8)(id >> 21));
	CANout(minor, frame.extframe.canid2, (uint8)(id >> 13));
	CANout(minor, frame.extframe.canid3, (uint8)(id >> 5));
	CANout(minor, frame.extframe.canid4, (uint8)(id << 3) & 0xff);
    } else {
	DBGprint(DBG_DATA, ("---> send std message"));
	CANout(minor, frameinfo, CAN_SFF + tx2reg);
	CANout(minor, frame.stdframe.canid1, (uint8)((id) >> 3) );
	CANout(minor, frame.stdframe.canid2, (uint8)(id << 5 ) & 0xe0);
    }


    tx2reg &= 0x0f;		/* restore length only */
    if(ext) {
	for( i=0; i < tx2reg ; i++) {
	    CANout(minor, frame.extframe.canxdata[R_OFF * i],
		   (TxFifo->data[TxFifo->tail]).data[i]);
	}
    } else {
	for( i=0; i < tx2reg ; i++) {
	    CANout(minor, frame.stdframe.candata[R_OFF * i],
		   (TxFifo->data[TxFifo->tail]).data[i]);
	}
    }

    CANout (minor, cancmd, CAN_TRANSMISSION_REQUEST);

    TxFifo->free[TxFifo->tail] = BUF_EMPTY; /* now this entry is EMPTY */
    TxFifo->tail = ++(TxFifo->tail) % TxFifo->size;
}



/* tasklet for the delayed transmission of a message */
static void
delayed_send (unsigned long minor)
{
    unsigned long flags;

    msg_fifo_t	 *TxFifo = &Tx_Buf[minor];

    /* enter critical section */
    save_flags(flags);cli();

    if (TxFifo->data[TxFifo->tail].timestamp.tv_usec > 1000)
	mdelay (TxFifo->data[TxFifo->tail].timestamp.tv_usec / 1000);
    else
	udelay (TxFifo->data[TxFifo->tail].timestamp.tv_usec);

    send_tx_queue_msg (minor);

    /* leave critical section */
    restore_flags(flags);
}




/*
 * The plain SJA1000 interrupt
 *
 *				RX ISR           TX ISR
 *                              8/0 byte
 *                               _____            _   ___
 *                         _____|     |____   ___| |_|   |__
 *---------------------------------------------------------------------------
 * 1) CPUPentium 75 - 200
 *  75 MHz, 149.91 bogomips
 *  AT-CAN-MINI                 42/27µs            50 µs
 *  CPC-PCI		        35/26µs		   
 * ---------------------------------------------------------------------------
 * 2) AMD Athlon(tm) Processor 1M
 *    2011.95 bogomips
 *  AT-CAN-MINI  std            24/12µs            ?? µs
 *               ext id           /14µs
 *
 * 
 * 1) 1Byte = (42-27)/8 = 1.87 µs
 * 2) 1Byte = (24-12)/8 = 1.5  µs
 *
 *
 *
 * RX Int with to Receive channels:
 * 1)                _____   ___
 *             _____|     |_|   |__
 *                   30    5  20  µs
 *   first ISR normal length,
 *   time between the two ISR -- short
 *   sec. ISR shorter than first, why? it's the same message
 */

int CAN_Interrupt ( int irq, void *dev_id)
{
int minor;
int i;
unsigned long flags;
int ext;			/* flag for extended message format */
int irqsrc, dummy;
msg_fifo_t   *RxFifo; 
msg_fifo_t   *TxFifo;
#if CAN_USE_FILTER
msg_filter_t *RxPass;
unsigned int id;
#endif 
#if 0
int first = 0;
#endif 

#if CONFIG_TIME_MEASURE
    outb(0xff, 0x378);  
#endif

    minor = *(int *)dev_id;
    RxFifo = &Rx_Buf[minor]; 
    TxFifo = &Tx_Buf[minor];
#if CAN_USE_FILTER
    RxPass = &Rx_Filter[minor];
#endif 

    /* Disable PITA Interrupt */
    /* writel(0x00000000, Can_pitapci_control[minor] + 0x0); */

    irqsrc = CANin(minor, canirq);
    if(irqsrc == 0) {
         /* first call to ISR, it's not for me ! */
	 goto IRQdone_doneNothing;
    }
    do {
    /* loop as long as the CAN controller shows interrupts */
    DBGprint(DBG_DATA, (" => got IRQ[%d]: 0x%0x", minor, irqsrc));
    /* Can_dump(minor); */

    do_gettimeofday(&(RxFifo->data[RxFifo->head]).timestamp);

#if 0
    /* how often do we lop through the ISR ? */
    if(first) printk("n = %d", first);
    first++;
#endif

    /* preset flags */
    (RxFifo->data[RxFifo->head]).flags =
        		(RxFifo->status & BUF_OVERRUN ? MSG_BOVR : 0);

    /*========== receive interrupt */
    if( irqsrc & CAN_RECEIVE_INT ) {
        /* fill timestamp as first action */

	dummy  = CANin(minor, frameinfo );
        if( dummy & CAN_RTR ) {
	    (RxFifo->data[RxFifo->head]).flags |= MSG_RTR;
	}

        if( dummy & CAN_EFF ) {
	    (RxFifo->data[RxFifo->head]).flags |= MSG_EXT;
	}
	ext = (dummy & CAN_EFF);
	if(ext) {
	    (RxFifo->data[RxFifo->head]).id =
	          ((unsigned int)(CANin(minor, frame.extframe.canid1)) << 21)
		+ ((unsigned int)(CANin(minor, frame.extframe.canid2)) << 13)
		+ ((unsigned int)(CANin(minor, frame.extframe.canid3)) << 5)
		+ ((unsigned int)(CANin(minor, frame.extframe.canid4)) >> 3);
	} else {
	    (RxFifo->data[RxFifo->head]).id =
        	  ((unsigned int)(CANin(minor, frame.stdframe.canid1 )) << 3) 
        	+ ((unsigned int)(CANin(minor, frame.stdframe.canid2 )) >> 5);
	}
	/* get message length */
        dummy  &= 0x0f;			/* strip length code */ 
        (RxFifo->data[RxFifo->head]).length = dummy;

        dummy %= 9;	/* limit count to 8 bytes */
        for( i = 0; i < dummy; i++) {
            /* SLOW_DOWN_IO;SLOW_DOWN_IO; */
	    if(ext) {
		(RxFifo->data[RxFifo->head]).data[i] =
			CANin(minor, frame.extframe.canxdata[R_OFF * i]);
	    } else {
		(RxFifo->data[RxFifo->head]).data[i] =
			CANin(minor, frame.stdframe.candata[R_OFF * i]);
	    }
	}
	RxFifo->status = BUF_OK;
        RxFifo->head = (RxFifo->head+1) % RxFifo->size;

	if(RxFifo->head == RxFifo->tail) {
		printk("CAN[%d] RX: FIFO overrun", minor);
		RxFifo->status = BUF_OVERRUN;
        } 
        /*---------- kick the select() call  -*/
        /* __wake_up(struct wait_queue ** p, unsigned int mode); */
        /* __wake_up(struct wait_queue_head_t *q, unsigned int mode); */
        /*
        void wake_up(struct wait_queue**condition)                                                                  
        */
        /* This function will wake up all processes
           that are waiting on this event queue,
           that are in interruptible sleep
        */
        wake_up_interruptible(  &CanWait[minor] ); 

        CANout(minor, cancmd, CAN_RELEASE_RECEIVE_BUFFER );
        if(CANin(minor, canstat) & CAN_DATA_OVERRUN ) {
		 printk("CAN[%d] Rx: Overrun Status \n", minor);
		 CANout(minor, cancmd, CAN_CLEAR_OVERRUN_STATUS );
	}

   }

    if( irqsrc & CAN_ERROR_WARN_INT ) {
	int s;
	int err = 0;
	printk("CAN[%d]: ErrWarn!", minor);
	if (CANin(minor,canmode) & CAN_RESET_REQUEST) {
	    printk (" RES");

	    /* undo reset request */
	    CANreset(minor, canmode, CAN_RESET_REQUEST);
	}

	/* insert error */
	s = CANin(minor,canstat);
	if (s & CAN_ERROR_STATUS) {
	    printk(" ERR");
	    err = + MSG_PASSIVE;
	}
	if (s & CAN_BUS_STATUS) {
	    printk(" BUS");
	    err = + MSG_BUSOFF;
	}


	if (!err) {
	    printk (" ACT");
	    if (TxFifo->active
		&& !(irqsrc & CAN_TRANSMIT_INT)
		&& (CANin(minor, canstat) & CAN_TRANSMIT_BUFFER_ACCESS)) {

		/* If there has been a bus-off before, followed by a
		 * reset request, there is the possibility of a
		 * writable transmit buffer without SJA telling us by
		 * an interrupt. Since this would lock the tx queue,
		 * simulate a transmit interrupt now, if the queue is
		 * non-empty. */
		irqsrc |= CAN_TRANSMIT_INT;
		printk(" fake-tx-int");
	    }
	}
	else
	    (RxFifo->data[RxFifo->head]).flags += err;

	printk ("\n");

	/* do nothing, but check FIFO */
	(RxFifo->data[RxFifo->head]).id = 0xFFFFFFFF;
	/* (RxFifo->data[RxFifo->head]).length = 0; */
	/* (RxFifo->data[RxFifo->head]).data[i] = 0; */
	RxFifo->status = BUF_OK;
	RxFifo->head = (RxFifo->head+1) % RxFifo->size;
	if(RxFifo->head == RxFifo->tail) {
	    printk("CAN[%d] RX: FIFO overrun\n", minor);
            RxFifo->status = BUF_OVERRUN;
	}
    }



    /*========== transmit interrupt */
    if( irqsrc & CAN_TRANSMIT_INT ) {
	if( TxFifo->free[TxFifo->tail] == BUF_EMPTY ) {
	    /* printk("TXE\n"); */
	    TxFifo->status = BUF_EMPTY;
            TxFifo->active = 0;

	    /* wake up the user sleeping in select call; tx-queue is
	       empty, user may fill the tx-queue again */
	    wake_up_interruptible (&CanWait[minor]);
            goto Tx_done;
	} else {
	    /* printk("TX\n"); */
	}

        /* enter critical section */
        save_flags(flags);cli();


	/* if the user wants the current message to be delayed for
	   some micro-seconds, a tasklet will be registered to handle
	   the delayed sending */
	if (TxFifo->data[TxFifo->tail].timestamp.tv_usec) {
	    static struct tasklet_struct delayed_send_tasklet[MAX_CHANNELS];

	    tasklet_init (&delayed_send_tasklet[minor], delayed_send, minor);

	    tasklet_hi_schedule (&delayed_send_tasklet[minor]);
	}
	else /* no delay, send it immediately */
	    send_tx_queue_msg (minor);

        /* leave critical section */
	restore_flags(flags);
   }
Tx_done:
   /*========== error status */
    if( irqsrc & CAN_OVERRUN_INT ) {
   int s;
	printk("CAN[%d]: Overrun!\n", minor);
	Overrun[minor]++;

	/* insert error */
	s = CANin(minor,canstat);
	if(s & CAN_DATA_OVERRUN)
	    (RxFifo->data[RxFifo->head]).flags += MSG_OVR;

	(RxFifo->data[RxFifo->head]).id = 0xFFFFFFFF;
	/* (RxFifo->data[RxFifo->head]).length = 0; */
	/* (RxFifo->data[RxFifo->head]).data[i] = 0; */
	RxFifo->status = BUF_OK;
	RxFifo->head = (RxFifo->head+1) % RxFifo->size;
	if(RxFifo->head == RxFifo->tail) {
            printk("CAN[%d] RX: FIFO overrun\n", minor);
            RxFifo->status = BUF_OVERRUN;
	}

	CANout(minor, cancmd, CAN_CLEAR_OVERRUN_STATUS );
    }
   } while( (irqsrc = CANin(minor, canirq)) != 0);

/* IRQdone: */
    DBGprint(DBG_DATA, (" => leave IRQ[%d]", minor));
#ifdef CAN4LINUX_PCI
    /* Interrupt_0_Enable (bit 17) + Int_0_Reset (bit 1) */
    /*  
     Uttenthaler:
      nur 
        writel(0x00020002, Can_pitapci_control[minor] + 0x0);
      als letzte Anweisung in der ISR
     Schött:
      bei Eintritt
        writel(0x00000000, Can_pitapci_control[minor] + 0x0);
      am ende
        writel(0x00020002, Can_pitapci_control[minor] + 0x0);
    */
    writel(0x00020002, Can_pitapci_control[minor] + 0x0);
    writel(0x00020000, Can_pitapci_control[minor] + 0x0);
#endif
IRQdone_doneNothing:
    ;
#if CONFIG_TIME_MEASURE
    outb(0x00, 0x378);  
#endif
	return 1;
}


