#include "Burger.h"

/**********************************

	Wait for a number of system ticks

**********************************/

void WaitTicks(Word Delay)
{
	LongWord NewTick;		/* Temp */
	LongWord LastTemp;		/* Register variable */

    LastTemp = LastTick;	/* Prefetch the last tick mark */
	do {
		NewTick = ReadTick();	/* Read in the current time tick */
	} while ((NewTick-LastTemp)<(LongWord)Delay);	/* Time has elapsed? */
	LastTick = NewTick;		/* Mark the time */
}
