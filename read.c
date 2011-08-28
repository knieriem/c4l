/*
 * can4linux -- LINUX CAN device driver source
 * 
 * Copyright (c) 2001 port GmbH Halle/Saale
 * (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
 *          Claus Schroeter (clausi@chemie.fu-berlin.de)
 * derived from the the LDDK can4linux version
 *     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
 */

#include "defs.h"

static int copymsgtouser(canmsg_t *src, void *dst)
{
	__lddk_copy_to_user(dst, src, sizeof(canmsg_t));
	return 1;	/* consumed */
}

int can_read( __LDDK_READ_PARAM )
{
	Dev *dev = filedev(file);
	canmsg_t *addr; 
	int written;

	DBGin("can_read");

	addr = (canmsg_t *)buffer;
	
	if (!access_ok( VERIFY_WRITE, buffer, count * sizeof(canmsg_t))) {
		DBGout();
		return -EINVAL;
	}
	for (written=0; written < count; written++) {
		if (!qconsume(&dev->rxq, copymsgtouser, &addr[written])) {
			break;
		}
	}
	DBGout();
	return(written);
}
