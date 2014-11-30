#include "Burger.h"

/**********************************

	Wait for a system event to occur...

**********************************/

Word WaitEvent(void)
{
	Word EventNum;
	do {
		EventNum = WaitTicksEvent(6000);
	} while (!EventNum);
	return EventNum;
}
