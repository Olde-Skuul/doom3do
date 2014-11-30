#include "IntMath.h"

/**********************************

	Divide 2 unsigned integers and return

**********************************/

Word IMUDivide(Word a,Word b,WordDivRec *Answer)
{
	if (!b) {
		return IMBadInptParam;
	}
	Answer->quotient = a/b;
	Answer->remainder = a%b;
	return 0;
}
