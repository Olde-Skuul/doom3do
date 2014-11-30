#include <burger.h>

/********************************

    Destroy the data associated with a resource

********************************/

void KillAResource2(Word RezNum,Word Type)
{
    MyRezEntry2 *Entry;
    Entry = ScanRezMap(RezNum,Type);	/* Scan for the resource */
    if (Entry) {
        if (Entry->MemPtr) {     /* Is there a handle? */
            DeallocAHandle(Entry->MemPtr);
            Entry->MemPtr = 0;       /* Mark as GONE */
        }
    }
}
