/* Can_error
*
* can4linux -- LINUX CAN device driver source
* 
* Copyright (c) 2001 port GmbH Halle/Saale
* (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
*          Claus Schroeter (clausi@chemie.fu-berlin.de)
* derived from the the LDDK can4linux version
*     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
*------------------------------------------------------------------
* $Header: /z2/cvsroot/products/0530/software/can4linux/src/Can_error.c,v 1.3 2001/09/14 14:58:09 oe Exp $
*
*--------------------------------------------------------------------------
*
*
* modification history
* --------------------
* $Log: Can_error.c,v $
* Revision 1.3  2001/09/14 14:58:09  oe
* first free release
*
*
*/
#include <can_defs.h>

int Can_errno = 0;


int Error(int err)
{
    Can_errno = err;
    return 0;
}
