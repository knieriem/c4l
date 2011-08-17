/************************************************************************/
/* hardware access functions or macros */
/************************************************************************/
#ifdef  CAN_PORT_IO
/* #error Intel port I/O access */
/* using port I/O with inb()/outb() for Intel architectures like 
   AT-CAN-MINI ISA board */

#ifdef IODEBUG
#  define CANout(bd,adr,v)	\
	(printk("Cout: (%x)=%x\n", (int)&((canregs_t *)Base[bd])->adr, v), \
		outb(v, (int) &((canregs_t *)Base[bd])->adr ))
#else
#  define CANout(bd,adr,v)	\
		(outb(v, (int) &((canregs_t *)Base[bd])->adr ))
#endif
#define CANin(bd,adr)		\
		(inb ((int) &((canregs_t *)Base[bd])->adr  ))
#define CANset(bd,adr,m)	\
	outb((inb((int) &((canregs_t *)Base[bd])->adr)) \
		| m ,(int) &((canregs_t *)Base[bd])->adr )
#define CANreset(bd,adr,m)	\
	outb((inb((int) &((canregs_t *)Base[bd])->adr)) \
		& ~m,(int) &((canregs_t *)Base[bd])->adr )
#define CANtest(bd,adr,m)	\
	(inb((int) &((canregs_t *)Base[bd])->adr  ) & m )

#endif 	/* CAN_PORT_IO */
#if ! defined CAN_PORT_IO
#ifdef CAN_INDEXED_PORT_IO
/* #error Indexed Intel port I/O access */
/* using port I/O with indexed inb()/outb() for Intel architectures like 
   SSV TRM/816 DIL-NET-PC */

#ifdef IODEBUG
#define CANout(bd,adr,v) {\
        printk("CANout bd:%x base:%x reg:%x val:%x\n", \
                bd, (u32) Base[bd], \
		(u32) &regbase->adr,v); \
        outb((u32) &regbase->adr,(u32) Base[bd]);\
        outb(v,((u32) Base[bd])+1);\
  }
#else
#define CANout(bd,adr,v) {\
        outb((u32) &regbase->adr,(u32) Base[bd]);\
        outb(v,((u32) Base[bd])+1);\
}
#endif
#define CANin(bd,adr) \
        Indexed_Inb((u32) Base[bd],(u32) &regbase->adr)

#define CANset(bd,adr,m) {\
        unsigned val; \
        val=Indexed_Inb((u32) Base[bd],(u32) &regbase->adr);\
        outb((u32) &regbase->adr,(u32) Base[bd]);\
        outb(val | m,((u32) Base[bd])+1);\
}
#define CANreset(bd,adr,m) {\
        unsigned val; \
        val=Indexed_Inb((u32) Base[bd],(u32) &regbase->adr);\
        outb((u32) &regbase->adr,(u32) Base[bd]);\
        outb(val & ~m,((u32) Base[bd])+1);\
}
#define CANtest(bd,adr,m) \
        (Indexed_Inb((u32) Base[bd],(u32) &regbase->adr) & m)
#else

/* using memory acces with readb(), writeb() */
/* #error  memory I/O access */
/* #define can_base Base */
#ifdef IODEBUG
#  define CANout(bd,adr,v)	\
	(printk("Cout (%x)=%x\n", (u32)&((canregs_t *)can_base[bd])->adr, v), \
		writeb(v, (u32) &((canregs_t *)can_base[bd])->adr ))
#define CANset(bd,adr,m)     do	{\
	unsigned char v;	\
        v = (readb((u32) &((canregs_t *)can_base[bd])->adr)); \
	printk("CANset %x |= %x\n", (v), (m)); \
	writeb( v | (m) , (u32) &((canregs_t *)can_base[bd])->adr ); \
	} while (0)

#define CANreset(bd,adr,m)	do {\
	unsigned char v; \
        v = (readb((u32) &((canregs_t *)can_base[bd])->adr)); \
	printk("CANreset %x &= ~%x\n", (v), (m)); \
	writeb( v & ~(m) , (u32) &((canregs_t *)can_base[bd])->adr ); \
	} while (0)

#else
#define CANout(bd,adr,v)	\
		(writeb(v, (u32) &((canregs_t *)can_base[bd])->adr ))
#define CANset(bd,adr,m)	\
	writeb((readb((u32) &((canregs_t *)can_base[bd])->adr)) \
		| (m) , (u32) &((canregs_t *)can_base[bd])->adr )
#define CANreset(bd,adr,m)	\
	writeb((readb((u32) &((canregs_t *)can_base[bd])->adr)) \
		& ~(m), (u32) &((canregs_t *)can_base[bd])->adr )
#endif
#define CANin(bd,adr)		\
		(readb ((u32) &((canregs_t *)can_base[bd])->adr  ))
#define CANtest(bd,adr,m)	\
	(readb((u32) &((canregs_t *)can_base[bd])->adr  ) & (m) )

#endif  	/* ! CAN_INDEXED_PORT_IO */
#endif 		/* ! CAN_PORT_IO */
