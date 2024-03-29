#include <sys/io.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "trm.h"

/* Write given Elan SC4xx Configuration Register */
void write_port(unsigned char index,unsigned char data) {
   outb (index, SC4xx_INDEX_REG);
   outb (data, SC4xx_DATA_REG);
}

/* Read given Elan SC4xx Configuration Register */
char read_port(unsigned char index) {
   outb (index, SC4xx_INDEX_REG);
   return (inb (SC4xx_DATA_REG));
}

/* Function..: mapping the CAN-Controller to given I/O-Base and IRQ */
void dnp_map_can(unsigned base, unsigned irq, int type) {
  //cli();
  /* if IRQ6 is'nt mapped, map now ... */
  if ((read_port (0xD7) & 0x0F) == 0) {
    write_port (0xD7, (read_port(0xD7) & 0xF0) | irq);
  } 

  if (type==ADNPTRM) {
    /* Init (A)DNP for CAN SJA1000  */
    /* GPIO_CS0..7 disable (can't read) */
    write_port(0xA6, 0xFF);

    /* GPIO_CS1 to output
     *
     * Bit 3: CS1_PRI = Cause activity or wake up
     * Bit 2: CS1_DIR = Output
     */
    write_port(0xA0, read_port (0xA0) | 0x0C);

    /* GP_CSB address decode register: Bits 0-7 of Base Address (base) */
    write_port(0xB6, base & 0x00FF);

    /* GP_CSB address decode register & mask:
     *
     * Bits 5-2: CSB_SA{3-0}_MASK = 1110, i.e. address range [base..base+1]
     * Bits 0-1: bits 8-9 of Base Address
     */
    write_port(0xB7, (base >> 8) | 0x38);

    /* CSA/B command qualification register
     *
     * Bit    6: 8-bit data bus size and timings
     * Bits 5-4: GP_CSB qualified with IOR & IOW
     */
    write_port(0xB8, (read_port(0xB8) & 0x0F) | 0x30);

    /* Bits 7-4: map GP_CSB to GPIO_CS1 */
    write_port(0xB2, (read_port(0xB2) & 0x0F) | 0x10);

    /* GPIO_CS0..7 enable */
    write_port(0xA6,0x00);
  }

  if (type==ADNPCUST) {
    fprintf(stderr,"mapping for ADNP1486 on Custom board not yet implemented\n");
  }
  //sti();
}

void *pMapmemory(off_t phy_addr, size_t phy_lenght) {
#define MAP_PAGESIZE 4096UL

  int iFd;
  void *pMem;

  if ((phy_addr % MAP_PAGESIZE) != 0) {
    fprintf(stderr,"physical address not aligned on PAGESIZE boundary!\n");
    return(NULL);
  }

  if ((phy_lenght % MAP_PAGESIZE) != 0) {
    fprintf(stderr,"physical lenght not aligned on PAGESIZE boundary!\n");
    return(NULL);
  }

  // open mem device for read/write
  iFd = open("/dev/mem", O_RDWR | O_SYNC);
  if (iFd < 0) {
    fprintf(stderr,"open of /dev/mem fail !\n");
    return(NULL);
  }
  
  // get pointer to DNP1110 memory
  pMem = mmap(NULL,
	      phy_lenght,
	      (PROT_READ | PROT_WRITE),
	      MAP_SHARED,
	      iFd,
	      phy_addr);

  if ((pMem == MAP_FAILED) || (pMem == NULL)) {
    fprintf(stderr,"mmap of /dev/mem fail !\n");
    return(NULL);
  }

  // close mem device
  if (close(iFd) != 0)
    fprintf(stderr,"close of /dev/mem fail !\n");
  
  return(pMem);
}

void adnp1520_map_can(unsigned base,int type) {
  pSC520_MMCR_BASE = pMapmemory(0xFFFEF000, 0x00001000);

  if (!pSC520_MMCR_BASE) {
    fprintf(stderr,"Error running mmap\n");
    exit(1);
  }

  if (type==XADNPTRM) {
    SC520_PAR0 = (0x2c010000 | base);
  }

  if (type==XADNPCUST) {
    // map PAR0, CS2 (GPCS3) to base
    SC520_PAR0=(0x28000000 | base);
    // map PAR1, CS1 (GPCS2) to base+1
    SC520_PAR1=(0x2c000000 | (base+1));
  }

}

