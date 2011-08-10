/* can_i82527.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * This software is released under the GPL-License.
 * Version 0.7  6 Aug 2001
 * 
 * Modified and extended to support the esd elctronic system
 * design gmbh PC/104-CAN Card (www.esd-electronics.com)
 * by Jean-Jacques Tchouto (tchouto@fokus.fraunhofer.de)
 * 
 * $Id: can_i82527.h,v 1.1 2003/08/27 13:07:44 oe Exp $
 */

//#include <can4linux.h>

int i82527_CAN_ChipReset(int board);
int i82527_CAN_SetTiming(int board, int baud);
int i82527_CAN_StartChip(int board);
int i82527_CAN_StopChip(int board);
int i82527_CAN_SetMask(int board, unsigned int code, unsigned int mask);
int i82527_CAN_SetOMode(int board, int arg);
int i82527_CAN_SendMessage(int board,  canmsg_t *tx, int wait );
int i82527_CAN_GetMessage(int board,  canmsg_t *rx);
void i82527_CAN_Interrupt(int irq, void *unused, struct pt_regs *ptregs );
int i82527_CAN_VendorInit(int minor);
int i82527_CAN_ShowStat(int board);
//for intel
int i82527_CAN_SetStdMask(int board, unsigned int mask);
int i82527_CAN_SetExtMask(int board, unsigned int mask);
int i82527_CAN_SetMsg15Mask(int board,  unsigned int mask);
int i82527_CAN_ClearObjects(int board);
int i82527_funcRegistry( struct chipfunctions_t *pchip);
int i82527_CAN_Release_io(int board);
int i82527_CAN_pre_read_config( int board );

//typedef unsigned char uint8;

extern uint8 iCanTiming[10][2];

#define CAN_RANGE 0x08




#define iCTL 0x00		// Control Register
#define iSTAT 0x01		// Status Register
#define iCPU 0x02		// CPU Interface Register
#define iHSR 0x04		// High Speed Read
#define iSGM0 0x06		// Standard Global Mask byte 0
#define iSGM1 0x07
#define iEGM0 0x08		// Extended Global Mask byte 0
#define iEGM1 0x09
#define iEGM2 0x0a
#define iEGM3 0x0b
#define i15M0 0x0c		// Message 15 Mask byte 0
#define i15M1 0x0d
#define i15M2 0x0e
#define i15M3 0x0f
#define iCLK 0x1f		// Clock Out Register
#define iBUS 0x2f		// Bus Configuration Register
#define iBT0 0x3f		// Bit Timing Register byte 0
#define iBT1 0x4f
#define iIRQ 0x5f		// Interrupt Register
#define iP1C 0x9f		// Port 1 Register
#define iP2C 0xaf		// Port 2 Register
#define iP1I 0xbf		// Port 1 Data In Register
#define iP2I 0xcf		// Port 2 Data In Register
#define iP1O 0xdf		// Port 1 Data Out Register
#define iP2O 0xef		// Port 2 Data Out Register
#define iSRA 0xff		// Serial Reset Address

#define iMSGCTL0	0x00	/* First Control register */
#define iMSGCTL1	0x01	/* Second Control register */
#define iMSGID0		0x02	/* First Byte of Message ID */
#define iMSGID1		0x03
#define iMSGID2		0x04
#define iMSGID3		0x05
#define iMSGCFG		0x06	/* Message Configuration */
#define iMSGDAT0	0x07	/* First Data Byte */
#define iMSGDAT1	0x08
#define iMSGDAT2	0x09
#define iMSGDAT3	0x0a
#define iMSGDAT4	0x0b
#define iMSGDAT5	0x0c
#define iMSGDAT6	0x0d
#define iMSGDAT7	0x0e

/* object base address */
enum i82527_OBJ_BASE_ADDR {
  obj_base_addr_tx =  0x10 , // msgobj 1
  obj_base_addr_rx =  0xf0  // msgobj 15
};


/* Control Register (0x00) */
enum i82527_iCTL {
	iCTL_INI = 1,		// Initialization
	iCTL_IE  = 1<<1,	// Interrupt Enable
	iCTL_SIE = 1<<2,	// Status Interrupt Enable
	iCTL_EIE = 1<<3,	// Error Interrupt Enable
	iCTL_CCE = 1<<6		// Change Configuration Enable
};

/* Status Register (0x01) */
enum i82527_iSTAT {
	iSTAT_TXOK = 1<<3,	// Transmit Message Successfully
	iSTAT_RXOK = 1<<4,	// Receive Message Successfully
	iSTAT_WAKE = 1<<5,	// Wake Up Status
	iSTAT_WARN = 1<<6,	// Warning Status
	iSTAT_BOFF = 1<<7	// Bus Off Status
};

/* CPU Interface Register (0x02) */
enum i82527_iCPU {
	iCPU_CEN = 1,		// Clock Out Enable
	iCPU_MUX = 1<<2,	// Multiplex
	iCPU_SLP = 1<<3,	// Sleep
	iCPU_PWD = 1<<4,	// Power Down Mode
	iCPU_DMC = 1<<5,	// Divide Memory Clock
	iCPU_DSC = 1<<6,	// Divide System Clock
	iCPU_RST = 1<<7,	// Hardware Reset Status

	int_cpu_reg = (iCPU_DMC | iCPU_DSC | iCPU_CEN ) //e.g cpu config   
};

/* Clock Out Register (0x1f) */
enum i82527_iCLK {
	iCLK_CD0 = 1,		// Clock Divider bit 0
	iCLK_CD1 = 1<<1,
	iCLK_CD2 = 1<<2,
	iCLK_CD3 = 1<<3,
	iCLK_SL0 = 1<<4,	// Slew Rate bit 0
	iCLK_SL1 = 1<<5
};

/* Bus Configuration Register (0x2f) */
enum i82527_iBUS {
	iBUS_DR0 = 1,		// Disconnect RX0 Input
	iBUS_DR1 = 1<<1,	// Disconnect RX1 Input
	iBUS_DT1 = 1<<3,	// Disconnect TX1 Output
	iBUS_POL = 1<<5,	// Polarity
	iBUS_CBY = 1<<6		// Comparator Bypass
};

#define RESET 1			// Bit Pair Reset Status
#define SET 2			// Bit Pair Set Status
#define UNCHANGED 3		// Bit Pair Unchanged

/* Message Control Register 0 (Base Address + 0x0) */
enum i82527_iMSGCTL0 {
	INTPD_SET = SET,		// Interrupt pending
	INTPD_RES = RESET,		// No Interrupt pending
	INTPD_UNC = UNCHANGED,
	RXIE_SET  = SET<<2,		// Receive Interrupt Enable
	RXIE_RES  = RESET<<2,		// Receive Interrupt Disable
	RXIE_UNC  = UNCHANGED<<2,
	TXIE_SET  = SET<<4,		// Transmit Interrupt Enable
	TXIE_RES  = RESET<<4,		// Transmit Interrupt Disable
	TXIE_UNC  = UNCHANGED<<4,
	MVAL_SET  = SET<<6,		// Message Valid
	MVAL_RES  = RESET<<6,		// Message Invalid
	MVAL_UNC  = UNCHANGED<<6
};

/* Message Control Register 1 (Base Address + 0x01) */
enum i82527_iMSGCTL1 {
	NEWD_SET = SET,			// New Data
	NEWD_RES = RESET,		// No New Data
	NEWD_UNC = UNCHANGED,
	MLST_SET = SET<<2,		// Message Lost
	MLST_RES = RESET<<2,		// No Message Lost
	MLST_UNC = UNCHANGED<<2,
	CPUU_SET = SET<<2,		// CPU Updating
	CPUU_RES = RESET<<2,		// No CPU Updating
	CPUU_UNC = UNCHANGED<<2,
	TXRQ_SET = SET<<4,		// Transmission Request
	TXRQ_RES = RESET<<4,		// No Transmission Request
	TXRQ_UNC = UNCHANGED<<4,
	RMPD_SET = SET<<6,		// Remote Request Pending
	RMPD_RES = RESET<<6,		// No Remote Request Pending
	RMPD_UNC = UNCHANGED<<6
};

/* the base address register array */
extern int Base[];

/* Message Configuration Register (Base Address + 0x06) */
enum i82527_iMSGCFG {
	MCFG_XTD = 1<<2,		// Extended Identifier
	MCFG_DIR = 1<<3			// Direction is Transmit
};

/* the timings are valid for clock 8Mhz */
#define CAN_TIM0_10K		  49
#define CAN_TIM1_10K		0x1c
#define CAN_TIM0_20K		  24	
#define CAN_TIM1_20K		0x1c
#define CAN_TIM0_40K		0x89	/* Old Bit Timing Standard of port */
#define CAN_TIM1_40K		0xEB	/* Old Bit Timing Standard of port */
#define CAN_TIM0_50K		   9
#define CAN_TIM1_50K		0x1c
#define CAN_TIM0_100K              4    /* sp 87%, 16 abtastungen, sjw 1 */
#define CAN_TIM1_100K           0x1c
#define CAN_TIM0_125K		   3
#define CAN_TIM1_125K		0x1c
#define CAN_TIM0_250K		   1
#define CAN_TIM1_250K		0x1c
#define CAN_TIM0_500K		   0
#define CAN_TIM1_500K		0x1c
#define CAN_TIM0_800K		   0
#define CAN_TIM1_800K		0x16
#define CAN_TIM0_1000K		   0
#define CAN_TIM1_1000K		0x14
