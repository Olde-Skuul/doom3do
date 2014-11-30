#include "Burger.h"

/**********************************

	Wait for a key from the 3DO keyboard

**********************************/

Word (*MyGetchCallBack)(Word);

Word MyGetch(void)			/* What 3do keyboard!?? */
{
	Word Key;

    Key = 0;
	if (MyGetchCallBack) {		/* Pass the key through the filter */
		Key = MyGetchCallBack(Key);
	}
	return Key;			/* Exit with the key */
}
