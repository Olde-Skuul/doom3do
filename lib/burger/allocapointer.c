#include "Burger.h"

/**********************************

	Allocate a POINTER, I create a handle with 4
	bytes extra data and I place the handle at the beginning

**********************************/

void *AllocAPointer(LongWord Size)
{
	LongWord *ThePointer;
	void **TheHandle;
	TheHandle = AllocAHandle2(Size+4,HANDLEFIXED);		/* Get the memory */
	if (TheHandle) {			/* Valid? */
		ThePointer = (LongWord *)*TheHandle;	/* Deref the memory! */
		ThePointer[0] = (LongWord)TheHandle;	/* Save the handle in memory */
		return (ThePointer+1);		/* Return the memory pointer */
	}
	return 0;			/* Can't do it!! */
}
