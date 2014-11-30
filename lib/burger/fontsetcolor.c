#include "Burger.h"

/**********************************

	Set the color in the font

**********************************/

void FontSetColor(Word ColorNum,Word Color)
{
	Color = Color<<16;			/* Clear the high word */
	Color |= Color>>16;			/* Make both halves the same */
	FontOrMask[ColorNum] = Color;	/* Save the color */
}
