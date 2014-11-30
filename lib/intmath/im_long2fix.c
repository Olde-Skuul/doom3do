#include "IntMath.h"

/**************************

	Convert a signed long into a 32 bit 16.16 fixed point
	number.
	If the converted number is out of bounds, then it will be set
	to the maximum allowed (0x7FFFFFFF if positive or 0x80000000 if
	negative).

**************************/

Fixed IMLong2Fix(long Input)
{
	if (Input<0x8000) {		/* Too large? */
		if (Input>=-0x8000) {	/* Not too small? */
			return Input>>16;	/* Number is OK! Convert and return */
		}
		return 0x80000000;	/* Return smallest number */
	}
	return 0x7FFFFFFF;		/* Return largest number */
}
