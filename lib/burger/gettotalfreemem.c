 #include "burger.h"

/********************************

    Returns the total free space with purging

********************************/

extern MyHandle UsedHands;

LongWord GetTotalFreeMem(void)
{
    LongWord Free;		/* Running total */
    MyHandle *Scan;		/* Pointer to handle */
    Byte *EndPtr;		/* Pointer to end of data chunk */

    Free = 0;			/* Init free size */
    Scan = UsedHands.NextHandle;	/* Follow the entire list */
    if (Scan!=&UsedHands) {			/* List valid? */
	    for (;;) {        /* Not the last? */
    	    EndPtr = (Byte*)Scan->MemPtr;    /* End mem pointer */
    	    if ((Scan->Flags&HANDLELOCK) ||
				!(Scan->Flags&HANDLEPURGEBITS)) {
            	EndPtr += Scan->Length;       /* Don't add the chunk size */
	        }
        	Scan = Scan->NextHandle;          /* Next pointer */
	        if (Scan==&UsedHands) {            /* Valid handle? */
				break;
        	}
	        Free += (Byte*)Scan->MemPtr - EndPtr;   /* Add free memory */
		}
	}
	return Free;
}


