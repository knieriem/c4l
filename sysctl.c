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
#include <linux/version.h>
#include <linux/sysctl.h>
#include ",,sysctl.h"

#ifndef CTL_UNNUMBERED
#define CTL_UNNUMBERED 1
#endif


/* ----- global variables accessible through /proc/sys/Can */

char version[] = VERSION;
char IOModel[MAX_CHANNELS+1];
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


#include ",,sysctl.c"

static struct ctl_table_header *systable;

void register_systables(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
    systable = register_sysctl_table(sys, 0);
#else
    systable = register_sysctl_table(sys);
#endif
}

void unregister_systables(void)
{
    unregister_sysctl_table(systable);
}
