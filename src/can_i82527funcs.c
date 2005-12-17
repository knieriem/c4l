/*
 * 82527funcs.c
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * This software is released under the GPL-License.
 * Version 0.7  6 Aug 2001
 * 
 * Modified and extended to support the esd elctronic system
 * design gmbh PC/104-CAN Card (www.esd-electronics.com)
 * by Jean-Jacques Tchouto (tchouto@fokus.fraunhofer.de), 2003
 *
 * $Id: can_i82527funcs.c,v 1.1 2003/08/27 13:07:44 oe Exp $
 *
 */

#include <can_defs.h>
#include "can_i82527.h"


inline void i82527_irq_read_handler( int minor, msg_fifo_t *RxFifo );

inline void i82527_irq_write_handler( int minor, msg_fifo_t *TxFifo);

void i82527_irq_rtr_handler( int minor );


/* int	CAN_Open = 0; */

/* timing values */

uint8 iCanTiming[10][2]={
	{CAN_TIM0_10K,  CAN_TIM1_10K},
	{CAN_TIM0_20K,  CAN_TIM1_20K},
	{CAN_TIM0_40K,  CAN_TIM1_40K},
	{CAN_TIM0_50K,  CAN_TIM1_50K},
	{CAN_TIM0_100K, CAN_TIM1_100K},
	{CAN_TIM0_125K, CAN_TIM1_125K},
	{CAN_TIM0_250K, CAN_TIM1_250K},
	{CAN_TIM0_500K, CAN_TIM1_500K},
	{CAN_TIM0_800K, CAN_TIM1_800K},
	{CAN_TIM0_1000K,CAN_TIM1_1000K}};





/* Board reset
 * means the following procedure:
 * set Reset Request
 * wait for Rest request is changing - used to see if board is available
 * initialize board (with valuse from /proc/sys/Can)
 *   set output control register
 *   set timings
 *   set acceptance mask
 */

int i82527_CAN_ChipReset (int board)
{
uint8 status;
//int i;

    DBGin("CAN_ChipReset");
    DBGprint(DBG_DATA,(" INT 0x%x\n", CANin(board, iIRQ)));


    // CANout(board, iCPU,  (iCPU_DMC | iCPU_DSC | iCPU_CEN) );
    CANout(board, iCPU,  int_cpu_reg );
    DBGprint(DBG_DATA,("pc104_200 reset:  Reset Status=0x%x\n",
		       CANin(board, iCPU) & 0x80 ));
  
  
  //for(i = 0; i < 100; i++) SLOW_DOWN_IO;

    status = CANin(board, iCPU);

    DBGprint(DBG_DATA,("status=0x%x ctl=0x%x", status,
	    CANin(board, iCTL) ));
    /* if( ! (CANin(board, iCPU) & iCPU_RST ) ){
      
      printk("ERROR: no board present!");
      //MOD_DEC_USE_COUNT;
      DBGout();
      return -1;

      }*/

    DBGprint(DBG_DATA, ("[%d] CAN_CPU 0x%x\n", board, CANin(board, iCPU)));

    // Configure cpu interface  
    //  CANout(board, iCPU, (iCPU_DMC | iCPU_DSC | iCPU_CEN) );    
  CANout(board, iCPU, int_cpu_reg );    
  // Enable configuration
  CANout(board, iCTL, ( iCTL_CCE | iCTL_INI ));    

  // Set clock out slew rates
  CANout(board,iCLK, iCLK_SL1 | iCLK_CD1 );    

  // Bus configuration
  CANout(board, iBUS, iBUS_CBY );    

  // Clear error status register 
  CANout(board, iSTAT, 0x00 );    



  CANout(board,(MSG_OFFSET(1) + iMSGDAT1 ), 0x25);
  CANout(board,(MSG_OFFSET(2) + iMSGDAT3 ), 0x52);
  CANout(board,(MSG_OFFSET(10) + iMSGDAT6 ), 0xc3);  

  
  if ( ( CANin(board, (MSG_OFFSET( 1 ) + iMSGDAT1 )) != 0x25 ) ||
       ( CANin(board,  (MSG_OFFSET( 2 ) + iMSGDAT3 )) != 0x52 ) ||
       ( CANin(board,   (MSG_OFFSET( 10 ) + iMSGDAT6 )) != 0xc3 )
       ) {
    DBGprint(DBG_DATA,("Could not read back from the hardware."));
    DBGprint(DBG_DATA,("This probably means that your hardware is not correctly configured!\n"));
    return -1;
  }
  else {
    DBGprint(DBG_DATA,("Could read back, hardware is probably configured correctly"));
  }
  
  unsigned short flags=0;

  flags = (CANin(board, iCTL ) & ( iCTL_IE | iCTL_SIE | iCTL_EIE ) );

  CANout(board, iCTL, (flags | iCTL_CCE));    


  DBGprint(DBG_DATA, ("[%d] CAN_CTL 0x%x\n", board, CANin(board, iCTL)));
    

  i82527_CAN_SetTiming(board, Baud[board]);
  DBGprint(DBG_DATA, ("[%d] CAN_CTL 0x%x\n", board, CANin(board, iCTL)));
  
  i82527_CAN_SetStdMask  (board,  StdMask[board] );
  DBGprint(DBG_DATA, ("[%d] CAN_CTL 0x%x\n", board, CANin(board, iCTL)));

  i82527_CAN_SetExtMask  (board, ExtMask[board] );
  DBGprint(DBG_DATA, ("[%d] CAN_CTL 0x%x\n", board, CANin(board, iCTL)));

  i82527_CAN_SetMsg15Mask(board, Msg15Mask[board]);
  
  i82527_CAN_ClearObjects(board);

  flags = (CANin(board, iCTL ) & ( iCTL_IE | iCTL_SIE | iCTL_EIE ) );

  CANout(board, iCTL, flags);    

  DBGprint(DBG_DATA, ("[%d] CAN_CPU 0x%x\n", board, CANin(board, iCPU)));
  
  /* Can_dump(board); */
  DBGout();
  return 0;
}


int i82527_CAN_pre_read_config(int board)
{
  
  if ( Extended[board] ) {
    CANout(board, (obj_base_addr_rx + iMSGCFG ), MCFG_XTD);
  }
  else {
    CANout(board, (obj_base_addr_rx + iMSGCFG ), 0x00);
  }
  
  CANout(board, (obj_base_addr_rx + iMSGCTL1 ), ( NEWD_RES | MLST_RES | TXRQ_RES | RMPD_RES ));
  CANout(board, (obj_base_addr_rx + iMSGCTL0 ), ( MVAL_SET | TXIE_RES | RXIE_SET | INTPD_RES ));

  return 0;
  
}

int i82527_CAN_SetTiming (int board, int baud)
{
int i = 5;
int custom=0;
    DBGin("i82527_CAN_SetTiming");
    DBGprint(DBG_DATA, ("baud[%d]=%d", board, baud));
    switch(baud)
    {
	case   10: i = 0; break;
	case   20: i = 1; break;
	case   40: i = 2; break;
	case   50: i = 3; break;
	case  100: i = 4; break;
	case  125: i = 5; break;
	case  250: i = 6; break;
	case  500: i = 7; break;
	case  800: i = 8; break;
	case 1000: i = 9; break;
	default  : 
		custom=1;
    }
    if( custom ) {
      CANout(board, iBT0, (uint8) (baud >> 8) & 0xff);
      CANout(board, iBT1, (uint8) (baud & 0xff ));
      printk(" custom bit timing BT0=0x%x BT1=0x%x ",
	     CANin(board, iBT0), CANin(board, iBT1));
    } else {
      CANout(board, iBT0, (uint8) iCanTiming[i][0]);
      CANout(board, iBT1, (uint8) iCanTiming[i][1]);
    }
    DBGprint(DBG_DATA,("tim0=0x%x tim1=0x%x",
    		CANin(board, iBT0), CANin(board, iBT1)) );

    DBGout();
    return 0;
}


int i82527_CAN_StartChip (int board)
{
  int i;
  unsigned short flags = 0;
  RxErr[board] = TxErr[board] = 0L;
  DBGin("CAN_StartChip");
  DBGprint(DBG_DATA, ("[%d] CAN_CPU 0x%x\n", board, CANin(board, iCPU)));
  
  for(i=0;i<100;i++) SLOW_DOWN_IO;
  /* clear interrupts */
  CANin(board, iIRQ);
  /* Interrupts on Rx, TX, any Status change and data overrun */
  
  flags = ( CANin(board, iCTL ) | ( iCTL_IE | iCTL_SIE | iCTL_EIE ) );
  CANout(board, iCTL, flags );
  
  //CANset(board, iCTL,( iCTL_IE | iCTL_SIE | iCTL_EIE ));

  DBGprint(DBG_DATA,("start CTL=0x%x", CANin(board, iCTL)));

  DBGout();
  return 0;
}


int i82527_CAN_StopChip (int board)
{
  DBGin("CAN_StopChip");

  unsigned short flags = 0;

  flags = CANin(board, iCTL ) & ( iCTL_IE | iCTL_SIE | iCTL_EIE );
  CANout(board, iCTL, flags | ( iCTL_CCE | iCTL_INI ));
  //CANset(board, iCTL, ( iCTL_IE | iCTL_SIE | iCTL_EIE ) )
	
  DBGout();
  return 0;
}


int i82527_CAN_SetStdMask (int board, unsigned int mask)
{
  unsigned char mask0, mask1;
  
  mask0 = (unsigned char) (mask >> 3);
  mask1 = (unsigned char) (mask << 5);
    
  DBGin("CAN_SetStdMask");
  DBGprint(DBG_DATA,("[%d] iSGM0=0x%x iSGM1=0x%x",  board, mask0, mask1));
  
  CANout(board, iSGM0, mask0);	
  CANout(board, iSGM1, mask1);
  
  DBGout();
  return 0;
}

int i82527_CAN_SetExtMask(int board, unsigned int mask)
{
  unsigned char mask0, mask1, mask2, mask3;
  
  DBGin("CAN_SetExtMask");
  

  mask0 = (unsigned char) (mask >> 21);
  mask1 = (unsigned char) (mask >> 13);
  mask2 = (unsigned char) (mask >> 5);
  mask3 = (unsigned char) (mask << 3);
  
  CANout(board, iEGM0, mask0);
  CANout(board, iEGM1, mask1);
  CANout(board, iEGM2, mask2);
  CANout(board, iEGM3, mask3);
  
  DBGprint(DBG_DATA,("[%d] ExtMask=0x%x ==> iEGM0=0x%x iEGM1=0x%x iEGM2=0x%x iEGM3=0x%x" , 
		     board, (unsigned char)mask, mask0, mask1,mask2, mask3));
  DBGout();
  return 0;
}


int i82527_CAN_SetMsg15Mask(int board, unsigned int mask)
{

  DBGin("CAN_SetExtMask");
  unsigned char mask0, mask1, mask2, mask3;

  mask0 = (unsigned char) (mask >> 21);
  mask1 = (unsigned char) (mask >> 13);
  mask2 = (unsigned char) (mask >> 5);
  mask3 = (unsigned char) (mask << 3);
    
  CANout(board, i15M0, mask0);
  CANout(board, i15M1, mask1);
  CANout(board, i15M2, mask2);
  CANout(board, i15M3, mask3);

  DBGprint(DBG_DATA,("[%d] Msg15Mask=0x%x ==> i15M0=0x%x i15M1=0x%x i15M2=0x%x i15M3=0x%x" , 
		     board, (unsigned char)mask,mask0, mask1, mask2, mask3));

  DBGout();
  return 0;
}


int i82527_CAN_ClearObjects(int board)
{
  int i=0,id=0,data=0;

  DBGin("CAN_SetExtMask");
  
  
  for (i=1; i<=15; i++) {
      
    CANout(board, (MSG_OFFSET(i) + iMSGCTL0), 
	   (INTPD_RES | RXIE_RES | TXIE_RES | MVAL_RES ));
		   

    CANout(board, (MSG_OFFSET(i)+iMSGCTL1),
	   ( NEWD_RES | MLST_RES | TXRQ_RES | RMPD_RES ));
		

    for ( data=0x07; data<0x0f; data++ )
      CANout(board, (MSG_OFFSET(i) + data ), 0x00);

    for ( id=2; id<6; id++ ) {
      CANout(board, (MSG_OFFSET(i) + id ),0x00 );
    }

    if ( Extended[board] == 0) {
      CANout(board, (MSG_OFFSET(i) + iMSGCFG ),0x00);
    }
    else {
      CANout(board, (MSG_OFFSET(i) + iMSGCFG ),MCFG_XTD);
    }
  }
  if ( Extended[board] == 0 )
    {
    DBGprint(DBG_DATA,("All message ID's set to standard\n"));
    }
  else
    {
    DBGprint(DBG_DATA,("All message ID's set to extended\n"));
    }
  
  return 0;
}

/**************************************** hier weiter machen *****************/
int i82527_CAN_SendMessage (int board, canmsg_t *tx, int wait)
{

  DBGin("i82527_CAN_SendMessage");

  int i=0 , cycle = 0;
  int ext;
  uint8 id0=0,id1=0,id2=0,id3=0;

  if(wait) {
      LDDK_START_TIMER(Timeout[board]);
  }

  if( ! LDDK_TIMEDOUT  ) {
    
    /* if(tx->flags & MSG_RTR) {
      CANout(board, (obj_base_addr_tx + iMSGCTL1 ), 
	     ( RMPD_SET | TXRQ_RES | CPUU_SET | NEWD_RES ));
    }
    else {
    CANout(board, (obj_base_addr_tx + iMSGCTL1 ), 
	   ( RMPD_RES | TXRQ_RES | CPUU_SET | NEWD_RES ));
	   }*/
    CANout(board, (obj_base_addr_tx + iMSGCTL1 ), 
	   ( RMPD_RES | TXRQ_RES | CPUU_SET | NEWD_RES ));
    CANout(board, (obj_base_addr_tx + iMSGCTL0), 
	   ( MVAL_SET | TXIE_SET | RXIE_RES | INTPD_RES ));
    
    tx->length %= 9;             /*limit CAN message length to 8*/
    ext = (tx->flags & MSG_EXT); /* read message format */
    if ( ext ) {
      CANout(board, (obj_base_addr_tx + iMSGCFG ),
	     ( tx->length << 4 ) + ( MCFG_DIR | MCFG_XTD ));
      
    }
    else {
      CANout(board, (obj_base_addr_tx + iMSGCFG ), ( tx->length << 4 ) + MCFG_DIR);
    }
    
    if ( ext ) {
      id0 = (u8)( tx->id << 3 );
      id1 = (u8)( tx->id >> 5 );
      id2 = (u8)( tx->id >> 13 );
      id3 = (u8)( tx->id >> 21 );
      CANout(board, ( obj_base_addr_tx + iMSGID3 ), id0);
      CANout(board, ( obj_base_addr_tx + iMSGID2 ), id1);
      CANout(board, ( obj_base_addr_tx + iMSGID1 ), id2);
      CANout(board, ( obj_base_addr_tx + iMSGID0 ), id3);
    }
    else {
      id1 = (u8)( tx->id << 5 );
      id0 = (u8)( tx->id >> 3 );
      CANout(board, ( obj_base_addr_tx + iMSGID1 ), id1);
      CANout(board, ( obj_base_addr_tx + iMSGID0 ), id0);
    }
    
    CANout(board, ( obj_base_addr_tx + iMSGCTL1 ), 0xfa);
    
    for ( i=0; i < tx->length; i++ ) {
      CANout(board, (obj_base_addr_tx + iMSGDAT0 + i ), tx->data[ i ]);SLOW_DOWN_IO; SLOW_DOWN_IO;
      
    }
    
    if ( (tx->flags & MSG_RTR) < 0 ) {
      CANout(board, (obj_base_addr_tx + iMSGCTL1 ), 
	     ( RMPD_RES | TXRQ_RES | CPUU_RES | NEWD_SET ));
    }
    else {
      CANout(board, (obj_base_addr_tx + iMSGCTL1 ), 
	     ( RMPD_RES | TXRQ_SET | CPUU_RES | NEWD_SET ));
      
    }
    
    if( wait ) {
      while( ! (CANin(board, iSTAT) & iSTAT_TXOK) 
	     && ! LDDK_TIMEDOUT ) cycle++;
      if( LDDK_TIMEDOUT ) {
	DBGprint(DBG_DATA,("Timeout! stat=0x%x",CANin(board, iSTAT )));	
      }
    }
    else if( TxSpeed[board] == 'f' ) {
      unsigned long flags;
      /* enter critical section */
      save_flags(flags); cli();
      /* leave critical section */
      restore_flags(flags);
    }
  } 
  else {
    printk("CAN[%d] Tx: Timeout! stat=0x%x\n", board, CANin(board, iSTAT));
    i= -1;
  }
  
  if(wait) LDDK_STOP_TIMER();
  DBGout();return i;
  
    DBGout();
}


int i82527_CAN_GetMessage (int board, canmsg_t *rx )
{

  uint8 dummy = 0, msgctl1;
  int i = 0 , id0 = 0, id1 = 0;
  
  DBGin("i82527_CAN_GetMessage");
  
  LDDK_START_TIMER(Timeout[board]);
  msgctl1 = CANin(board, (obj_base_addr_rx + iMSGCTL1 ) );   // & NEWD_SET ) ) 
  
  rx->flags = 0;
  rx->length = 0;

  if( msgctl1 & MLST_SET ) { //check if message lost
    DBGprint(DBG_DATA,("Rx: overrun!\n"));
    Overrun[board]++;
  }

  if(msgctl1 & NEWD_SET) { //check if new Data available
    
    id0 = CANin(board, ( obj_base_addr_rx + iMSGID1) );
    id1 = CANin(board, ( obj_base_addr_rx + iMSGID0) ) << 8;
    rx->id = ( id0 | id1 ) >> 5;
    
    dummy = (CANin(board,(obj_base_addr_rx + iMSGCFG)) & 0xf0 ) >> 4;
    rx->length = dummy;
    DBGprint(DBG_DATA,("rx.id=0x%lx rx.length=0x%x", rx->id, dummy));
    
    dummy %= 9;
    
    for(i = 0; i < dummy; i++) {
      rx->data[i] = CANin(board, (obj_base_addr_rx + iMSGDAT0 + i ));
      DBGprint(DBG_DATA,("rx[%d]: 0x%x ('%c')",i,rx->data[i],rx->data[i] ));
    }

    // Make the chip ready to receive the next message
    CANout(board, (obj_base_addr_rx + iMSGCTL0 ),
	   ( MVAL_SET | TXIE_RES | RXIE_SET | INTPD_RES ));
		
    
    CANout(board, (obj_base_addr_rx + iMSGCTL1 ),
	   ( RMPD_RES | TXRQ_RES | MLST_RES |  NEWD_RES ));
		     
  }//end if NEWD_SET
  
  else{
    i=0;
  }
  
  LDDK_STOP_TIMER();
  DBGout();
  return i;
  
  DBGout();
}


int i82527_CAN_VendorInit (int minor)
{
    DBGin("i82527_CAN_VendorInit");

/* 1. Vendor specific part ------------------------------------------------ */
    can_range[minor] = CAN_RANGE;
    if( (VendOpt[minor] == 's') || (VendOpt[minor] == 'a')) {   
	can_range[minor] = 0x200; /*stpz board or Advantech Pcm-3680 */
    }
    if(VendOpt[minor] == 'p') {
      can_range[minor] = 0x08; /*pc104/200 board */
      DBGin("CAN_VendorInit");
    }   
/* End: 1. Vendor specific part ------------------------------------------- */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,11)
    DBGin("K. Version >= 2,3,11");
    /* can also be a compile time decision CAN_PORT_IO */
    if(IOModel[minor] == 'p') {
	/* It's port I/O */
      DBGprint(DBG_DATA,("IO-PORT = 0x%x", Base[minor]));
	if(NULL == request_region(Base[minor], CAN_RANGE, "CAN-IO" )) {
	  DBGin("Request region failed --->>>>>");
	    return -EBUSY;
	}
    } else {
	/* It's Memory I/O */
	if(NULL == request_mem_region(Base[minor], can_range[minor], "CAN-IO" )) {
	    return -EBUSY;
	}

	can_base[minor] = ioremap(Base[minor], 0x200);

    }
#else 	  /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,11) */
    
    DBGin("K. Version < 2,3,11");
/* both drivers use high memory area */
#if !defined(CONFIG_PPC) && !defined(CAN4LINUX_PCI)
#if PC104_200
    DBGin("PC104_200 defined");
    if( check_region(Base[minor], can_range[minor] ) ) {
      DBGin("check_region failed !!!");
#else
    DBGin("PC104_200 not defined");
    if( check_region(Base[minor], CAN_RANGE ) ) {
#endif
		     /* MOD_DEC_USE_COUNT; */
      DBGin("check region failure 2");
		     DBGout();
		     return -EBUSY;
    } else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,0,0)
      DBGin("K. Version >= 2,0,0");
          request_region(Base[minor], can_range[minor], "CAN-IO" );
	  DBGin("requested region for K-V >= 2,0,0");
#else
	  DBGin("K. Version < 2,0,0");
          request_region(Base[minor], can_range[minor] );
	  DBGin("requested region for K-V < 2,0,0");
#endif  /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,0,0) */
    }
#endif /* !defined  ... */
    
    /* we don't need ioremap in older Kernels */
    can_base[minor] = Base[minor];
#endif  /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,11) */

    /* now the virtual address can be used for the register address macros */



/* End: 2. Vendor specific part ------------------------------------------- */
 
DBGin("setup IRQ");

    if( IRQ[minor] > 0 ) {
#if PC104_200
      CANactivate_irq(minor, IRQ[minor]);
      int i;
      for(i=0;i<500;i++) SLOW_DOWN_IO;
#endif
        if( Can_RequestIrq( minor, IRQ[minor] , pchip->CAN_Interrupt) ) {
	     printk("Can[%d]: Can't request IRQ %d \n", minor, IRQ[minor]);
	     DBGout(); return -EBUSY;
        }
	
    }
    CANprintReg(minor);
    DBGin("CAN_VendOpt going out");

    DBGout(); return 1;
}


/*
 * The plain i82527 interrupt
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

void i82527_CAN_Interrupt ( int irq, void *dev_id, struct pt_regs *ptregs )
{
  //not implemented
  DBGin("i82527_CAN_Interrupt ----------------->");
  int minor;

  minor = *(int *)dev_id;

  //int id0=0, id1=0, id2=0, id3=0;
  u8 idxobj = 0, irqreg = 0;

  u8 status;

  unsigned short flags = 0;

  msg_fifo_t   *RxFifo; 
  msg_fifo_t   *TxFifo;
  	
#if CAN_USE_FILTER
  msg_filter_t *RxPass;
  unsigned int id;
#endif 
#if 0
  int first = 0;
#endif 
  
  RxFifo = &Rx_Buf[minor]; 
  TxFifo = &Tx_Buf[minor];

#if CAN_USE_FILTER
  RxPass = &Rx_Filter[minor];
#endif 

  //read the content of the IRQ reg
  irqreg = CANin(minor, iIRQ);
  
  //read the STATUS reg
  status = CANin(minor, iSTAT);

  //reset status register
  CANout(minor, iSTAT, 0);
  
  DBGprint(DBG_DATA,("IRQ handler: irqreg=0x%x  statusreg=0x%x", irqreg, status));

  while( irqreg ) {
    
    // Handle change in status register
    if ( irqreg == 0x01 ) {
      DBGprint(DBG_DATA,( "Status register: 0x%x\n", status ));
      
      if ( status & iSTAT_WAKE ) {
	//must be implement here for chip statistic
      }
      
      if ( status & iSTAT_WARN ) {
	// There is an abnormal # of errors
	//must be implement here for chip statistic
	DBGprint(DBG_DATA,("82527_irq_handler: Bus Warning\n" ));

	(RxFifo->data[RxFifo->head]).flags += MSG_PASSIVE;
      }
      
      if ( status & iSTAT_BOFF ) {
	// We have a bus off condition
	(RxFifo->data[RxFifo->head]).flags += MSG_BUSOFF; 
	
	flags = ( CANin(minor, iCTL ) | ( iCTL_IE | iCTL_SIE | iCTL_EIE ) );
	flags &= ~iCTL_INI; // Reset init flag
	CANout(minor, flags, iCTL );    

	(RxFifo->data[RxFifo->head]).id = 0xFFFFFFFF;
	
	RxFifo->status = BUF_OK;
	RxFifo->head = ++(RxFifo->head) % RxFifo->size;
	if(RxFifo->head == RxFifo->tail) {
	  printk("CAN[%d] RX: FIFO overrun\n", minor);
	  RxFifo->status = BUF_OVERRUN;
	}
	
	DBGprint(DBG_DATA,("82527_irq_handler: Bus Off\n" ));
      }

      
      
    }//end if irqreg == 0x01
    else {
      
      if ( irqreg == 0x02 ) {
	idxobj = 14;
      }
      else {
	idxobj = irqreg - 3;
      }
      
      if ( 0 == idxobj ) {
	//   * * * Transmitt * * *
	i82527_irq_write_handler( minor, TxFifo ); 
      }
      else if (14 == idxobj ) {
      
	DBGprint(DBG_DATA,("Want to Receive message ---->"));
	i82527_irq_read_handler(minor, RxFifo);
      }
      else {
	//   * * * Some other strange interrupt * * *
      }
      
    }//end else 

    // Get irq status again
    irqreg = CANin(minor, iIRQ );

  }//end while irqreg

  DBGout();
  return ;
}


void i82527_irq_write_handler(int minor, msg_fifo_t *TxFifo ) {

  DBGin( "i82527_irq_write_handler"); 
  
  unsigned long flags;
  uint8 tx2reg;
  unsigned int id;
  int ext;			/* flag for extended message format */
  int i;
  uint8 id0=0,id1=0,id2=0,id3=0;

  CANout(minor,( obj_base_addr_tx + iMSGCTL0 ), 
	 ( MVAL_RES | TXIE_RES | RXIE_RES | INTPD_RES ));
  
  
  /* enter critical section */
  save_flags(flags);cli();

  if( TxFifo->free[TxFifo->tail] == BUF_EMPTY ) {
    /* printk("TXE\n"); */
    /* leave critical section */
    restore_flags(flags);
    TxFifo->status = BUF_EMPTY;
    TxFifo->active = 0;
    CANactivateIRQline(minor);

    return;
  }

  CANout(minor, (obj_base_addr_tx + iMSGCTL1 ), 
	 ( RMPD_RES | TXRQ_RES | CPUU_SET | NEWD_RES ));
    
  CANout(minor, (obj_base_addr_tx + iMSGCTL0), 
	 ( MVAL_SET | TXIE_SET | RXIE_RES | INTPD_RES ));

  tx2reg = (TxFifo->data[TxFifo->tail]).length;
  
  /* if( (TxFifo->data[TxFifo->tail]).flags & MSG_RTR) {
    tx2reg |= CAN_RTR;
    }*/

  ext = (TxFifo->data[TxFifo->tail]).flags & MSG_EXT;
  id = (TxFifo->data[TxFifo->tail]).id;
  
  if ( ext ) {
    CANout(minor, (obj_base_addr_tx + iMSGCFG ),
	   (tx2reg << 4 ) + ( MCFG_DIR | MCFG_XTD ));
  }
  else {
    CANout(minor, (obj_base_addr_tx + iMSGCFG ), ( tx2reg << 4 ) + MCFG_DIR);
  }
  
  if ( ext ) {
      id0 = (uint8)( id << 3 );
      id1 = (uint8)( id >> 5 );
      id2 = (uint8)( id >> 13 );
      id3 = (uint8)( id >> 21 );
      CANout(minor, ( obj_base_addr_tx + iMSGID3 ), id0);
      CANout(minor, ( obj_base_addr_tx + iMSGID2 ), id1);
      CANout(minor, ( obj_base_addr_tx + iMSGID1 ), id2);
      CANout(minor, ( obj_base_addr_tx + iMSGID0 ), id3);
    }
    else {
      id1 = (uint8)( id << 5 );
      id0 = (uint8)( id >> 3 );
      CANout(minor, ( obj_base_addr_tx + iMSGID1 ), id1);
      CANout(minor, ( obj_base_addr_tx + iMSGID0 ), id0);
    }
    
  CANout(minor, ( obj_base_addr_tx + iMSGCTL1 ), 0xfa); //set direction = transmit
    
  tx2reg &= 0x0f; //restore length only
    for ( i=0; i < tx2reg; i++ ) {
      CANout(minor, (obj_base_addr_tx + iMSGDAT0 + i ), 
	     (TxFifo->data[TxFifo->tail]).data[i]);
      
    }
    
    if ( ((TxFifo->data[TxFifo->tail]).flags  & MSG_RTR) < 0 ) {
      CANout(minor, (obj_base_addr_tx + iMSGCTL1 ), 
	     ( RMPD_RES | TXRQ_RES | CPUU_RES | NEWD_SET ));
    }
    else {
      CANout(minor, (obj_base_addr_tx + iMSGCTL1 ), 
	     ( RMPD_RES | TXRQ_SET | CPUU_RES | NEWD_SET ));
      
    }
  
    TxFifo->free[TxFifo->tail] = BUF_EMPTY; /* now this entry is EMPTY */
    TxFifo->tail = ++(TxFifo->tail) % TxFifo->size;
  
    /* leave critical section */
    restore_flags(flags);

    CANactivateIRQline(minor);
} 



void i82527_irq_read_handler( int minor, msg_fifo_t *RxFifo ) 
{
  uint8 dummy = 0 , ctl1reg=0;
  int i = 0 ,  bDataAvail = 1;
  int ext;
  int id0=0, id1=0, id2=0, id3=0;
  DBGin("i82527_irq_read_handler");
  

  /*  
      must be implemented 
      #if CAN_USE_FILTER
      msg_filter_t *RxPass;
      unsigned int id;
      RxPass = &Rx_Filter[minor];
      #endif 
  */
  while( bDataAvail ) {
  
    /* fill timestamp as first action */
    do_gettimeofday(&(RxFifo->data[RxFifo->head]).timestamp);

    dummy = CANin(minor, (obj_base_addr_rx + iMSGCFG));
		  
    ctl1reg = CANin(minor, (obj_base_addr_rx + iMSGCTL1));

    DBGprint(DBG_DATA, ("iCTL1 = 0x%x", ctl1reg));

    if(ctl1reg & RMPD_SET) {
       DBGprint(DBG_DATA, ("Receive RTR Message --->>"));
      (RxFifo->data[RxFifo->head]).flags |= MSG_RTR; 
    }
    if(dummy & MCFG_XTD) {
      (RxFifo->data[RxFifo->head]).flags |= MSG_EXT; 
    }
    
    ext = dummy & MCFG_XTD;

    if( ext ) {
      id0 = (unsigned int)(CANin(minor, (obj_base_addr_rx + iMSGID3))); 
      id1 = (unsigned int)(CANin(minor, (obj_base_addr_rx + iMSGID2))) << 8;
      id2 = (unsigned int)(CANin(minor, (obj_base_addr_rx + iMSGID1))) << 16;
      id3 = (unsigned int)(CANin(minor, (obj_base_addr_rx + iMSGID0))) << 24;
      
      (RxFifo->data[RxFifo->head]).id =(id0 | id1 | id2 | id3) >> 3;
	
    } else {
      
      id0 = (unsigned int)(CANin(minor, (obj_base_addr_rx + iMSGID1))); 
      id1 = (unsigned int)(CANin(minor, (obj_base_addr_rx + iMSGID0))) << 8;
      (RxFifo->data[RxFifo->head]).id =(id0 | id1 ) >> 5;

    }

    dummy  &= 0xf0;/* strip length code */ 
    dummy  = dummy >>4;

    (RxFifo->data[RxFifo->head]).length = dummy;
    dummy %= 9;	/* limit count to 8 bytes */
    DBGprint(DBG_DATA, ("dummy= %d\n",dummy));


    
    for( i = 0; i < dummy; i++) {
      SLOW_DOWN_IO;SLOW_DOWN_IO;
      (RxFifo->data[RxFifo->head]).data[i]  =
	CANin(minor, (obj_base_addr_rx + iMSGDAT0 + i ));
      DBGprint(DBG_DATA,("rx[%d]: 0x%x ('%c')",i,
			 (RxFifo->data[RxFifo->head]).data[i],
			 (RxFifo->data[RxFifo->head]).data[i] ));
    }
    
    RxFifo->status = BUF_OK;

    RxFifo->head = ++(RxFifo->head) % RxFifo->size;

   
    if(RxFifo->head == RxFifo->tail) {
      printk("CAN[%d] RX: FIFO overrun\n", minor);
      RxFifo->status = BUF_OVERRUN;
    } 
    
    wake_up_interruptible(  &CanWait[minor] ); 

    // Make the chip ready to receive the next message
    CANout(minor, (obj_base_addr_rx + iMSGCTL0 ),
	   ( MVAL_SET | TXIE_RES | RXIE_SET | INTPD_RES ));
    CANout(minor, (obj_base_addr_rx + iMSGCTL1 ),
	   ( RMPD_RES | TXRQ_RES | MLST_RES |  NEWD_RES ));

    // Check if new data arrived
    if ( !( ( bDataAvail = CANin(minor,(obj_base_addr_rx + iMSGCTL1 )) ) & NEWD_SET ) ) {
      break;
    }
    
    if ( bDataAvail & MLST_SET ) {
      DBGprint(DBG_DATA,("82527 fifo full: Message lost!\n"));
    }
  }
  
  DBGout();
  
}



int i82527_CAN_ShowStat(int board)
{
  //not implemented
  return -1;
}

int i82527_CAN_SetMask(int board, unsigned int code, unsigned int mask)
{
  //not implemented
  return -1;
}

int i82527_CAN_SetOMode(int board,int arg )
{
  //not inplemented
  return -1;
}


int i82527_CAN_Release_io(int board)
{
  unsigned i;
  DBGin("i82527_CAN_Release_io");  
  // disable IRQ generation 
  CANout(board,iCTL, iCTL_CCE );
  
  // clear all message objects 
  for (i=1; i<=15; i++) {
    CANout(board, (i*0x10 + iMSGCTL0 ), 
	  (INTPD_RES | RXIE_RES | TXIE_RES | MVAL_RES));

    CANout(board, (i*0x10 + iMSGCTL1 ),
	  ( NEWD_RES | MLST_RES | CPUU_RES | TXRQ_RES | RMPD_RES));
			 
  }
  
  release_region(Base[board],CAN_RANGE);
  DBGout();
  return 0;
  
}


int i82527_funcRegistry(struct chipfunctions_t *pchip)
{
  pchip->CAN_ChipReset = i82527_CAN_ChipReset;
  pchip->CAN_SetTiming = i82527_CAN_SetTiming ;
  pchip->CAN_StartChip =i82527_CAN_StartChip ;
  pchip->CAN_StopChip =i82527_CAN_StopChip;
  pchip->CAN_SetMask = i82527_CAN_SetMask;
  pchip->CAN_SetOMode = i82527_CAN_SetOMode;
  pchip->CAN_SendMessage =  i82527_CAN_SendMessage;
  pchip->CAN_GetMessage = i82527_CAN_GetMessage;
  pchip->CAN_Interrupt = i82527_CAN_Interrupt;
  pchip->CAN_VendorInit = i82527_CAN_VendorInit;
  pchip-> CAN_ShowStat= i82527_CAN_ShowStat;
  
  pchip->CAN_SetStdMask = i82527_CAN_SetStdMask;
  pchip->CAN_SetExtMask = i82527_CAN_SetExtMask;
  pchip->CAN_SetMsg15Mask = i82527_CAN_SetMsg15Mask;
  pchip->CAN_ClearObjects =i82527_CAN_ClearObjects ;

  pchip->CAN_Release_io = i82527_CAN_Release_io ;
  pchip->CAN_pre_read_config = i82527_CAN_pre_read_config ;
  return 0;
} 
