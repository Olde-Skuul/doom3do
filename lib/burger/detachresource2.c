#include "burger.h"

/********************************

    Destroy the data associated with a resource

********************************/

void DetachAResource2(Word RezNum,Word Type)
{
    MyRezEntry2 *Entry;
    Entry = ScanRezMap(RezNum,Type);
    if (Entry) {     /* Scan for the resource */
		Entry->MemPtr = 0;       /* Mark as GONE */
    }
}
