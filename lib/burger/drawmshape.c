#include "Burger.h"
#include <Graphics.h>

/**********************************

	Draw a masked shape on the screen

**********************************/

void DrawMShape(Word x,Word y,void *ShapePtr)
{
	((CCB*)ShapePtr)->ccb_XPos = x<<16;		/* Set the X coord */
	((CCB*)ShapePtr)->ccb_YPos = y<<16;		/* Set the Y coord */
	((CCB*)ShapePtr)->ccb_Flags &= ~CCB_BGND;	/* Enable masking */
	DrawCels(VideoItem,(CCB*)ShapePtr);		/* Draw the shape */
}
