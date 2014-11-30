#include "Burger.h"

/**********************************

	All variables dealing with the font system

**********************************/

Word FontX;				/* X coord to draw the font */
Word FontY;				/* Y coord to draw the font */
void *FontPointer;		/* Pointer to the active font */
Byte *FontWidths;		/* Width of the font in pixels (Table ptr) */
Word FontHeight;		/* Heigth of the font in pixels */
Word FontLast;			/* Last char I can draw */
Word FontFirst;			/* First allowable font to draw */
Word FontLoaded;		/* Resource ID of the last font loaded */
Word FontInvisible;		/* And masks for font */
Word FontOrMask[16];	/* Or masks for font */
