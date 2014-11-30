#include "IntMath.h"

/**********************************

	Divide 2 signed integers and return

**********************************/

Word IMSDivide(int a,int b,IntDivRec *Answer)
{
	if (!b) {
		return IMBadInptParam;
	}
	Answer->quotient = a/b;
	Answer->remainder = a%b;
	return 0;
}
