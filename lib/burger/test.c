#include <burger.h>
#include <stdio.h>
#include <string.h>
#include <BlockFile.h>
#include <Init3do.h>
#include <Event.h>

#define SCREENS 2
uint32 MainTask;		/* My own task item */
ulong ScreenPageCount;	/* Number of screens */
Item ScreenItems[SCREENS];	/* Referances to the game screens */
Item VideoItems[SCREENS];
long ScreenByteCount;		/* How many bytes for each screen */
Item ScreenGroupItem;		/* Main screen referance */
ubyte *ScreenMaps[SCREENS];	/* Pointer to the bitmap screens */
Bitmap *Bitmaps[SCREENS];
Item VRAMIOReq;				/* IO Request item for SPORT transfer */

/********************************************

	Start up all the tools for the 3DO system
	Return TRUE if all systems are GO!

********************************************/

void InitTools(void)
{
	Word i;		/* Temp */
	long width, height;	/* Screen width & height */
	Screen *screen;	/* Pointer to screen info */
	ItemNode *Node;

		/* Read page PRF-85 for info */

	TagArg ScreenTags[] =	{		/* Change this to change the screen count! */
		CSG_TAG_SPORTBITS, (void *)0,	/* Allow SPORT DMA (Must be FIRST) */
		CSG_TAG_SCREENCOUNT, (void *)SCREENS,	/* How many screens to make! */
		CSG_TAG_DONE, 0			/* End of list */
	};

	ChangeInitialDirectory(NULL,NULL, false ); /* Set to /CD-ROM or /remote */

#if 0		/* Remove for final build! */
	ChangeDirectory("/CD-ROM");
#endif

	if (OpenGraphicsFolio()) {	/* Start up the graphics system */
		exit(1);	/* Phooey */
	}

	OpenMacLink();	/* No checking since the 3DO doesn't come with a free mac */
				/* And if it dies here, the program is toast anyways... */

	if (!OpenSPORT()) {		/* Get a DMA channel */
		exit(1);
	}
	if (!OpenAudio()) {		/* Start up the audio system */
		exit(1);
	}
	if (OpenMathFolio()<0) {
		exit(1);
	}

	ScreenPageCount = 0;	/* No screen pages are active */
	ScreenTags[0].ta_Arg = (void *)GETBANKBITS(GrafBase->gf_ZeroPage);
	ScreenGroupItem = CreateScreenGroup(ScreenItems,ScreenTags);

	if (ScreenGroupItem<0) {		/* Error creating screens? */
		exit(1);
	}
	AddScreenGroup(ScreenGroupItem,NULL);		/* Add my screens tothe system */

	screen = (Screen *)LookupItem(ScreenItems[0]);
	if (!screen) {
		exit(1);
	}

	width = screen->scr_TempBitmap->bm_Width;		/* How big is the screen? */
	height = screen->scr_TempBitmap->bm_Height;

	ScreenPageCount = (width*2*height+GrafBase->gf_VRAMPageSize-1)/GrafBase->gf_VRAMPageSize;
	ScreenByteCount = ScreenPageCount * GrafBase->gf_VRAMPageSize;

	i = 0;
	do {		/* I need 4 screens */
		DisableHAVG(ScreenItems[i]);	/* Kill anti-aliasing */
		DisableVAVG(ScreenItems[i]);
		screen = (Screen *)LookupItem(ScreenItems[i]);
		ScreenMaps[i] = screen->scr_TempBitmap->bm_Buffer;
		Bitmaps[i] = screen->scr_TempBitmap;
		memset(ScreenMaps[i],0,ScreenByteCount);	/* Clear the screen */
		Node = (ItemNode *) screen->scr_TempBitmap;	/* Get the bitmap pointer */
		VideoItems[i] = (Item) Node->n_Item;			/* Get the bitmap item # */
		SetClipHeight(VideoItems[i],200);
		SetClipWidth(VideoItems[i],0);
		SetClipOrigin(VideoItems[i],0,20);		/* Set the clip top for the screen */
	} while (++i<SCREENS);

	InitEventUtility(1,1,0);	/* I want 1 joypad, 1 mouse, and passive listening */

	/* I need these signals for communication with my music task */

	MainTask = KernelBase->kb_CurrentTask->t.n_Item;	/* My task Item */
	VRAMIOReq = GetVRAMIOReq();
	DisplayScreen(ScreenItems[0],0);
	VideoItem = VideoItems[0];		/* Set the new page */
	VideoScreen = ScreenItems[0];
	VideoPointer = (Byte *)&ScreenMaps[0][640*20];	/* Set the raw pointer */
}

int main(void)
{
	Word i;
	Byte Wowzers[20];

	InitTools();
	InitMemory();			/* Init the memory manager */
	InitResource();			/* Init the resource manager */
	Randomize();
	DrawARect(0,0,320,200,GREEN);
	InstallAFont(1);		/* Install the basic font */
	FontUseMask();
	FontSetXY(50,0);
	FontSetColor(1,WHITE);
	DrawAString("This is a test");
	FontSetXY(70,10);
	LongWordToAscii(-1,Wowzers);
	DrawAString(Wowzers);
	WaitTicksEvent(5*TICKSPERSEC);
	FadeToBlack();
	return 0;
}
