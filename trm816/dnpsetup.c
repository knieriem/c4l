#include <stdlib.h>
#include "cmdline.h"
#include <string.h>
#include <unistd.h>
#include <sys/io.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include "trm.h"

extern void dnp_map_can(unsigned base, unsigned irq, int type);
extern void adnp1520_map_can(unsigned base,int type);

int main (int argc, char *argv[]) {

  unsigned port;

  /* parse clig values */
  Cmdline *cmd = parseCmdline(argc, argv);
  
  
  if (strchr(cmd->base,'x')==NULL) {  
    if (sscanf(cmd->base,"%d",&port)<1) {
      fprintf(stderr,"wrong format of portaddress: %s\n",cmd->base);
      return(1);
    }
  }else {
    if (sscanf(cmd->base,"%x",&port)<1) {
      fprintf(stderr,"wrong format of portaddress: %s\n",cmd->base);
      return(1);
    }
  }

  if ((strcmp(cmd->dnp,"1486")!=0) && (strcmp(cmd->dnp,"1520")!=0)) {
    fprintf(stderr,"Valid (X)DNP-Types are 1486 and 1520!\n");
    return(1);
  }

  if ((strcmp(cmd->hw,"trm816")!=0) && (strcmp(cmd->hw,"custom")!=0)) {
    fprintf(stderr,"Valid Hardware-Types are trm816 and custom!\n");
    return(1);
  }
  
  if (strcmp(cmd->dnp,"1486")==0) {
    if (getuid()!=0) {
      fprintf(stderr,"You need to be root to use this command!\n");
      return(1);
    } 
    iopl(3);
    if (strcmp(cmd->hw,"trm816")==0) {
      printf("Setting Hardware (X)DNP1486 TRM816: Base:0x%x IRQ:%d\n",
	     port,cmd->irq);
      dnp_map_can(port,cmd->irq,ADNPTRM);
    } else {
      printf("Setting Hardware (X)DNP1486 CUSTOM: Base:0x%x IRQ:%d\n",
	     port,cmd->irq);
      dnp_map_can(port,cmd->irq,ADNPCUST);
    }
  }

  if (strcmp(cmd->dnp,"1520")==0) {
    if (strcmp(cmd->hw,"trm816")==0) {
      printf("Setting Hardware ADNP1520 TRM816: Base:0x%x\n",port);
      adnp1520_map_can(port,XADNPTRM);
    } else {
      printf("Setting Hardware ADNP1520 CUSTOM: Base:0x%x\n",port);
      adnp1520_map_can(port,XADNPCUST);
    }
  }


  return(0);
}
















