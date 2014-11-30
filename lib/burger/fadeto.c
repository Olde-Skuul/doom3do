#include <burger.h>  

/********************************

	Fade the palette to a color set

********************************/

void FadeTo(Word PaletteNum)
{
	FadeToPtr(LoadAResource(PaletteNum));   /* Load in the resource file */
	ReleaseAResource(PaletteNum);   /* Release the resource data */
}
