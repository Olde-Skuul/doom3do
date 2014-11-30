#include "Burger.h"
#include <Stdio.h>

/**********************************

	For debugging the handle list.

**********************************/

extern MyHandle UsedHands;       /* Pointer to the used handle list */
extern MyHandle PurgeHands;      /* Pointer to the purged handle list */

/**********************************

	Print a list of handles with records of free
	memory.

**********************************/

static void PrintHandles(MyHandle *First)
{
	MyHandle *Scan;
	MyHandle *Next;

	Word Count;
	Count = 1;
	printf("#    Handle  Addr  Attr  ID    Size    Prev   Next\n");
	Scan = First->NextHandle;		/* Index to the first valid handle */
	if (Scan!=First) {
		for (;;) {
			printf("%04X ",Count);
			printf("%06X ",Scan);
			printf("%06X ",Scan->MemPtr);
			printf("%04X ",Scan->Flags&0xFFFF);
			printf("%04X ",Scan->Flags>>16);
			printf("%08X ",Scan->Length);
			printf("%06X ",Scan->PrevHandle);
			printf("%06X\n",Scan->NextHandle);
			Next = Scan->NextHandle;
			if (Next==First) {
				break;
			}
			if (Scan->MemPtr && Next->MemPtr) {
				LongWord Free;
				Byte *Start;
				Start = ((Byte *)Scan->MemPtr) + Scan->Length;
				Free = ((Byte *)Next->MemPtr) - Start;
				if (Free) {
					printf("            %06X           %08X\n",Start,Free);
				}
			}
			Scan = Next;
			++Count;
		}
	}
}

/**********************************

	Display all the memory

**********************************/

void DumpHandles(void)
{
	LongWord Size;
	Size = GetTotalFreeMem();
	printf("Total free mem with purging %lX\n",Size);

	printf("Used handle list\n");
	PrintHandles(&UsedHands);
	printf("Purged handle list\n");
	PrintHandles(&PurgeHands);
}
