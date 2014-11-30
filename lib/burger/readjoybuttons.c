#include "Burger.h"
#include <event.h>

/**********************************

	Read the bits from the joystick

**********************************/

Word LastJoyButtons[4];		/* Save the previous joypad bits */

Word ReadJoyButtons(Word PadNum)
{
	ControlPadEventData ControlRec;

	GetControlPad(PadNum+1,FALSE,&ControlRec);		/* Read joypad */
	if (PadNum<4) {
		LastJoyButtons[PadNum] = (Word)ControlRec.cped_ButtonBits;
	}
	return (Word)ControlRec.cped_ButtonBits;		/* Return the data */
}
