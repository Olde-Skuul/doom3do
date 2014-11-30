#include "Burger.h"

/**********************************

	Release a POINTER, I created a handle with 4
	bytes extra data and I place the handle at the beginning

**********************************/

void DeallocAPointer(void *MemPtr)
{
	if (MemPtr) {			/* Null pointer?!? */
		MemPtr=((Byte *)MemPtr)-4;		/* Move back 4 bytes */
		DeallocAHandle(((void ***)MemPtr)[0]);
	}
}
