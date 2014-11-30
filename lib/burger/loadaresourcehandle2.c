#include "burger.h"
#include <stdio.h>
#include <filestreamfunctions.h>

/********************************

    Load in a resource and return the handle.
	The returned is NOT locked...

********************************/

extern Word RezIndexFound;	/* Which file had the entry? */
extern Stream *Rezfp[8];		/* Array of open files */

void **LoadAResourceHandle2(Word RezNum,Word Type)
{
    MyRezEntry2 *Entry;
    void **BufferPtr;
	LongWord Offset;
	Word Flags;
	Stream *fp;

    Entry = ScanRezMap(RezNum,Type);
    if (Entry) { /* Find the entry */
		BufferPtr = Entry->MemPtr;
        if (BufferPtr) {     /* Valid handle? */
            if (*BufferPtr) {       /* Handle not purged? */
				SetHandlePurgeFlag(BufferPtr,FALSE);	/* Not purgable */
                return BufferPtr;   /* Return */
            }
            DeallocAHandle(BufferPtr);   /* Release the memory */
            Entry->MemPtr = 0;              /* Mark as gone! */
        }
		Offset = Entry->Offset;
		Flags = 0xFFF0UL<<16;
		if (Offset&0x80000000) {
			Flags |= HANDLEFIXED;
		}
        BufferPtr = AllocAHandle2(Entry->Length,Flags); /* Get the memory */
        if (BufferPtr) {        /* Memory ok? */
        	fp = Rezfp[RezIndexFound];
            Entry->MemPtr = BufferPtr;       /* Save the handle */
			LockMusic();
			SeekDiskStream(fp,Offset&0x3FFFFFFF,SEEK_SET); /* Seek into the file */
            ReadDiskStream(fp,(char *)LockAHandle(BufferPtr),Entry->Length);  /* Read it in */
        	UnlockMusic();
			UnlockAHandle(BufferPtr);
			return BufferPtr;           /* Return the buffer pointer */
		}
    }
    return 0;           /* No data */
}

