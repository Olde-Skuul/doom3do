#include "Burger.h"
#include <String.h>

/**********************************

	Restore the current state of the font system

**********************************/

void FontRestoreState(FontState_t *StatePtr)
{
	if (StatePtr->FontLoaded) {
		FontX = StatePtr->FontX;
		FontY = StatePtr->FontY;
		InstallAFont(StatePtr->FontLoaded);
		FontInvisible = StatePtr->FontInvisible;
		memcpy(FontOrMask,&StatePtr->FontOrMask[0],sizeof(FontOrMask));
	}
}
