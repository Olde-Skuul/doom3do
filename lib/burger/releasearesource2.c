#include "burger.h"

/********************************

    Release a resource by marking it purgeable
    but don't destroy it.

********************************/

void ReleaseAResource2(Word RezNum,Word Type)
{
    MyRezEntry2 *Entry;
    Entry = ScanRezMap(RezNum,Type);	/* Scan for the resource */
    if (Entry) {
        if (Entry->MemPtr) {     /* Is there a handle? */
            UnlockAHandle(Entry->MemPtr);        /* Unlock it */
            SetHandlePurgeFlag(Entry->MemPtr,TRUE);  /* Mark as purgable */
        }
    }
}
