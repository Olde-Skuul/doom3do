#include "burger.h"

/********************************

	Init the YTable for a NEW video width variable.
	Note : I have to deal with the wacko way the 3DO deals 
	with video memory, it's NOT truly linear!

********************************/

void InitYTable(void)
{
	LongWord *EndTablePtr;
	LongWord Offset;
	Word Width;
	LongWord *YTablePtr;
	Offset = 0;
	Width = VideoWidth*2;		/* Adjust the 3DO lines */
	YTablePtr = (LongWord *)YTable;
	EndTablePtr = &YTablePtr[240];
	do {
		*YTablePtr++ = Offset;	/* Store twice! */
		*YTablePtr++ = Offset;
		Offset += Width;
	} while (YTablePtr<EndTablePtr);
}
