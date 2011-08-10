/* Can_debug
*
* can4linux -- LINUX CAN device driver source
* 
* Copyright (c) 2001 port GmbH Halle/Saale
* (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
*          Claus Schroeter (clausi@chemie.fu-berlin.de)
* derived from the the LDDK can4linux version
*     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
*------------------------------------------------------------------
* $Header: /z2/cvsroot/products/0530/software/can4linux/src/Can_debug.c,v 1.3 2002/08/20 05:54:06 oe Exp $
*
*--------------------------------------------------------------------------
*
*
* modification history
* --------------------
* $Log: Can_debug.c,v $
* Revision 1.3  2002/08/20 05:54:06  oe
* - print_tty() support
*
* Revision 1.2  2001/09/14 14:58:09  oe
* first free release
*
*
*/
#include "defs.h"


/* default debugging level */

#if DEBUG
# ifndef DEFAULT_DEBUG
  unsigned int   dbgMask  = \
    (DBG_ENTRY | DBG_EXIT | DBG_BRANCH | DBG_DATA | DBG_INTR | DBG_1PPL)
    & ~DBG_ALL;
# else
unsigned int   dbgMask  = 0;
# endif
#else
unsigned int   dbgMask  = 0;
#endif
