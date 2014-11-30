#include "Burger.h"

/**********************************

	Wait for a system event to occur...

**********************************/

Word WaitTicksEvent(Word Delay)
{
	LongWord Tick,NewTick;
	Word Temp;
	Word MouseBits;
	Word JoyBits;

	Tick = ReadTick();		/* Set the current time mark */
	MouseBits = LastMouseButton;
	JoyBits = LastJoyButtons[0];

	do {
		Temp = ReadJoyButtons(0);		/* Pressed a joypad button? */
		if ((Temp^JoyBits)&Temp) {
			return 1;				/* Joypad event */
		}
		JoyBits = Temp;			/* Save it */

		Temp = ReadMouseButtons();		/* Read the mouse */
		if ((Temp^MouseBits)&Temp) {		/* Pressed a mouse button? */
			return 1;
		}
		MouseBits = Temp;		/* Save it */

		Temp = GetAKey();		/* Try the keyboard */
		if (Temp) {
			return Temp;		/* Return the key event */
		}
		NewTick = ReadTick();	/* Get the time value */
	} while ((NewTick-Tick)<(LongWord)Delay);
	LastTick = NewTick;
    return 0;
}
