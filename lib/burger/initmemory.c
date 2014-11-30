#include "burger.h"
#include <stdlib.h>
#include <string.h>
#include <mem.h>

/********************************

	Initialize the memory manager so I can have purgable memory
	chunks.

********************************/

#define NUMOFCHUNKS 8

LongWord MaxMemSize = 0x400000;		/* Maximum memory the program will take (4M) */
LongWord MinReservedSize = 0x8000;	/* Minimum memory for OS (32K) */
Word MinHandles = 500;		/* Number of handles to create */
MyHandle UsedHands = {		/* Pointer to the used handle list */
	0,0,(0xFFFEUL<<16)|HANDLELOCK|HANDLEFIXED,&UsedHands,&UsedHands};
MyHandle *FreeHands;		/* Pointer to the free handle list */
MyHandle PurgeHands = {		/* Pointer to the purged handle list */
	0,0,(0xFFFEUL<<16)|0,&PurgeHands,&PurgeHands};
MyHandle *PrevAllocHand;	/* Handle previously allocated */
MyHandle *PrevAllocFixedHand;	/* Fixed handle last allocated */
void (*MemPurgeCallBack)(Word Stage) = 0;	/* Callback before memory purging */
static Byte *ChunksFromHell[NUMOFCHUNKS]; /* Pointers to true memory chunk */

static char ErrCantStartMM[] = "Can't start memory manager";

/**********************************

	Die horribly!

**********************************/

static void Die(void)
{
	Fatal(ErrCantStartMM);	/* Die! */
}

/**********************************

	Release memory back to the system on system death

**********************************/

static void DeInitMemory(void)
{
	Word i;
	i = 0;
	do {
		if (ChunksFromHell[i]) {
			free(ChunksFromHell[i]);		/* Release the memory */
			ChunksFromHell[i] = 0;			/* Not used anymore */
		}
	} while (++i<NUMOFCHUNKS);
}

/**********************************

	Allocate a chunk of true memory from the system
	memory pool.
	Return the size of the chunk allocated, save the pointer in
	ChunksFromHell[i].

**********************************/

static LongWord GotAChunk(Word i,LongWord MaxNeeded)
{
	void *MemPtr;
	LongWord Size;
	MemInfo WhatAboutIt;		/* 3DO memory manager info struct */

	MemPtr = malloc(MaxNeeded);		/* Do it the easy way!! */
	if (MemPtr) {					/* Got it? */
		ChunksFromHell[i] = (Byte *)MemPtr;		/* Save the memory */
		return MaxNeeded;			/* Return allocated size */
	}
	availMem(&WhatAboutIt,MEMTYPE_ANY);	/* Get juicy details about memory */
	Size = WhatAboutIt.minfo_SysFree+WhatAboutIt.minfo_TaskFree;
    while (Size>=0xFFF) {
		MemPtr = malloc(Size); /* Get the maximum? */
		if (MemPtr) {
			ChunksFromHell[i] = (Byte*)MemPtr;
			return Size;
		}
		Size-=0x1000;
	}
	return 0;			/* Utter failure! */
}

/**********************************

	Init the memory manager

**********************************/

void InitMemory(void)
{
	LongWord Sizes[NUMOFCHUNKS];	/* Sizes of each chunk */
	Byte *MemPtr;		/* Temp buffer ptr */
	Byte *ReservePtr;	/* Reserved memory pointer */
	LongWord Size;		/* Temp size */
	LongWord SmallSize;	/* Size for handles */
	MyHandle *Scan;		/* Running Handle ptr */
	MyHandle *Prev;		/* Previous handle */
	Word i;				/* Index */
	Word Chunks;		/* Number of memory chunks */
	Word Hit;			/* Hit flag for sort */

	atexit(DeInitMemory);		/* Add clean up code */
	ReservePtr = (Byte *)malloc(MinReservedSize);	/* Get the reserved memory */
	if (!ReservePtr) {
		Die();			/* Die! */
	}

	/* Get the memory chunks */

	memset(Sizes,0,sizeof(Sizes));		/* Clear out the table */
	Size = MaxMemSize;				/* How much memory I want? */
	Chunks = 0;						/* How many chunks are there? */
	do {
		Sizes[Chunks] = GotAChunk(Chunks,Size);		/* Get some memory */
		if (!Sizes[Chunks]) {		/* Allocated a memory chunk? */
			break;				/* Break out if not */
		}
		Size -= Sizes[Chunks];				/* Remove from request count */
		++Chunks;
	} while (Size && Chunks<NUMOFCHUNKS);	/* Need more? */

	if (!Chunks) {	/* Somehow I didn't get any memory! */
		Die();		/* Die! */
	}

	/* Sort the chunks by memory address */

	if (Chunks>=2) {		/* Should I sort them? */
		do {
			Hit = FALSE;	/* No entries were swapped */
			i = 0;			/* Init index */
			do {
				if (ChunksFromHell[i+1]<ChunksFromHell[i]) {	/* Swap? */
                    MemPtr = ChunksFromHell[i];
                    ChunksFromHell[i] = ChunksFromHell[i+1];
                    ChunksFromHell[i+1] = (Byte*)MemPtr;
                    Size = Sizes[i];
                    Sizes[i] = Sizes[i+1];
                    Sizes[i+1] = Size;
					Hit = TRUE;
				}
			} while (++i<(Chunks-1));
		} while (Hit);
	}

/* Place the handles into the SMALLEST memory chunk to prevent fragmentation */

	Size = MinHandles*sizeof(MyHandle);		/* Memory needed for handles */
	SmallSize = -1;			/* Init the variable */
	i = 0;				/* Init index */
	Hit = 0;
	do {
		if ((Sizes[i]<SmallSize) && (Sizes[i]>=Size)) {
			Hit = i;		/* Save the index */
			SmallSize = Sizes[i];	/* Set the new size */
		}
	} while (++i<Chunks);

	if (SmallSize==-1) {	/* You've got to be kidding me!! */
		Die();				/* Die! */
	}

/* Init the handle list */

	Scan = (MyHandle *)ChunksFromHell[Hit];	/* Init the free handle list */
	UsedHands.NextHandle = Scan;		/* Index to the first used handle */

	i = MinHandles-1;		/* Create the forward linked lists */
	do {
		Scan->NextHandle = Scan+1;	/* Create the list */
		++Scan;				/* Next index */
	} while (--i);
	Scan->NextHandle = 0;	/* Zap the last entry */

	ChunksFromHell[Hit]+=Size;		/* Remove memory used by the handles */
	Sizes[Hit]-=Size;				/* Remove the free space */

/* Create the handle list */

	Scan = UsedHands.NextHandle;		/* Get the first used handle */
	FreeHands = Scan+(Chunks+1);	/* Index to the first free handle */
	i = 0;
	MemPtr = 0;
	Prev = &UsedHands;
	do {
		Scan->Flags = (0xFFFEUL<<16)|HANDLELOCK|HANDLEFIXED;	/* Lock it down */
		Scan->MemPtr = MemPtr;			/* First memory byte */
		Scan->Length = ChunksFromHell[i]-MemPtr;	/* Length to first chunk of memory */
		Scan->PrevHandle = Prev;
		Prev = Scan;
		++Scan;						/* Next handle (Also it's the LAST!) */
		MemPtr = ChunksFromHell[i]+Sizes[i];

	} while (++i<Chunks);
	Scan->Flags = (0xFFFEUL<<16)|HANDLELOCK|HANDLEFIXED;	/* Lock it down */
	Scan->MemPtr = MemPtr;		/* Last byte to use */
	Scan->Length = 0xFFFFFFFC - (LongWord)MemPtr; /* Length to top */
	Scan->NextHandle = &UsedHands;		/* No more handles */
	Scan->PrevHandle = Prev;
	UsedHands.PrevHandle = Scan;

	PurgeHands.NextHandle = &PurgeHands;	/* No handles are purged */
	PurgeHands.PrevHandle = &PurgeHands;
	PrevAllocHand = UsedHands.NextHandle;		/* Init the previous handle */
	PrevAllocFixedHand = UsedHands.PrevHandle;	
	free(ReservePtr);		/* Release the system memory */
	ScavengeMem();			/* Send back to the system if needed */
}
