#include "burger.h"

/********************************

    Initialize the memory manager so I can have purgable memory
    chunks.

********************************/

void *LoadAResource(Word RezNum)
{
    return LoadAResource2(RezNum,1);
}

