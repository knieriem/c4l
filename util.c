/*
* can4linux -- LINUX CAN device driver source
* 
* derived from the the LDDK can4linux version
*     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
*
* (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
*          Claus Schroeter (clausi@chemie.fu-berlin.de)
*/


#include "defs.h"
#include "kapi.h"
#include <linux/sched.h> 
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include ",,sysctl.h"

#ifdef CAN_USE_FILTER
    msg_filter_t Rx_Filter[MAX_CHANNELS]; 
#endif

unsigned char *can_base[MAX_CHANNELS];		/* ioremapped adresses */

int Can_RequestIrq(Dev *dev, int irq)
{
	int err=0;

	DBGin("Can_RequestIrq");
	err = kapi_request_irq(irq, "Can", dev);
	if (!err) {
		/* printk("Requested IRQ[%d]: %d @ 0x%x", dev->minor, irq, handler); */
		DBGprint(DBG_BRANCH,("Requested IRQ: %d", irq));
		dev->requestedIrq = 1;
	}
	DBGout();
	return err;
}

int Can_FreeIrq(Dev *dev, int irq)
{
	DBGin("Can_FreeIrq");
	dev->requestedIrq = 0;
	free_irq(irq, dev);
	DBGout();
	return 0;
}

int Can_WaitInit(Dev *dev)
{
	DBGin("Can_WaitInit");
	init_waitqueue_head(&dev->wq);
	DBGout();
	return 0;
}

int Can_FifoInit(Dev *dev)
{
	DBGin("Can_FifoInit");
	if (qcreate(&dev->rxq, MAX_RX_BUFSIZE, 0) == -1 || qcreate(&dev->txq, MAX_TX_BUFSIZE, 1) == -1) {
		DBGout();
		return -1;
	}

	DBGout();
	return 0;
}


int Can_FifoCleanup(Dev *dev)
{
	qfree(&dev->txq);
	qfree(&dev->rxq);
	return 0;
}


#ifdef CAN_USE_FILTER
int Can_FilterInit(int minor)
{
int i;

    DBGin("Can_FilterInit");
       Rx_Filter[minor].use      = 0;
       Rx_Filter[minor].signo[0] = 0;
       Rx_Filter[minor].signo[1] = 0;
       Rx_Filter[minor].signo[2] = 0;

       for(i=0;i<MAX_ID_NUMBER;i++)	
	  Rx_Filter[minor].filter[i].rtr_response = NULL;

    DBGout();
    return 0;
}

int Can_FilterCleanup(int minor)
{
int i;

    DBGin("Can_FilterCleanup");
    for(i=0;i<MAX_ID_NUMBER;i++) {
	    if( Rx_Filter[minor].filter[i].rtr_response != NULL )	
	       kfree( Rx_Filter[minor].filter[i].rtr_response);
	    Rx_Filter[minor].filter[i].rtr_response = NULL;
    }
    DBGout();
    return 0;
}


int Can_FilterOnOff(int minor, unsigned on) {
    DBGin("Can_FilterOnOff");
       Rx_Filter[minor].use = (on!=0);
    DBGout();
    return 0;
}

int Can_FilterMessage(int minor, unsigned message, unsigned enable) {
    DBGin("Can_FilterMessage");
       Rx_Filter[minor].filter[message].enable = (enable!=0);
    DBGout();
    return 0;
}

int Can_FilterTimestamp(int minor, unsigned message, unsigned stamp) {
    DBGin("Can_FilterTimestamp");
       Rx_Filter[minor].filter[message].timestamp = (stamp!=0);
    DBGout();
    return 0;
}

int Can_FilterSignal(int minor, unsigned id, unsigned signal) {
    DBGin("Can_FilterSignal");
       if( signal <= 3 )
       Rx_Filter[minor].filter[id].signal = signal;
    DBGout();
    return 0;
}

int Can_FilterSigNo(int minor, unsigned signo, unsigned signal ) {
    DBGin("Can_FilterSigNo");
       if( signal < 3 )
       Rx_Filter[minor].signo[signal] = signo;
    DBGout();
    return 0;
}
#endif

#ifdef CAN_RTR_CONFIG
int Can_ConfigRTR( int minor, unsigned message, canmsg_t *Tx )
{
canmsg_t *tmp;

    DBGin("Can_ConfigRTR");
    if( (tmp = kmalloc ( sizeof(canmsg_t), GFP_ATOMIC )) == NULL ){
	    DBGprint(DBG_BRANCH,("memory problem"));
	    DBGout(); return -1;
    }
    Rx_Filter[minor].filter[message].rtr_response = tmp;
    memcpy( Rx_Filter[minor].filter[message].rtr_response , Tx, sizeof(canmsg_t));	
    DBGout(); return 1;
    return 0;
}

int Can_UnConfigRTR( int minor, unsigned message )
{
canmsg_t *tmp;

    DBGin("Can_UnConfigRTR");
    if( Rx_Filter[minor].filter[message].rtr_response != NULL ) {
	    kfree(Rx_Filter[minor].filter[message].rtr_response);
	    Rx_Filter[minor].filter[message].rtr_response = NULL;
    }	
    DBGout(); return 1;
    return 0;
}
#endif


#ifdef DEBUG

/* dump_CAN or CAN_dump() which is better ?? */
#if CAN4LINUX_PCI
#else
#endif
#include <asm/io.h>

#if 0
/* simply dump a memory area bytewise for 2*16 addresses */
void dump_CAN(unsigned long adress, int offset)
{
int i, j;
unsigned long ptr = (unsigned long)ioremap(adress, 256);
    printk("     CAN at Adress 0x%x\n", adress);
    for(i = 0; i < 2; i++) {
	printk("     ");
	for(j = 0; j < 16; j++) {
	    /* printk("%02x ", *ptr++); */
	    printk("%02x ", readb(ptr));
	    ptr += offset;
	}
	printk("\n");
    }
}

void dump_CAN(unsigned long adress, int offset)
{
int i, j;
    printk("     CAN at Adress 0x%x\n", adress);
    for(i = 0; i < 2; i++) {
	printk("     ");
	for(j = 0; j < 16; j++) {
	    /* printk("%02x ", *ptr++); */
	    printk("%02x ", readb(adress));
	    adress += offset;
	}
	printk("\n");
    }
}
#endif

#if CAN4LINUX_PCI
# define REG_OFFSET 4
#else
# define REG_OFFSET 1
#endif
/**
*   Dump the CAN controllers register contents,
*   identified by the device minr number to stdout
*
*   Base[minor] should contain the virtual adress
*/
void Can_dump(int minor)
{
int i, j;
int index = 0;

	for(i = 0; i < 2; i++) {
	    for(j = 0; j < 16; j++) {
		if (j == 3 && i == 0)
		    /* don't print canirq, since this could reset interrupts */
		    printk("## ");
		else {
		    printk("%02x ",
#ifdef	CAN_PORT_IO
		    inb((int) (Base[minor] + index)) );
#elif	CAN_INDEXED_PORT_IO
		    ({outb (index, Base[minor]); inb (Base[minor] + 1);}) );
#else
		    readb((u32) (can_base[minor] + index)) );
#endif
		}
		index += REG_OFFSET;
	    }
	    printk("\n");
	}
}
#endif

#ifdef CAN4LINUX_PCI
/* reset both can controllers on the EMS-Wünsche CPC-PCI Board */
/* writing to the control range at BAR1 of the PCI board */
void reset_CPC_PCI(unsigned long address)
{
unsigned long ptr = (unsigned long)ioremap(address, 32);
    DBGin("reset_CPC_PCI");
    writeb(0x01, ptr);
    /* writeb(0x00, ptr); */
}

/* check memory region if there is a CAN controller
*  assume the controller was resetted before testing 
*
*  The check for an avaliable controller is difficult !
*  After an Hardware Reset (or power on) the Conroller 
*  is in the so-called 'BasicCAN' mode.
*     we can check for: 
*         adress  name      value
*	    0x00  mode       0x21
*           0x02  status     0xc0
*           0x03  interrupt  0xe0
* Once loaded thr driver switches into 'PelCAN' mode and things are getting
* difficult, because we now have only a 'soft reset' with not so  unique
* values. The have to be masked before comparing.
*         adress  name       mask   value
*	    0x00  mode               
*           0x01  command    0xff    0x00
*           0x02  status     0x37    0x34
*           0x03  interrupt  0xfb    0x00
*
*/
int controller_available(unsigned long address, int offset)
{
unsigned long ptr = (unsigned long)ioremap(address, 32 * offset);

    DBGin("controller_available");
    /* printk("controller_available %ld\n", address); */


    /* printk("0x%0x, ", readb(ptr + (2 * offset)) ); */
    /* printk("0x%0x\n", readb(ptr + (3 * offset)) ); */

    if ( 0x21 == readb(ptr))  {
	/* compare rest values of status and interrupt register */
	if(   0x0c == readb(ptr + (2 * offset))
	   && 0xe0 == readb(ptr + (3 * offset)) ) {
	    return 1;
	} else {
	    return 0;
	}
    } else {
	/* may be called after a 'soft reset' in 'PeliCAN' mode */
	/*   value     address                     mask    */
	if(   0x00 ==  readb(ptr + (1 * offset))
	   && 0x34 == (readb(ptr + (2 * offset))    & 0x37)
	   && 0x00 == (readb(ptr + (3 * offset))    & 0xfb)
	  ) {
	    return 1;
	} else {
	    return 0;
	}

    }
}
#endif


#ifdef CAN4LINUX_PCI
#ifndef CONFIG_PCI
#error "trying to compile a PCI driver for a kernel without CONFIG_PCI"
#endif

#define PCI_BASE_ADDRESS0(dev) (dev->resource[0].start)
#define PCI_BASE_ADDRESS1(dev) (dev->resource[1].start)

/* used for storing the global pci register address */
u32 Can_pitapci_control[MAX_CHANNELS];

int pcimod_scan(void)
{
struct	pci_dev *pdev = NULL;
int	candev = 0;				/* number of found devices */
unsigned long ptr;				/* ptr to PITA control */

    if (pci_present ()) {
	    while ((pdev = pci_find_device (PCIvendorid, PCIdeviceid, pdev))) {
	    printk("  found CPC-PCI: %s\n", pdev->name);
	    if (pci_enable_device(pdev)) {
		continue;
	    }
	    /* printk("        using IRQ %d\n", pdev->irq); */

	    ptr = (unsigned long)ioremap(PCI_BASE_ADDRESS0(pdev), 256);
	    /* enable memory access */
	    /* printk("write to pita\n"); */
	    writel(0x04000000, ptr + 0x1c);
	    Can_pitapci_control[candev] = ptr;

	    /* printk("        pita ptr %lx\n", ptr); */
	    /* printk("---------------\n"); */
	    /* dump_CAN(PCI_BASE_ADDRESS1(pdev)+0x400, 4); */
	    /* printk("---------------\n"); */
	    /* dump_CAN(PCI_BASE_ADDRESS1(pdev)+0x600, 4); */

	    /* PCI_BASE_ADDRESS1:
	     * at address 0 are some EMS control registers
	     * at address 0x400 the first controller area 
	     * at address 0x600 the second controller area 
	     * registers are read as 32bit
	     *
	     * at adress 0 we can verify the card
	     * 0x55 0xaa 0x01 0xcb
	     */
	    {
		unsigned long sigptr; /* ptr to EMS signature  */
		unsigned long signature = 0;
	        sigptr = (unsigned long)ioremap(PCI_BASE_ADDRESS1(pdev), 256);
	        signature =
	        	  (readb(sigptr)      << 24)
	        	+ (readb(sigptr +  4) << 16)
	        	+ (readb(sigptr +  8) <<  8)
	        	+  readb(sigptr + 12);
	    	/* printk("        signature  %lx\n", signature); */
	    	if( 0x55aa01cb != signature) {
	    	    printk(" wrong signature -- no EMS CPC-PCI board\n");
		    return -ENODEV;
	    	}
	    }
	    /* we are now sure to have the right board,
	       reset the CAN controller(s) */
	    reset_CPC_PCI(PCI_BASE_ADDRESS1(pdev) + 0x400);
	    reset_CPC_PCI(PCI_BASE_ADDRESS1(pdev) + 0x600);

	    /* enable interrupts Int_0 */
	    /* write to PITAs ICR register */
	    writel(0x00020000, Can_pitapci_control[candev] + 0x0);

	    /* look for a CAN controller at 0x400 */
	    if(controller_available(PCI_BASE_ADDRESS1(pdev) + 0x400, 4)) {
		printk(" CAN: %d. at pos 1\n", candev + 1);
		if(candev > MAX_CHANNELS) {
		    printk("CAN: only %d devices supported\n", MAX_CHANNELS);
		    break; /* the devices scan loop */
		}
		Base[candev]
		= (unsigned long)ioremap(PCI_BASE_ADDRESS1(pdev) + 0x400, 32*4);
		IOModel[candev] = 'm';
		IRQ[candev] = pdev->irq;
		candev++;
	    } else {
		/* printk(" CAN: NO at pos 1\n"); */
		;
	    }
	    /* look for a CAN controller at 0x400 */
	    if(controller_available(PCI_BASE_ADDRESS1(pdev) + 0x600, 4)) {
		printk(" CAN: %d. at pos 2\n", candev + 1);
		if(candev > MAX_CHANNELS) {
		    printk("CAN: only %d devices supported\n", MAX_CHANNELS);
		    break; /* the devices scan loop */
		}
		/* share the board control register with prev ch */
		Can_pitapci_control[candev] = 
		    Can_pitapci_control[candev - 1];
		Base[candev]
		= (unsigned long)ioremap(PCI_BASE_ADDRESS1(pdev) + 0x600, 32*4);
		IOModel[candev] = 'm';
		IRQ[candev] = pdev->irq;
		candev++;
	    } else {
		/* printk(" CAN: NO at pos 2\n"); */
		;
	    }
	}
    } else {
        printk("CAN: No PCI bios present\n");
        return -ENODEV;
    }

    return 0;
}
#endif
