#include "Burger.h"
#include <Graphics.h>

/**********************************

	Draw an unmasked shape on the screen

**********************************/

void DrawShape(Word x,Word y,void *ShapePtr)
{
	((CCB*)ShapePtr)->ccb_XPos = x<<16;		/* Set the X coord */
	((CCB*)ShapePtr)->ccb_YPos = y<<16;		/* Set the Y coord */
	((CCB*)ShapePtr)->ccb_Flags |= CCB_BGND;	/* Disable masking */
	DrawCels(VideoItem,(CCB*)ShapePtr);		/* Draw the shape */
}
