#include "burger.h"

/********************************

    Set the lock flag to a given handle and return the data pointer

********************************/

void *LockAHandle(void **TheHandle)
{
    ((MyHandle *)TheHandle)->Flags |= HANDLELOCK;
    return *TheHandle;
}
