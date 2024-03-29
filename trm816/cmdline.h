#ifndef __cmdline__
#define __cmdline__
/*****
  command line parser interface -- generated by clig 
  (http://wsd.iitb.fhg.de/~kir/clighome/)

  The command line parser `clig':
  (C) 1995---2001 Harald Kirsch (kirschh@lionbioscience.com)
*****/

typedef struct s_Cmdline {
  /***** -p: portadress */
  char baseP;
  char* base;
  int baseC;
  /***** -irq: Interrupt */
  char irqP;
  int irq;
  int irqC;
  /***** -d: type of DIL-NET-PC */
  char dnpP;
  char* dnp;
  int dnpC;
  /***** -hw: type of Hardware */
  char hwP;
  char* hw;
  int hwC;
  /***** uninterpreted command line parameters */
  int argc;
  /*@null*/char **argv;
} Cmdline;


extern char *Program;
extern void usage(void);
extern /*@shared*/Cmdline *parseCmdline(int argc, char **argv);

#endif

