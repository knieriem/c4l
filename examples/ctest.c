/* simple driver test: just open and close the device
* 
* open is called with starting the application
* close is called after pressing ^C - SIGKILL or after a very long time 
*
* first argument can be the device name -- else it uses can0
*
* if a second arg is given, the programm loops every second with e read()
* call
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <can4linux.h>

#define STDDEV "can0"

unsigned long usleeptime = 1000;	/* 1000 ms */

int main(int argc,char **argv)
{
int fd;
canmsg_t rx;
char device[40];

    if(argc > 1) {
	sprintf(device, "/dev/%s", argv[1]);
    }
    else {
	sprintf(device, "/dev/%s", STDDEV);
    }
    printf("using CAN device %s\n", device);
    
    if(( fd = open(device, O_RDWR )) < 0 ) {
	fprintf(stderr,"Error opening CAN device %s\n", device);
        exit(1);
    }
    if(argc == 3) {
    int count = 100000;
    int i, n;

        usleeptime = atoi(argv[2]);
	/* loop for a long time */
        while(count--) {
            do {
		n = read(fd, &rx, 1);
		if(n < 0) {
		    perror("CAN read error");
		}
		else if(n == 0) {
		    fprintf(stderr, "read returned 0\n");
		} else {
		    fprintf(stderr, "read: 0x%08lx : %d bytes",
		    			   rx.id,   rx.length);
		    if(rx.length > 0) {
			fprintf(stderr, "\t");
			for(i = 0; i < rx.length; i++) {
			    fprintf(stderr, "%02x ", rx.data[i]);
			}
		    }
		    fprintf(stderr, "\n");
		}
	    } while( n == 1);
            usleep(usleeptime * 1000);
        }
    } else {
        /* wait very long */
	sleep(100000);
    }
    close(fd);
    return 0;
}
