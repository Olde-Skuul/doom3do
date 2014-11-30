#include <burger.h>

/********************************

	Wait for a keypress, but call MyKbhit to allow
	screen savers to kick in.

********************************/

Word WaitAKey(void)
{
    while (!MyKbhit()) {}    /* Key pending? */
    return MyGetch();   /* Return the key */
}
