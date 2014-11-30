#include "burger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <FileStreamFunctions.h>

#define MAXREZFILES 8

Stream *Rezfp[MAXREZFILES];				/* Open file referance for resource files */
Word RezIndexFound;			/* Which file was found in ScanRez */
static void **RezHandles[MAXREZFILES];       /* Handles to resource maps */
static Word RezCounts[MAXREZFILES];          /* Number of active resources */
static Boolean OnceHit=FALSE; /* True if already init'd once */
static Word OpenFileCount;		/* Number of open files */

/********************************

    Release all the data allocated by the resource manager

********************************/

static void DeInitResource(void)
{
	Word i;

	i = OpenFileCount;		/* Any files open? */
	if (i) {
		do {
			--i;
			CloseAResourceFile(i);		/* Close all resource files */
    	} while (i);
	}
}

/********************************

	Close a resource file
	Use the passed "Magic" resource referance and see
	if I am ok.

********************************/

void CloseAResourceFile(Word RezRef)
{
	if (RezRef<OpenFileCount) {		/* Failsafe */
 		Word GroupCount;		/* Number of resource groups */
      	CloseDiskStream(Rezfp[RezRef]);      /* Close the open resource file */
		GroupCount = RezCounts[RezRef];		/* Any valid entries? */
		if (GroupCount) {
			MyRezEntry *MainPtr;	/* Group array pointer */
			MainPtr = (MyRezEntry*)LockAHandle(RezHandles[RezRef]);	/* Lock it down */
			do {
				Word EntryCount;		/* Number of entries per group */
				MyRezEntry2 *EntryPtr;	/* Entry pointer */
				EntryPtr = &MainPtr->Array[0];
				EntryCount = MainPtr->Count;
				do {
					if (EntryPtr->MemPtr) {		/* Dispose of all handles */
						DeallocAHandle(EntryPtr->MemPtr);
					}
					++EntryPtr;		/* Next entry */
				} while (--EntryCount);
				MainPtr = (MyRezEntry *)EntryPtr;
			} while (--GroupCount);
		}
       	DeallocAHandle(RezHandles[RezRef]);  /* Free the resource map */
		--OpenFileCount;				/* Remove from the list */
		if (RezRef<OpenFileCount) {		/* Not the last one? */
			do {
				Word Next;		/* Move all other entries down one */
				Next = RezRef+1;
				Rezfp[RezRef] = Rezfp[Next];
				RezHandles[RezRef] = RezHandles[Next];
				RezCounts[RezRef] = RezCounts[Next];
				RezRef = Next;
			} while (RezRef<OpenFileCount);
		}
	}
}

/********************************

	Open a resource file for reading

********************************/

Word OpenAResourceFile(Byte *FileName)
{
    MyRezHeader MyHeader;      /* Struct for resource file header */
	void **RezHandle;
	Word Index;
	Stream *fp;

	Index = OpenFileCount;
	if (Index<MAXREZFILES) {		/* Open index? */
	    fp = OpenDiskStream((char *)FileName,0);        /* Open the resource file */
	    if (!fp) {                           /* Did the file open? */
       		NonFatal("Can't open the resource file");   /* Nope, leave */
	        return -1;
	    }
		ReadDiskStream(fp,(char *)&MyHeader,sizeof(MyHeader));  /* Read in the header */
		if (memcmp((char *)&MyHeader.Name,"BRGR",4)) {	/* Valid header? */
			NonFatal("Invalid Resource file!");
			return -1;
		}
	    RezHandle = AllocAHandle2(MyHeader.MemSize,0xFFF1UL<<16);      /* Get the memory */
	    if (!RezHandle) {
			CloseDiskStream(fp);		/* Release the file */
	        return -1;	/* Return the error code from memory manager */
	    }
        ReadDiskStream(fp,(char *)LockAHandle(RezHandle),MyHeader.MemSize);
		UnlockAHandle(RezHandle);
		Rezfp[Index] = fp;		/* Save the file referance */
		RezCounts[Index] = MyHeader.Count;	/* Get the resource count */
	    RezHandles[Index] = RezHandle;      /* Get the memory */
		OpenFileCount = Index+1;			/* I now have an open file! */
		return Index;
	}
	NonFatal("Resource map full!");		/* Too many files! */
	return -1;	
}

/********************************

    Initialize the resource manager so I can
    access all my resource chunks

********************************/

void InitResource(void)
{
    if (!OnceHit) {             /* Release resources on shutdown */
        OnceHit = TRUE;
        atexit(DeInitResource);                 /* Allow closing on exit */
    }
	OpenAResourceFile((Byte *)RezFileName);		/* Open the default resource file */
}

/********************************

    Scan for a resource entry in the resource map

********************************/

/********************************

    Scan for a resource entry in the resource map
	Note: I scan from the highest map to the LOWEST!!

********************************/

MyRezEntry2 *ScanRezMap(Word RezNum,Word Type)
{
    MyRezEntry *MainPtr;
    Word GroupCount;
	Word Index;
	void **RezHandle;

	Index = OpenFileCount;			/* Which file to scan? */
	if (Index) {
		do {
			--Index;
   	    	GroupCount = RezCounts[Index];		/* Get the entry count */
		    if (GroupCount) {     /* Any resources available? */
    	    	RezHandle = RezHandles[Index];     /* Set the base pointer */
				MainPtr = (MyRezEntry *)*RezHandle;
	        	do {
    	        	if (MainPtr->Type==Type) {		/* Right type? */
						Word Temp;
						Temp = RezNum-MainPtr->RezNum;	/* Get the offset from base number */
						if (Temp<MainPtr->Count) {
							RezIndexFound = Index;		/* Return the file ref */
							return &MainPtr->Array[Temp];
						}
            		}
					MainPtr = (MyRezEntry *)&MainPtr->Array[MainPtr->Count];     /* Next? */
    	    	} while (--GroupCount);      /* Count down */
	    	}
		} while (Index);
	}
    NonFatal("Resource not in map");
    return 0;           /* No good! */
}

