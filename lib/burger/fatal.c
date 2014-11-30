#include <burger.h>       
#include <stdio.h>
#include <stdlib.h>

/********************************

    A FATAL error has occured, print message, then die!

********************************/
       
void Fatal(char *FatalMsg)
{
    if (FatalMsg) {         /* Message to print? */
        printf(FatalMsg);   /* Print it */
    }
    exit(10);               /* Exit to OS */
}

    
