#include "Burger.h"

extern MyHandle UsedHands;	/* Pointer to used handle list */

/**********************************

	Search the handle tree for a pointer, note that the pointer does
	NOT have to be the MemPtr, just in the domain of the handle
	Return 0 if the handle is not here.

**********************************/

void **FindAHandle(void *MemPtr)
{
	Byte *ThePtr;		/* Temp pointer */
	MyHandle *List;		/* Current handle */

    List = UsedHands.NextHandle;	/* Get the first handle */
	if (List!=&UsedHands) {		/* Failsafe... */
		do {
			ThePtr = (Byte *)List->MemPtr;	/* Get the handle's memory pointer */
			if (ThePtr>((Byte *)MemPtr)) {	/* Is it too far? */
				break;			/* Abort now... */
			}
			ThePtr += List->Length;			/* Get the final byte address */
			if (ThePtr>((Byte *)MemPtr)) {	/* In range? */
				return (void **)List;		/* This is the handle! */
			}
			List = List->NextHandle;
		} while (List!=&UsedHands);					/* List still valid? */
	}
	NonFatal("Can't find handle");			/* Didn't find it... */
	return 0;							/* Return null */
}
