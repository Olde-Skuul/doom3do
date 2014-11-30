#include "IntMath.h"

/**********************************

	Return the low 16 bits of a 32 bit unsigned long

**********************************/

Word IMLoWord(LongWord Input)
{
	return Input&0xFFFF;
}
