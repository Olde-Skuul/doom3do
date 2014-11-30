#include "Burger.h"

/**********************************

	Check if a key is pending for the 3DO keyboard

**********************************/

void (*MyKbhitCallBack)(void);

Word MyKbhit(void)			/* What 3do keyboard!?? */
{
			/* First check for pending keys... */
	if (MyKbhitCallBack) {		/* If no key is waiting then call polled events */
		MyKbhitCallBack();
	}
	return 0;			/* Exit with the key pending */
}
