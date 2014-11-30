#include "burger.h"

extern MyHandle *FreeHands;		/* Free handle list */
extern MyHandle UsedHands;		/* Used handle list */
extern MyHandle *PrevAllocHand;	/* Previously allocated handle */
extern MyHandle *PrevAllocFixedHand;	/* Previously allocated fixed handle */

/********************************

	Allocates an unlocked, unpurgable block
	(Convience routine, but used so often, I might as well
	include it here)

********************************/

void **AllocAHandle(LongWord MemSize)
{
	return AllocAHandle2(MemSize,0);
}

/********************************

	Allocates a block of memory
	I allocate from the top down if fixed and bottom up if movable
	This routine handles all the magic for memory purging and
	allocation.

********************************/

void **AllocAHandle2(LongWord MemSize,Word Flag)
{
    MyHandle *Scan;			/* Current handle checking */
    MyHandle *LastScan;		/* Previous handle checked */
    MyHandle *EndScan;		/* Handle to stop scanning */
    MyHandle *NewHandlePtr;	/* New handle to create */
    Word Stage;             /* Search stage */
	LongWord Length;		/* Space between handles */
	Word NewPrevHand;		/* Flag to search for first empty spot */

#if defined(__3DO__)		/* 3DO specific... */
	MemSize = (MemSize+3)&~3;	/* Must be long word aligned! */
#endif

    NewHandlePtr=FreeHands;		/* Get a new handle from the free list */
    if (!NewHandlePtr) {		/* No handles available? */
        NonFatal("No handles free");	/* Uhh, out of handles! */
        return 0;           /* Return error */
    }
    FreeHands=NewHandlePtr->NextHandle;	/* Unlink this free handle */

    NewHandlePtr->Length = MemSize;     /* Save the handle size */
    NewHandlePtr->Flags = Flag;    /* Save the default attributes */
    Stage = 0;              /* Init data memory search stage */

	if (Flag&HANDLEFIXED) {

		/* Scan from the top down for fixed handles */
		/* Increases odds for compaction success */

	    do {
    	    switch (Stage) {    /* Do the scan from the last allocated handle */
        	case 0:
				NewPrevHand = TRUE;
	            LastScan = PrevAllocFixedHand;       /* Allocate from last allocated handle */
    	        EndScan = &UsedHands;			/* Continue to the end */
        	    break;
			case 2:		/* Purge level 3 */
			case 3:		/* Purge level 2 */
			default:		/* Purge level 1 */
	            PurgeHandles(5-Stage);       /* Purge all handles */
			case 1:
				CompactHandles();		/* Pack memory together */
        	    LastScan = UsedHands.PrevHandle;   /* Scan the entire list */
            	EndScan = &UsedHands;        /* End of list */
				NewPrevHand = TRUE;		/* Mark for the first one found */
	        }

			Scan = LastScan->PrevHandle;	/* Get next index */

		/* Find the memory, LastScan has the handle the memory */
		/* will occupy BEFORE, Scan is the prev handle before the new one */

	   	    if (Scan != EndScan) {		/* Already stop? */
			  	do {
					Byte *StartPtr;         /* Possible block pointer */

				    StartPtr = ((Byte*)Scan->MemPtr) + Scan->Length;  /* Get pointer to free memory */
					Length = ((Byte *)LastScan->MemPtr) - StartPtr;
					if (Length) {		/* Valid chunk? */
						if (Length >= MemSize) {       /* Space between handles big enough? */
    	   		        	LastScan->PrevHandle = NewHandlePtr;    /* Link in my NEW handle */
							Scan->NextHandle = NewHandlePtr;		/* Backward link */

							NewHandlePtr->MemPtr = (Byte *)LastScan->MemPtr-MemSize;    /* New memory start */
			    	    	NewHandlePtr->NextHandle = LastScan;    /* Link into the next handle */
							NewHandlePtr->PrevHandle = Scan;	/* Previous handle */
							if (NewPrevHand) {
								PrevAllocFixedHand = NewHandlePtr;
							}
		    	           	return (void **)NewHandlePtr;         /* Good allocation! */
	    	    	    }
						if (NewPrevHand) {
							NewPrevHand = FALSE;
							PrevAllocFixedHand = LastScan;	/* Mark first empty area */
						}
					}
					LastScan = Scan;        /* Mark from this handle */
		       	    Scan=Scan->PrevHandle;        /* Look at next handle */
    		    } while (Scan != EndScan);	/* End of the list? */
			}
	    } while (++Stage<5);            /* Still can't get it! */

	} else {

		/* Scan from the bottom up for movable handles */
		/* Increases odds for compaction success */

	    do {
    	    switch (Stage) {    /* Do the scan from the last allocated handle */
        	case 0:
				NewPrevHand = TRUE;
	            LastScan = PrevAllocHand;       /* Allocate from last allocated handle */
    	        EndScan = &UsedHands;			/* Continue to the end */
				break;
			case 2:		/* Purge level 3 */
			case 3:		/* Purge level 2 */
			default:		/* Purge level 1 */
	            PurgeHandles(5-Stage);       /* Purge all handles */
			case 1:
				CompactHandles();		/* Pack memory together */
        	    LastScan = UsedHands.NextHandle;   /* Scan the entire list */
            	EndScan = &UsedHands;        /* End of list */
				NewPrevHand = TRUE;
	        }

			Scan = LastScan->NextHandle;	/* Get next index */

		/* Find the memory, LastScan has the handle the memory */
		/* will occupy AFTER, Scan is the next handle after the new one */

	   	    if (Scan != EndScan) {		/* Already stop? */
			  	do {
					Byte *StartPtr;         /* Possible block pointer */

				    StartPtr = ((Byte*)LastScan->MemPtr) + LastScan->Length;  /* Get pointer to free memory */
					Length = ((Byte *)Scan->MemPtr) - StartPtr;
					if (Length) {
   			        	if (Length >= MemSize) {       /* Space between handles big enough? */
    	   		        	LastScan->NextHandle = NewHandlePtr;    /* Link in my NEW handle */
							Scan->PrevHandle = NewHandlePtr;		/* Backward link */

    	        	    	NewHandlePtr->MemPtr = StartPtr;    /* New memory start */
			        		NewHandlePtr->NextHandle = Scan;    /* Link into the next handle */
							NewHandlePtr->PrevHandle = LastScan;	/* Previous handle */
							if (NewPrevHand) {
    		       	    		PrevAllocHand = NewHandlePtr;         /* Last handle touched! */
							}
		    	           	return (void **)NewHandlePtr;         /* Good allocation! */
	    	    	    }
						if (NewPrevHand) {
							NewPrevHand = FALSE;
							PrevAllocHand = LastScan;
						}
					}
					LastScan = Scan;        /* Mark from this handle */
		       	    Scan=Scan->NextHandle;        /* Look at next handle */
    		    } while (Scan != EndScan);	/* End of the list? */
			}
	    } while (++Stage<5);            /* Still can't get it! */
	}

/* I failed in my quest for memory, exit as a miserable loser */

    NewHandlePtr->NextHandle = FreeHands;   /* Put the handle back! */
    FreeHands = NewHandlePtr;       /* Put in the free list */
	NonFatal("Out of memory");      /* Too bad! */
    return 0;
}
