#include "Burger.h"
#include <string.h>

/********************************

    Packs all memory together

********************************/

extern MyHandle UsedHands;		/* Used list */
extern MyHandle PurgeHands;		/* Purged Master handle */

void CompactHandles(void)
{
    MyHandle *Scan;
	Word CalledCallBack;

    Scan = UsedHands.NextHandle;	/* Index to the active handle list */
	if (Scan==&UsedHands) {			/* Failsafe */
		return;
	}
	Scan=Scan->NextHandle;			/* I can't move the first handle ever! */
	if (Scan==&UsedHands) {
		return;
	}
	CalledCallBack = TRUE;		/* Assume bogus */
	if (MemPurgeCallBack) {		/* Valid pointer? */
		CalledCallBack = FALSE;
	}
	do {	/* Skip all locked or fixed handles */
		if (!(Scan->Flags & (HANDLELOCK|HANDLEFIXED))) {
			LongWord Size;
			Byte *StartMem;
			MyHandle *Last;

			Last = Scan->PrevHandle;		/* Get the previous handle */
			StartMem = (Byte *)Last->MemPtr + Last->Length;
			Size = (Byte *)Scan->MemPtr - StartMem;	/* Any space here? */
			if (Size) {		/* If there is free space, then pack them */
				if (!CalledCallBack) {		/* Hadn't called it yet? */
					CalledCallBack = TRUE;
					MemPurgeCallBack(MMStageCompact);	/* Tell the app */
				}
				memmove(StartMem,Scan->MemPtr,Scan->Length);	/* Move */
				Scan->MemPtr = StartMem;		/* New start address */
			}
        }
        Scan = Scan->NextHandle;        /* Next handle in chain */
    } while (Scan!=&UsedHands);
}
