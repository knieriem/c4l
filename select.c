/* can_select
*
* can4linux -- LINUX CAN device driver source
* 
* Copyright (c) 2001 port GmbH Halle/Saale
* (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
*          Claus Schroeter (clausi@chemie.fu-berlin.de)
* derived from the the LDDK can4linux version
*     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
*/

#include "defs.h"

unsigned int can_select( __LDDK_SELECT_PARAM )
{
	Dev *dev = filedev(file);

	DBGin("can_select");
	DBGprint(DBG_DATA,("minor = %d", dev->minor));
#ifdef DEBUG
    CAN_ShowStat(dev->minor);
#endif
    DBGprint(DBG_BRANCH,("POLL: fifo empty,poll waiting...\n"));

    /* every event queue that could wake up the process
     * and change the status of the poll operation
     * can be added to the poll_table structure by
     * calling the function poll_wait:  
     */
                /*     _select para, wait queue, _select para */
/* X */		poll_wait(file, &CanWait[dev->minor] , wait);

    DBGprint(DBG_BRANCH,("POLL: wait returned \n"));
    {
      int ret_mask = 0;

      if (qlen(&dev->rxq) > 0) {
	  /* fifo has some telegrams */
	  /* Return a bit mask
	   * describing operations that could be immediately performed
	   * without blocking.
	   */
	  /*
	   * POLLIN This bit must be set
	   *        if the device can be read without blocking. 
	   * POLLRDNORM This bit must be set
	   * if "normal'' data is available for reading.
	   * A readable device returns (POLLIN | POLLRDNORM)
	   *
	   *
	   *
	   */
	  ret_mask |= POLLIN | POLLRDNORM;
      }
      if (qlen(&dev->txq) <= qsize(&dev->txq)/2) {
	  /* Tx-queue is half-empty, the user may fill it again.
	   *
	   * POLLOUT This bit is set in the return value if the device
	   * can be written to without blocking.
	   *
	   * POLLWRNORM This bit has the same meaning as POLLOUT, and
	   * sometimes it actually is the same number. A writable
	   * device returns (POLLOUT | POLLWRNORM).
	   *
	   */
	  ret_mask |= POLLOUT | POLLWRNORM;
      }

      DBGout();
      return ret_mask;
    }
        
    DBGout();
    return 0;
}
