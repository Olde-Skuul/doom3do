#include "Burger.h"

/**********************************

	Return the pointer of a shape from a shape array

**********************************/

void *GetShapeIndexPtr(void *ShapeArrayPtr,Word Index)
{
	return &((Byte *)ShapeArrayPtr)[((LongWord *)ShapeArrayPtr)[Index]];
}
