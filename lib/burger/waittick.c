#include "Burger.h"

/**********************************

	Wait for a single system tick

**********************************/

void WaitTick(void)
{
	LongWord NewTick;		/* Temp */
	LongWord LastTemp;

    LastTemp = LastTick;	/* Prefetch the last tick mark */
	do {
		NewTick = ReadTick();	/* Read in the current time tick */
	} while (NewTick==LastTemp);	/* Time has elapsed? */
	LastTick = NewTick;		/* Mark the time */
}
