#include <burger.h>

/********************************

	Sets an ENTIRE palette loaded from the resource fork

********************************/

void SetAPalette(Word PaletteNum)
{
	SetAPalettePtr(LoadAResource(PaletteNum));      /* Load and show */
	ReleaseAResource(PaletteNum);           /* Release it */
}
