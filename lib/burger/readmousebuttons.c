#include "Burger.h"
#include <event.h>

/**********************************

	Read the bits from the mouse

**********************************/

Word LastMouseButton;

Word ReadMouseButtons(void)
{
	MouseEventData ControlRec;

	GetMouse(1,FALSE,&ControlRec);		/* Read mouse */
	LastMouseButton = (Word) ControlRec.med_ButtonBits;	/* Return the data */
	return (Word) ControlRec.med_ButtonBits;	/* Return the data */
}
