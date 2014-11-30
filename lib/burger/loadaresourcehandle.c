#include "burger.h"

/********************************

    Initialize the memory manager so I can have purgable memory
    chunks.

********************************/

void **LoadAResourceHandle(Word RezNum)
{
    return LoadAResourceHandle2(RezNum,1);
}

