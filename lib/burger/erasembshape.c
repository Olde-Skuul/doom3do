#include "Burger.h"
#include <graphics.h>

/**********************************

	Erase a masked shape from the screen

**********************************/

void EraseMBShape(Word x,Word y,void *ShapePtr,void *BackPtr)
{
	LongWord TempPIXC;

	TempPIXC = ((CCB*)ShapePtr)->ccb_PIXC;
	((CCB*)ShapePtr)->ccb_PIXC = TempPIXC | 0x80008000;	/* Cel engine #1 and #2 */
	SetReadAddress(VideoItem,(char *)BackPtr,320);		/* Set the read address to offscreen buffer */
	DrawMShape(x,y,ShapePtr);				/* Draw the masked shape */
	((CCB*)ShapePtr)->ccb_PIXC = TempPIXC;	/* Restore the bits */
	ResetReadAddress(VideoItem);			/* Restore the read address */
}
