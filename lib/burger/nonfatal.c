#include <burger.h>        

extern Byte BombFlag;

/********************************

    A Non-FATAL error has occured, save message, but if the 
    BombFlag is set, then die anyways.

********************************/

void NonFatal(char *Message)
{
    if (Message) {      /* No message, no error! */
        ErrorMsg = Message; /* Save the message */
        if (BombFlag) {         /* Bomb on ANY Error? */
            Fatal(Message);     /* Force a fatal error */
        }
    }
}

    
