#include "burger.h"

/********************************

    Initialize the memory manager so I can have purgable memory
    chunks.

********************************/

void DetachAResource(Word RezNum)
{
    DetachAResource2(RezNum,1);
}

