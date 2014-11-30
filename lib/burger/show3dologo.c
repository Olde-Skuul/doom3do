#include "Burger.h"
#include <Task.h>
#include <Filefunctions.h>

#define LOGOPROG "Logo"

/**********************************

	This routine will load an executable program
	that will display the 3DO logo and then fade to black

**********************************/

void Show3DOLogo(void)
{
	Item LogoItem;
	LogoItem=LoadProgram(LOGOPROG);		/* Load and begin execution */
	do {
		Yield();						/* Yield all CPU time to the other program */
	} while (LookupItem(LogoItem));		/* Wait until the program quits */
	DeleteItem(LogoItem);				/* Dispose of the 3DO logo code */
}
