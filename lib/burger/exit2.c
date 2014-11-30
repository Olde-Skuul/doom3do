#include "Burger.h"
#undef exit				/* Override the exit routine */
#include <stdlib.h>

/**********************************

	If there are functions tied to the end
	of the program, call it now!!

**********************************/

static void NullProc(void)		/* Doesn't do much... */
{
}

void (*___atexit)(void) = NullProc;	/* Patched if atexit was called */

void exit2(int ErrCode)
{
	___atexit();			/* Call the exit procedures */
	exit(ErrCode);			/* Tell the 3do to die... */
}
