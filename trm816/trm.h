/* ELAN SC4xx Index-Reg. */
#define SC4xx_INDEX_REG         0x22
/* ELAN SC4xx Data-Reg. */
#define SC4xx_DATA_REG          0x23

typedef volatile unsigned int   vu32;
volatile void *pSC520_MMCR_BASE;

/* ELAN SC520 Programmable Address Regions */
#define SC520_PAR0      (*(((vu32 *)pSC520_MMCR_BASE) + 0x0088 / 4))
#define SC520_PAR1      (*(((vu32 *)pSC520_MMCR_BASE) + 0x008c / 4))

#define ADNPTRM 1
#define XADNPTRM 2
#define ADNPCUST 3
#define XADNPCUST 4
