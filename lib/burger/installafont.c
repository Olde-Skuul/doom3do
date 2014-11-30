#include "Burger.h"

/**********************************

	Install a font into my font system

**********************************/

#define FHeight	0	/* Index to the font height in pixels */
#define FLast	1	/* Index to the number of chars */
#define FFirst	2	/* Index to the first valid char */
#define FData	6	/* Index to the raw data */
#define FWidth	6	/* Index to the font width in pixels */

void InstallAFont(Word FontNum)
{
	void *TempPtr;
	if (FontLoaded==FontNum) {
		return;
	}
	if (FontLoaded) {
		ReleaseAResource(FontLoaded);
	}
	FontLoaded = FontNum;
	TempPtr = LoadAResource(FontNum);
	FontWidths = ((Byte *)TempPtr)+FWidth;
	FontHeight = SwapUShort(((Short *)TempPtr)[FHeight]);
	FontLast = SwapUShort(((Short *)TempPtr)[FLast]);
	FontFirst = SwapUShort(((Short *)TempPtr)[FFirst]);
	FontPointer = FontWidths+FontLast;
}
