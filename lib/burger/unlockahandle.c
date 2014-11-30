#include "burger.h"

/********************************

    Clear the lock flag to a given handle

********************************/

void UnlockAHandle(void **TheHandle)
{
    ((MyHandle*)TheHandle)->Flags&=(~HANDLELOCK);   /* Clear the lock flag */
}
