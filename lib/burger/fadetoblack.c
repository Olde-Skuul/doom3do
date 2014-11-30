#include "Burger.h"
#include <String.h>

/**********************************

	Fade the palette to black

**********************************/

void FadeToBlackX(void)
{
	Byte TempPalette[32*4];		/* Pass a palette */
	Word i;

	memset(TempPalette,0,sizeof(TempPalette));
	i = 0;
	do {
		TempPalette[i*4] = i;		/* Create the color indexs */
	} while (++i<32);
	FadeToPtr(TempPalette);			/* Fade the image */
}
