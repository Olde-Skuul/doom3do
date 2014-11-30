#include "Burger.h"
#include <String.h>

/**********************************

	Save the current state of the font system

**********************************/

void FontSaveState(FontState_t *StatePtr)
{
	StatePtr->FontX = FontX;
	StatePtr->FontY = FontY;
	StatePtr->FontLoaded = FontLoaded;
	StatePtr->FontInvisible = FontInvisible;
	memcpy(&StatePtr->FontOrMask[0],FontOrMask,sizeof(FontOrMask));
}
