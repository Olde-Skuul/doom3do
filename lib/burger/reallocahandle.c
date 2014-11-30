#include "Burger.h"

/********************************

    If the handle was purged, reallocate memory to it.
    Note, the returned handle will REPLACE the handle you passed me.
    This code effectively disposes of the previous handle and allocates
    a new one of the old one's size. If the data is still intact then
    nothing happens

********************************/

void **ReallocAHandle(void **MemHandle)
{
    LongWord Size;      /* Size to allocate */

    if (*MemHandle) {   /* Handle already valid? */
        return MemHandle;   /* Leave now! */
    }
    Size = ((MyHandle *)MemHandle)->Length;    /* How much memory to allocate */
    DeallocAHandle(MemHandle);      /* Dispose of the old handle */
    return AllocAHandle(Size);   /* Create a new one with the old size */
}

