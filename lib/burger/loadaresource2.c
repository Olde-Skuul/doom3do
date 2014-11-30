#include "burger.h"

/********************************

    Load in a resource

********************************/

void *LoadAResource2(Word RezNum,Word Type)
{
    void **BufferPtr;

	BufferPtr = LoadAResourceHandle2(RezNum,Type);	/* Get the handle */
	if (BufferPtr) {
		return LockAHandle(BufferPtr);   /* Lock and return */
	}
	return 0;
}
