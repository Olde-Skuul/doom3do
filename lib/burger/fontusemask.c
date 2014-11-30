#include "Burger.h"

/**********************************

	Force the font manager to mask out color #0

**********************************/

void FontUseMask(void)
{
	FontInvisible = 0;		/* Set black as the invisible color */
	FontSetColor(0,0);		/* Set color zero as maskable */
}
