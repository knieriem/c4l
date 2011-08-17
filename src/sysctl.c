/* can_sysctl
*
* can4linux -- LINUX CAN device driver source
* 
* Copyright (c) 2001 port GmbH Halle/Saale
* (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
*          Claus Schroeter (clausi@chemie.fu-berlin.de)
* derived from the the LDDK can4linux version
*     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
*/
/*
 * This Template implements the SYSCTL basics, and handler/strategy routines
 * Users may implement own routines and hook them up with the 'handler'		
 * and 'strategy' methods of sysctl.
 * 
 *
 */
#include "defs.h"
#include <linux/sysctl.h>

#define SYSCTL_Can 1

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
		 0444, NULL, &proc_dostring , &sysctl_string },
 { SYSCTL_CHIPSET, "Chipset", &Chipset, PROC_CHIPSET_LENGTH, 
		 0444, NULL, &proc_dostring , &sysctl_string },
 { SYSCTL_IOMODEL, "IOModel", &IOModel, MAX_CHANNELS + 1, 
		 0444, NULL, &proc_dostring , &sysctl_string },
 { SYSCTL_IRQ, "IRQ",(void *) IRQ, MAX_CHANNELS*sizeof(int), 
		 0644, NULL, &proc_dointvec , NULL  },
 { SYSCTL_BASE, "Base",(void *) Base, MAX_CHANNELS*sizeof(int), 
		 0644, NULL, &proc_dointvec , NULL  },
 { SYSCTL_BAUD, "Baud",(void *) Baud, MAX_CHANNELS*sizeof(int), 
		 0666, NULL, &proc_dointvec , NULL  },
 { SYSCTL_ACCCODE, "AccCode",(void *) AccCode, MAX_CHANNELS*sizeof(unsigned int), 
		 0644, NULL, &proc_dointvec , NULL  },
 { SYSCTL_ACCMASK, "AccMask",(void *) AccMask, MAX_CHANNELS*sizeof(unsigned int), 
		 0644, NULL, &proc_dointvec , NULL  },
 { SYSCTL_TIMEOUT, "Timeout",(void *) Timeout, MAX_CHANNELS*sizeof(int), 
		 0644, NULL, &proc_dointvec , NULL  },
 { SYSCTL_OUTC, "Outc",(void *) Outc, MAX_CHANNELS*sizeof(int), 
		 0644, NULL, &proc_dointvec , NULL  },
 { SYSCTL_TXERR, "TxErr",(void *) TxErr, MAX_CHANNELS*sizeof(int), 
		 0444, NULL, &proc_dointvec , NULL  },
 { SYSCTL_RXERR, "RxErr",(void *) RxErr, MAX_CHANNELS*sizeof(int), 
		 0444, NULL, &proc_dointvec , NULL  },
 { SYSCTL_OVERRUN, "Overrun",(void *) Overrun, MAX_CHANNELS*sizeof(int), 
		 0444, NULL, &proc_dointvec , NULL  },
 { SYSCTL_DBGMASK, "dbgMask",(void *) &dbgMask, 1*sizeof(int), 
		 0644, NULL, &proc_dointvec , NULL  },
#include "vcs.h"
 { SYSCTL_VCSREV, "vcs-rev", VCS_REV_STRING, sizeof(VCS_REV_STRING)-1,
		 0444, NULL, &proc_dostring , &sysctl_string },
#ifdef DEBUG_COUNTER
/* ---------------------------------------------------------------------- */
 { SYSCTL_CNT1, "cnt1",(void *) Cnt1, MAX_CHANNELS*sizeof(int), 
		 0444, NULL, &proc_dointvec , NULL  },
 { SYSCTL_CNT2, "cnt2",(void *) Cnt2, MAX_CHANNELS*sizeof(int), 
		 0444, NULL, &proc_dointvec , NULL  },
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


struct ctl_table_header *Can_systable;

void register_systables(void)
{
    Can_systable = register_sysctl_table( Can_sys_table, 0 );
}

void unregister_systables(void)
{
    unregister_sysctl_table(Can_systable);
}
