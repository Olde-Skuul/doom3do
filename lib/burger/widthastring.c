#include "Burger.h"

/**********************************

	Return the width of a string in pixels

**********************************/

Word WidthAString(void *TextPtr)
{
	Word Result;
	Word Last;
	Word First;
	Byte Val;
	Byte *WidthTbl;

	Result = 0;
	WidthTbl = FontWidths;
	Last = FontLast;
	First = FontFirst;
	while ((Val=((Byte *)TextPtr)[0])!=0) {
		Val-=First;
		if (Val<Last) {
			Result+=WidthTbl[Val];
		}
		TextPtr = ((Byte *)TextPtr)+1;
	}
	return Result;
}
