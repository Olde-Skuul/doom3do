#include "Burger.h"
#include <string.h>

/********************************

	Purges all handles that are purgable and are
	greater or equal to the level requested

********************************/

extern MyHandle UsedHands;		/* Used list */
extern MyHandle PurgeHands;		/* Purged Master handle */
extern MyHandle *PrevAllocHand;
extern MyHandle *PrevAllocFixedHand;

void PurgeHandles(Word PurgeLevel)
{
    MyHandle *Scan;
	Word Level;
	Word CalledCallBack;

	if (!PurgeLevel) {		/* Purgelevel #0 means you can't purge it */
		PurgeLevel = 1;		/* Make sure I purge valid handles... */
	}
    Scan = UsedHands.NextHandle;	/* Index to the active handle list */
	if (Scan==&UsedHands) {
		return;
	}
	CalledCallBack = TRUE;		/* Assume bogus */
	if (MemPurgeCallBack) {
		CalledCallBack = FALSE;	/* Allow call */
	}
    do {			/* Valid handle? */
        if (!(Scan->Flags & HANDLELOCK)) {      /* Skip all locked handles */
            Level = Scan->Flags & HANDLEPURGEBITS;		/* Purgeable? */
			if (Level>=PurgeLevel) {	/* Can I purge it? */
				MyHandle *Prev;			/* Previous link */
				MyHandle *Next;

				if (!CalledCallBack) {
					CalledCallBack = TRUE;
					MemPurgeCallBack(MMStagePurge);
				}
				Next = Scan->NextHandle;	/* Forward link */
				Prev = Scan->PrevHandle;	/* Backward link */
				Next->PrevHandle = Prev;	/* Unlink me from the list */
				Prev->NextHandle = Next;

				/* Link into the purge list */

				Prev = PurgeHands.NextHandle;	/* Get the first link */
                Scan->MemPtr = 0;		/* Zap the pointer (Purge list) */
				Scan->PrevHandle = &PurgeHands;	/* I am the parent */
                Scan->NextHandle = Prev;	/* Link it to the purge list */
				Prev->PrevHandle = Scan;
                PurgeHands.NextHandle = Scan;	/* Make as the new head */

				/* It's gone, now continue with the same last handle */

                Scan = Next;	/* Jump back one to remove from list following */
                continue;		/* Continue the scan */
            }
        }
        Scan = Scan->NextHandle;        /* Next handle in chain */
    } while (Scan!=&UsedHands);
	PrevAllocHand = UsedHands.NextHandle;		/* Reset the alloc handle */
	PrevAllocFixedHand = UsedHands.PrevHandle;
}
