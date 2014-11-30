#include <burger.h>

/********************************

    Initialize the memory manager so I can have purgable memory
    chunks.

********************************/

void ReleaseAResource(Word RezNum)
{
    ReleaseAResource2(RezNum,1);
}
