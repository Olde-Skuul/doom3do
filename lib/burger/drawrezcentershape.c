#include "Burger.h"

/**********************************

	Draw a shape using a resource number
	and center it on the screen.
	
**********************************/

void DrawRezCenterShape(Word RezNum)
{
	void *ShapePtr;
	Word x,y;
	
	ShapePtr = LoadAResource(RezNum);
	x = (ScreenWidth-GetShapeWidth(ShapePtr))/2;
	y = (ScreenHeight-GetShapeHeight(ShapePtr))/2;
	DrawShape(x,y,ShapePtr);
	ReleaseAResource(RezNum);
}
