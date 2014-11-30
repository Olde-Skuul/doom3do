#include "Burger.h"
#include <Graphics.h>

/**********************************

	Return the width of a shape in pixels

**********************************/

Word GetShapeWidth(void *ShapePtr)
{
	Word Result;
	Result = ((CCB*)ShapePtr)->ccb_PRE1&0x7FF;	/* Get the HCount bits */
	return Result+1;			/* Return the TRUE result */
}
