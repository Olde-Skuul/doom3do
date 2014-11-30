#include "Burger.h"
#include <Graphics.h>

/**********************************

	Return the height of a shape in pixels

**********************************/

Word GetShapeHeight(void *ShapePtr)
{
	Word Result;
	Result = ((CCB*)ShapePtr)->ccb_PRE0>>6;	/* Get the VCount bits */
	Result &= 0x3FF;			/* Mask off unused bits */
	return Result+1;			/* Return the TRUE result */
}
