#include <burger.h>
#include <string.h>

/********************************

    Pretty palette fade

********************************/

Word FadeTimer = 4;

void FadeToPtr(void *PalettePtr)
{
    Word Count;
    Word i;
    int DestPalette[32*4];      /* Must be SIGNED! */
    Byte WorkPalette[32*4];
    
    if (!memcmp((char *)PalettePtr,(char *)CurrentPalette,32*4)) {   /* Same palette? */
        return;
    }
    memcpy(WorkPalette,CurrentPalette,32*4);		/* Save the current palette */
    i = 0;
    do {
        DestPalette[i]=((Byte *)PalettePtr)[i]-WorkPalette[i]; /* Get differances */
    } while (++i<32*4);
    Count = 1;
    do {
        i = 1;			/* Skip #0 */
        do {
        	if (i&3) {		/* Only affect the RGB values, not the counter */
            	CurrentPalette[i] = ((DestPalette[i]*(int)Count)>>4)+WorkPalette[i];
            }
        } while (++i<32*4);
        WaitTicks(FadeTimer);		/* 1/15th of a second */
        SetAPalettePtr(CurrentPalette);
    } while (++Count<17);
}
