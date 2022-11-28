#include <wiringPi.h>
#include <wiringSerial.h>
#include <cstdio>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


int main() {
    int serial_port;
    char dat[2];
    if ((serial_port = serialOpen ("/dev/ttyAMA0", 115200)) < 0){	/* open serial port */
        printf("Unable to open serial device\n");
        return 1 ;
    } else {
        printf("Serial opened\n");
    }

    if (wiringPiSetup () == -1)					/* initializes wiringPi setup */
    {
        fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
        return 1 ;
    }

    while(1){
        if(serialDataAvail (serial_port) ) { 

            read(serial_port, dat, 2);		/* receive character serially*/	
            printf ("data: %d %d\n", dat[0], dat[1]) ;
            // fflush (stdout) ;
            // serialPutchar(serial_port, '1');		/* transmit character serially on port */
            // usleep(500000);
        }

        serialPutchar(serial_port, '1');		/* transmit character serially on port */
        usleep(500000);
	}
}