#include "burger.h"

/********************************

    Dispose of a memory handle into the free handle pool
    Will do a nonfatal error if the handle is bogus

********************************/

extern MyHandle *FreeHands;		/* Pointer to free handle list */
extern MyHandle *PrevAllocHand;	/* Previously allocated handle */
extern MyHandle *PrevAllocFixedHand;
extern MyHandle UsedHands;

void DeallocAHandle(void **MemHandle)
{
    MyHandle *Next;
    MyHandle *Prev;

    if (MemHandle) {       /* Valid pointer? */
		Next = ((MyHandle *)MemHandle)->NextHandle;	/* Get the links */
		Prev = ((MyHandle *)MemHandle)->PrevHandle;

		Prev->NextHandle = Next;	/* Set the previous link */
		Next->PrevHandle = Prev;	/* Link the next handle */

		/* Insert to the free handle list */
		((MyHandle *)MemHandle)->NextHandle = FreeHands;
		FreeHands = (MyHandle*)MemHandle;	/* Make this the head */

		/* Failsafe for memory allocation */

		PrevAllocHand = UsedHands.NextHandle;
		PrevAllocFixedHand = UsedHands.PrevHandle;
        return;         /* Exit! */
    }
    NonFatal("Invalid Handle");     /* Bad handle! */
}
