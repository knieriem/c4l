
can_defs.h:

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,1)
 extern wait_queue_head_t CanWait[];
#else
 extern struct wait_queue *CanWait[];
#endif



can_82...funcs.c:  Interrupt ISR:

        /* This function will wake up all processes
           that are waiting on this event queue,
           that are in interruptible sleep
        */
        wake_up_interruptible(  &CanWait[minor] ); 


can_select.c: select()

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
    DBGprint(DBG_BRANCH,("POLL: fifo empty,poll waiting...\n"));

    /* every event queue that could wake up the process
     * and change the status of the poll operation
     * can be added to the poll_table structure by
     * calling the function poll_wait:  
     */
                /*     _select para, wait queue, _select para */
/* X */		poll_wait(file, &CanWait[minor] , wait);

    DBGprint(DBG_BRANCH,("POLL: wait returned \n"));
    if( RxFifo->head != RxFifo->tail ) {
	/* fifo has some telegrams */
	/* Return a bit mask
	 * describing operations that could be immediately performed
	 * without blocking.
	 */
	DBGout();
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
	return POLLIN | POLLRDNORM;
    }
    DBGout();return 0;

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,3)
    DBGprint(DBG_BRANCH,("POLL: fifo empty,poll waiting...\n"));
    poll_wait(file, &CanWait[minor] , wait);
    DBGprint(DBG_BRANCH,("POLL: wait returned \n"));
    if( RxFifo->head != RxFifo->tail ) {
	/* fifo has some telegrams */
	DBGout();
	return POLLIN | POLLRDNORM;
    }
    DBGout();return 0;

#else




can_util.c:  

 /* each CAN channel has one wait_queue for read() */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,1)
 wait_queue_head_t CanWait[MAX_BUFS];
#else
 struct wait_queue *CanWait[MAX_BUFS];
#endif



int Can_WaitInit(int minor)
{
    DBGin("Can_WaitInit");
	/* reset the wait_queue pointer */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
/* soll so sein */
	init_waitqueue_head(&CanWait[minor]);
/* das geht */
	/* CanWait[minor] = NULL; */
#else
	CanWait[minor] = NULL;
#endif
    DBGout();
    return 0;
}
