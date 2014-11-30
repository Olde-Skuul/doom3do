#include "Burger.h"

/**********************************

	If there are functions tied to the end
	of the program, call it now!!

**********************************/

extern void (*___atexit)(void);		/* Redirect the exit routine here... */

static Word ExitCount = 0;			/* Number of valid pointers */
static void (*ExitList[32])(void);	/* List of pointers */

static void AtexitWrapup(void)
{
	Word Index;

	Index = ExitCount;	/* How many routines are here? */
	ExitCount = 32;		/* Make sure that atexit will fail */
	do {
		--Index;		/* Count down */
		ExitList[Index]();	/* Call the procedure */
	} while (Index);	/* Any more? */
}

/**********************************

	ANSI compatible atexit routine for PROPER cleanup
	of "C" programs on the 3DO.

**********************************/

int atexit(void (*FuncPtr)(void))
{
	if (ExitCount>=32) {	/* Is the list full? */
		return -1;			/* Error! */
	}
	ExitList[ExitCount] = FuncPtr;	/* Save the vector */
	++ExitCount;				/* Inc the index */
	___atexit = AtexitWrapup;	/* Set the vector to me... */
	return 0;				/* No error */
}
