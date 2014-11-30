#include "Burger.h"

/**********************************

	Draw a string using my font manager

**********************************/

void DrawAString(void *TextPtr)
{
	Word Temp;
	for (;;) {
		Temp = ((Byte *)TextPtr)[0];	/* Get a char */
		if (!Temp) {					/* End of the string? */
			return;						/* Exit */
		}
		DrawAChar(Temp);				/* Draw the char */
		TextPtr = ((Byte *)TextPtr)+1;	/* Next char */
	}
}
