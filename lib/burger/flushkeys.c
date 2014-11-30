#include "Burger.h"
#include <String.h>

/**********************************

	Flush the keyboard buffer of all events

**********************************/

void FlushKeys(void)
{
	while (MyKbhit()) {		/* Any keys pending? */
		MyGetch();			/* Remove from queue */
	}
	memset((char *)KeyArray,0,sizeof(KeyArray));	/* Clear out the keydown array */
}
