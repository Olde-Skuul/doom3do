#include "IntMath.h"

/**********************************

	Return the high 16 bits of a 32 bit unsigned long

**********************************/

Word IMHiWord(LongWord Input)
{
	return Input>>16;
}
