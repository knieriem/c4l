/* can_sysctl
*
* can4linux -- LINUX CAN device driver source
* 
* Copyright (c) 2001 port GmbH Halle/Saale
* (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
*          Claus Schroeter (clausi@chemie.fu-berlin.de)
* derived from the the LDDK can4linux version
*     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
*------------------------------------------------------------------
* $Header: /z2/cvsroot/products/0530/software/can4linux/src/can_sysctl.c,v 1.9 2003/08/27 13:06:27 oe Exp $
*
*--------------------------------------------------------------------------
*
*
* modification history
* --------------------
* $Log: can_sysctl.c,v $
* Revision 1.9  2003/08/27 13:06:27  oe
* - Version 3.0
*
* Revision 1.8  2003/07/05 14:28:55  oe
* - all changes for the new 3.0: try to eliminate hw depedencies at run-time.
*   configure for HW at compile time
*
* Revision 1.7  2002/10/11 16:58:06  oe
* - IOModel, Outc, VendOpt are now set at compile time
* - deleted one misleading printk()
*
* Revision 1.6  2002/08/20 05:55:01  oe
* *** empty log message ***
*
* Revision 1.5  2002/08/08 18:05:02  oe
* - no predefined controller base addresses
* - changed /proc/ ... /version to be longer, containg TARGET name
*
* Revision 1.4  2001/11/20 16:43:52  oe
* *** empty log message ***
*
* Revision 1.3  2001/09/14 14:58:09  oe
* first free release
*
*
*/
/*
 * This Template implements the SYSCTL basics, and handler/strategy routines
 * Users may implement own routines and hook them up with the 'handler'		
 * and 'strategy' methods of sysctl.
 * 
 *
 */
#include "defs.h"
#include <linux/mm.h>
#include <linux/sysctl.h>
#include <linux/ctype.h>
/* #include <can_sysctl.h> */

#define Can_dointvec proc_dointvec
#define Can_dostring proc_dostring
#define Can_sysctl_string sysctl_string

#define SYSCTL_Can 1

/* ----- Prototypes */

extern int Can_dointvec(ctl_table *table, int write, struct file *filp,
		  void *buffer, size_t *lenp);

extern int Can_dostring(ctl_table *table, int write, struct file *filp,
		  void *buffer, size_t *lenp);


extern int Can_sysctl_string(ctl_table *table, int *name, int nlen,
		  void *oldval, size_t *oldlenp,
		  void *newval, size_t newlen, void **context);


/* ----- global variables accessible through /proc/sys/Can */

char version[] = VERSION;
char IOModel[MAX_CHANNELS];
char Chipset[] =
#if defined(ATCANMINI_PELICAN)
	"SJA1000"
#elif defined(CPC_PCI)
	"SJA1000"
#elif defined(IME_SLIMLINE)
	"SJA1000"
#elif defined(PCM3680)
	"SJA1000"
#elif defined(IXXAT_PCI03)
	"SJA1000"
#else
	""
#endif
;

int IRQ[MAX_CHANNELS] = { 0x0 };
/* dont assume a standard address, always configure,
 * address == 0 means no board available */
int Base[MAX_CHANNELS] = { 0x0 };
int Baud[MAX_CHANNELS];
unsigned int AccCode[MAX_CHANNELS];
unsigned int AccMask[MAX_CHANNELS];
int Timeout[MAX_CHANNELS];
/* predefined value of the output control register,
* depends of TARGET set by Makefile */
int Outc[MAX_CHANNELS];
int TxErr[MAX_CHANNELS]   = { 0x0 };
int RxErr[MAX_CHANNELS]   = { 0x0 };
int Overrun[MAX_CHANNELS] = { 0x0 };

#ifdef DEBUG_COUNTER
int Cnt1[MAX_CHANNELS]    = { 0x0 };
int Cnt2[MAX_CHANNELS]    = { 0x0 };
#endif /* DEBUG_COUNTER */

/* ----- the sysctl table */

ctl_table Can_sysctl_table[] = {
 { SYSCTL_VERSION, "version", &version, PROC_VER_LENGTH, 
		 0444, NULL, &Can_dostring , &Can_sysctl_string },
 { SYSCTL_CHIPSET, "Chipset", &Chipset, PROC_CHIPSET_LENGTH, 
		 0444, NULL, &Can_dostring , &Can_sysctl_string },
 { SYSCTL_IOMODEL, "IOModel", &IOModel, MAX_CHANNELS + 1, 
		 0444, NULL, &Can_dostring , &Can_sysctl_string },
 { SYSCTL_IRQ, "IRQ",(void *) IRQ, MAX_CHANNELS*sizeof(int), 
		 0644, NULL, &Can_dointvec , NULL  },
 { SYSCTL_BASE, "Base",(void *) Base, MAX_CHANNELS*sizeof(int), 
		 0644, NULL, &Can_dointvec , NULL  },
 { SYSCTL_BAUD, "Baud",(void *) Baud, MAX_CHANNELS*sizeof(int), 
		 0666, NULL, &Can_dointvec , NULL  },
 { SYSCTL_ACCCODE, "AccCode",(void *) AccCode, MAX_CHANNELS*sizeof(unsigned int), 
		 0644, NULL, &Can_dointvec , NULL  },
 { SYSCTL_ACCMASK, "AccMask",(void *) AccMask, MAX_CHANNELS*sizeof(unsigned int), 
		 0644, NULL, &Can_dointvec , NULL  },
 { SYSCTL_TIMEOUT, "Timeout",(void *) Timeout, MAX_CHANNELS*sizeof(int), 
		 0644, NULL, &Can_dointvec , NULL  },
 { SYSCTL_OUTC, "Outc",(void *) Outc, MAX_CHANNELS*sizeof(int), 
		 0644, NULL, &Can_dointvec , NULL  },
 { SYSCTL_TXERR, "TxErr",(void *) TxErr, MAX_CHANNELS*sizeof(int), 
		 0444, NULL, &Can_dointvec , NULL  },
 { SYSCTL_RXERR, "RxErr",(void *) RxErr, MAX_CHANNELS*sizeof(int), 
		 0444, NULL, &Can_dointvec , NULL  },
 { SYSCTL_OVERRUN, "Overrun",(void *) Overrun, MAX_CHANNELS*sizeof(int), 
		 0444, NULL, &Can_dointvec , NULL  },
 { SYSCTL_DBGMASK, "dbgMask",(void *) &dbgMask, 1*sizeof(int), 
		 0644, NULL, &Can_dointvec , NULL  },
#include "vcs.h"
 { SYSCTL_VCSREV, "vcs-rev", VCS_REV_STRING, sizeof(VCS_REV_STRING)-1,
		 0444, NULL, &Can_dostring , &Can_sysctl_string },
#ifdef DEBUG_COUNTER
/* ---------------------------------------------------------------------- */
 { SYSCTL_CNT1, "cnt1",(void *) Cnt1, MAX_CHANNELS*sizeof(int), 
		 0444, NULL, &Can_dointvec , NULL  },
 { SYSCTL_CNT2, "cnt2",(void *) Cnt2, MAX_CHANNELS*sizeof(int), 
		 0444, NULL, &Can_dointvec , NULL  },
/* ---------------------------------------------------------------------- */
#endif /* DEBUG_COUNTER */
   {0}
};

/* ----- the main directory entry in /proc/sys */

ctl_table Can_sys_table[] = {
	    {SYSCTL_Can, "Can", NULL, 0, 0555, 
                 Can_sysctl_table},	
	    {0}	
};

/* ----- register and unregister entrys */

struct ctl_table_header *Can_systable;

void register_systables(void)
{
    Can_systable = register_sysctl_table( Can_sys_table, 0 );
}

void unregister_systables(void)
{
    unregister_sysctl_table(Can_systable);
}
