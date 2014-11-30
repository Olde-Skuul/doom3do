#include "burger.h"

/********************************

    Set the purge flag to a given handle

********************************/

void SetHandlePurgeFlag(void **TheHandle,Word Flag)
{
	((MyHandle*)TheHandle)->Flags =
		(((MyHandle*)TheHandle)->Flags & ~HANDLEPURGEBITS) | Flag;
}
