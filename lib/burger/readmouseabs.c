#include "Burger.h"
#include <event.h>

/**********************************

	Read the bits from the mouse

**********************************/

void ReadMouseAbs(Word *x,Word *y)
{
	MouseEventData ControlRec;

	GetMouse(1,FALSE,&ControlRec);		/* Read mouse */
	*x = (Word)ControlRec.med_HorizPosition;
	*y = (Word)ControlRec.med_VertPosition;
}
