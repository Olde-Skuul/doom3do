#include "Doom.h"
#include <Portfolio.h>
#include <event.h>
#include <Init3do.h>
#include <FileFunctions.h>
#include <stdio.h>
#include <IntMath.h>
#include <BlockFile.h>
#include <Time.h>
#include <audio.h>
#include <celutils.h>

/**********************************

	This contains all the 3DO specific calls for Doom
	
**********************************/

typedef struct MyCCB {		/* Clone of the CCB Block from the 3DO includes */
	uint32 ccb_Flags;
	struct MyCCB *ccb_NextPtr;
	CelData    *ccb_SourcePtr;
	void       *ccb_PLUTPtr;
	Coord ccb_XPos;
	Coord ccb_YPos;
	int32  ccb_HDX;
	int32  ccb_HDY;
	int32  ccb_VDX;
	int32  ccb_VDY;
	int32  ccb_HDDX;
	int32  ccb_HDDY;
	uint32 ccb_PIXC;
	uint32 ccb_PRE0;
	uint32 ccb_PRE1;
} MyCCB;			/* I DON'T include width and height */

#define LIGHTSCALESHIFT 3
#if 0
	0x0000,0x0400,0x0800,0x0C00,0x1000,0x1400,0x1800,0x1C00,	/* 1/16 - 8/16 */
	0x00D0,0x1300,0x08D0,0x1700,0x10D0,0x1B00,0x18D0,0x1F00,
#endif

static Word LightTable[] = {
	0x0000,0x0400,0x0800,0x0C00,0x1000,0x1400,0x1800,0x1C00,	/* 1/16 - 8/16 */
	0x00D0,0x00D0,0x1300,0x1300,0x08D0,0x08D0,0x1700,0x1700,
	0x10D0,0x10D0,0x1B00,0x1B00,0x18D0,0x18D0,0x1F00,0x1F00,
	0x1F00,0x1F00,0x1F00,0x1F00,0x1F00,0x1F00,0x1F00,0x1F00,
};
static void FlushCCBs(void);
static void LowMemCode(Word Type);
static void WipeDoom(LongWord *OldScreen,LongWord *NewScreen);
extern void DrawASpan(Word Count,LongWord xfrac,LongWord yfrac,Fixed ds_xstep,Fixed ds_ystep,
	Byte *Dest);

#define CCBTotal 0x200

static MyCCB CCBArray[CCBTotal];		/* Array of CCB structs */
static MyCCB *CurrentCCB = &CCBArray[0];	/* Pointer to empty CCB */
static LongWord LastTicCount;	/* Time mark for page flipping */
LongWord LastTics;				/* Time elapsed since last page flip */
Word WorkPage;					/* Which frame is not being displayed */
static Byte *CelLine190;

Byte SpanArray[MAXSCREENWIDTH*MAXSCREENHEIGHT];	/* Buffer for floor textures */
Byte *SpanPtr = SpanArray;		/* Pointer to empty buffer */

#define SKYSCALE(x) (Fixed)(1048576.0*(x/160.0))

static Fixed SkyScales[6] = {
	SKYSCALE(160.0),
	SKYSCALE(144.0),
	SKYSCALE(128.0),
	SKYSCALE(112.0),
	SKYSCALE(96.0),
	SKYSCALE(80.0)
};

#define SCREENS 3					/* Need to page flip */
uint32 MainTask;					/* My own task item */
static ulong ScreenPageCount;		/* Number of screens */
static Item ScreenItems[SCREENS];	/* Referances to the game screens */
static Item VideoItems[SCREENS];
static long ScreenByteCount;		/* How many bytes for each screen */
static Item ScreenGroupItem = 0;	/* Main screen referance */
static Byte *ScreenMaps[SCREENS];	/* Pointer to the bitmap screens */
static Item VRAMIOReq;				/* I/O Request for screen copy */
Item AllSamples[NUMSFX];			/* Items to sound samples */
Word AllRates[NUMSFX];

/**********************************

	Run an external program and wait for compleation

**********************************/

static void RunAProgram(char *ProgramName)
{
	Item LogoItem;
	LogoItem=LoadProgram(ProgramName);		/* Load and begin execution */
	do {
		Yield();						/* Yield all CPU time to the other program */
	} while (LookupItem(LogoItem));		/* Wait until the program quits */
	DeleteItem(LogoItem);				/* Dispose of the 3DO logo code */
}

/**********************************

	Init the 3DO variables to a specific screen

**********************************/

static void SetMyScreen(Word Page)
{
	VideoItem = VideoItems[Page];			/* Get the bitmap item # */
	VideoScreen = ScreenItems[Page];
	VideoPointer = (Byte *) &ScreenMaps[Page][0];
	CelLine190 = (Byte *) &VideoPointer[190*640];
}

/**********************************

	Init the system tools

	Start up all the tools for the 3DO system
	Return TRUE if all systems are GO!

**********************************/

static Word HeightArray[1] = 200;		/* I want 200 lines for display memory */
static Word MyCustomVDL[] = {
	VDL_RELSEL|			/* Relative pointer to next VDL */
	(1<<VDL_LEN_SHIFT)|		/* (DMA) 1 control words in this VDL entry */
	(20<<VDL_LINE_SHIFT),	/* Scan lines to persist */
	0,				/* Current video buffer */
	0,				/* Previous video buffer */
	4*4,			/* Pointer to next vdl */
	0xE0000000,		/* Set the screen to BLACK */
	VDL_NOP,		/* Filler to align to 16 bytes */
	VDL_NOP,
	VDL_NOP,

	VDL_RELSEL|
	VDL_ENVIDDMA|				/* Enable video DMA */
	VDL_LDCUR|					/* Load current address */
	VDL_LDPREV|					/* Load previous address */
	((32+2)<<VDL_LEN_SHIFT)|			/* (DMA) 2 control words in this VDL entry */
	(198<<VDL_LINE_SHIFT),		/* Scan lines to persist */
	0,				/* Current video buffer */
	0,				/* Previous video buffer */
	(32+4)*4,			/* Pointer to next vdl */

	VDL_DISPCTRL|	/* Video display control word */
	VDL_CLUTBYPASSEN|	/* Allow fixed clut */
	VDL_WINBLSB_BLUE|	/* Normal blue */
#if 1
	VDL_WINVINTEN|		/* Enable HV interpolation */
	VDL_WINHINTEN|
	VDL_VINTEN|
	VDL_HINTEN|
#endif
	VDL_BLSB_BLUE|		/* Normal */
	VDL_HSUB_FRAME,		/* Normal */
	0x00000000,		/* Default CLUT */
	0x01080808,
	0x02101010,
	0x03181818,
	0x04202020,
	0x05292929,
	0x06313131,
	0x07393939,
	0x08414141,
	0x094A4A4A,
	0x0A525252,
	0x0B5A5A5A,
	0x0C626262,
	0x0D6A6A6A,
	0x0E737373,
	0x0F7B7B7B,
	0x10838383,
	0x118B8B8B,
	0x12949494,
	0x139C9C9C,
	0x14A4A4A4,
	0x15ACACAC,
	0x16B4B4B4,
	0x17BDBDBD,
	0x18C5C5C5,
	0x19CDCDCD,
	0x1AD5D5D5,
	0x1BDEDEDE,
	0x1CE6E6E6,
	0x1DEEEEEE,
	0x1EF6F6F6,
	0x1FFFFFFF,
	0xE0000000,		/* Set the screen to BLACK */
	VDL_NOP,				/* Filler to align to 16 bytes */
	VDL_NOP,

	(1<<VDL_LEN_SHIFT)|			/* (DMA) 1 control words in this VDL entry */
	(0<<VDL_LINE_SHIFT),		/* Scan lines to persist (Forever) */
	0,				/* Current video buffer */
	0,				/* Previous video buffer */
	0,				/* Pointer to next vdl (None) */
	0xE0000000,		/* Set the screen to BLACK */
	VDL_NOP,				/* Filler to align to 16 bytes */
	VDL_NOP,
	VDL_NOP
};

static TagArg ScreenTags[] =	{		/* Change this to change the screen count! */
	CSG_TAG_SPORTBITS, (void *)0,	/* Allow SPORT DMA (Must be FIRST) */
	CSG_TAG_SCREENCOUNT, (void *)SCREENS,	/* How many screens to make! */
	CSG_TAG_BITMAPCOUNT,(void *)1,
	CSG_TAG_BITMAPHEIGHT_ARRAY,(void *)&HeightArray[0],
	CSG_TAG_DONE, 0			/* End of list */
};

static TagArg SoundRateArgs[] = {
	AF_TAG_SAMPLE_RATE,(void*)0,	/* Get the sample rate */
	TAG_END,0		/* End of the list */
};
static char FileName[32];

void InitTools(void)
{
	Word i;		/* Temp */
	long width, height;	/* Screen width & height */
	Screen *screen;	/* Pointer to screen info */
	ItemNode *Node;
	Item MyVDLItem;
		/* Read page PRF-85 for info */

#if 1
	Show3DOLogo();				/* Show the 3DO Logo */
	RunAProgram("IdLogo IDLogo.cel");
#if 1			/* Set to 1 for Japanese version */
	RunAProgram("IdLogo LogicLogo.cel");
	RunAProgram("PlayMovie EALogo.cine");
	RunAProgram("IdLogo AdiLogo.cel");
#else
	RunAProgram("PlayMovie Logic.cine");
	RunAProgram("PlayMovie AdiLogo.cine");
#endif
#endif

	if (OpenGraphicsFolio() ||	/* Start up the graphics system */
		(OpenAudioFolio()<0) ||		/* Start up the audio system */
		(OpenMathFolio()<0) ) {
FooBar:
		exit(10);
	}
	
#if 0	/* Set to 1 for the PAL version, 0 for the NTSC version */
	QueryGraphics(QUERYGRAF_TAG_DEFAULTDISPLAYTYPE,&width);
	if (width==DI_TYPE_NTSC) {
		goto FooBar();
	}
#endif

#if 0	/* Remove for final build! */
	ChangeDirectory("/CD-ROM");
#endif

	ScreenTags[0].ta_Arg = (void *)GETBANKBITS(GrafBase->gf_ZeroPage);
	ScreenGroupItem = CreateScreenGroup(ScreenItems,ScreenTags);

	if (ScreenGroupItem<0) {		/* Error creating screens? */
		goto FooBar;
	}
	AddScreenGroup(ScreenGroupItem,NULL);		/* Add my screens to the system */

	screen = (Screen *)LookupItem(ScreenItems[0]);
	if (!screen) {
		goto FooBar;
	}

	width = screen->scr_TempBitmap->bm_Width;		/* How big is the screen? */
	height = screen->scr_TempBitmap->bm_Height;

	ScreenPageCount = (width*2*height+GrafBase->gf_VRAMPageSize-1)/GrafBase->gf_VRAMPageSize;
	ScreenByteCount = ScreenPageCount * GrafBase->gf_VRAMPageSize;

	i=0;
	do {		/* Process the screens */
		screen = (Screen *)LookupItem(ScreenItems[i]);
		ScreenMaps[i] = (Byte *)screen->scr_TempBitmap->bm_Buffer;
		memset(ScreenMaps[i],0,ScreenByteCount);	/* Clear the screen */
		Node = (ItemNode *) screen->scr_TempBitmap;	/* Get the bitmap pointer */
		VideoItems[i] = (Item)Node->n_Item;			/* Get the bitmap item # */
		MyCustomVDL[9]=MyCustomVDL[10] = (Word)ScreenMaps[i];
		MyVDLItem = SubmitVDL((VDLEntry *)&MyCustomVDL[0],sizeof(MyCustomVDL)/4,VDLTYPE_FULL);
		SetVDL(ScreenItems[i],MyVDLItem);

		SetClipWidth(VideoItems[i],320);
		SetClipHeight(VideoItems[i],200);		/* I only want 200 lines */
		SetClipOrigin(VideoItems[i],0,0);		/* Set the clip top for the screen */
	} while (++i<SCREENS);

	InitEventUtility(1,1,FALSE);	/* I want 1 joypad, 1 mouse, and passive listening */

	InitSoundPlayer("system/audio/dsp/varmono8.dsp",0); /* Init memory for the sound player */
	InitMusicPlayer("system/audio/dsp/dcsqxdstereo.dsp");	/* Init memory for the music player */
//	InitMusicPlayer("system/audio/dsp/fixedstereosample.dsp");	/* Init memory for the music player */

	MainTask = KernelBase->kb_CurrentTask->t.n_Item;	/* My task Item */
	VRAMIOReq = GetVRAMIOReq();
	SetMyScreen(0);				/* Init the video display */

	i = 0;
	do {
		sprintf(FileName,"Sounds/Sound%02d.aiff",i+1);
		AllSamples[i] = LoadSample(FileName);
		if (AllSamples[i]<0) {
			AllSamples[i] = 0;
		}
		if (AllSamples[i]) {
			GetAudioItemInfo(AllSamples[i],SoundRateArgs);
			AllRates[i] = (Word)(((LongWord)SoundRateArgs[0].ta_Arg)/(44100UL*2UL));	/* Get the DSP rate for the sound */
		}
	} while (++i<(NUMSFX-1));

	MinHandles = 1200;		/* I will need lot's of memory handles */
	InitMemory();			/* Init the memory manager */
	InitResource();			/* Init the resource manager */
	InterceptKey();			/* Init events */
	SetErrBombFlag(TRUE);	/* Any OS errors will kill me */
	MemPurgeCallBack = LowMemCode;
	
	{
	MyCCB *CCBPtr;
	i = CCBTotal;
	CCBPtr = CCBArray;
	do {
		CCBPtr->ccb_NextPtr = (MyCCB *)(sizeof(MyCCB)-8);	/* Create the next offset */
		CCBPtr->ccb_HDDX = 0;	/* Set the defaults */
		CCBPtr->ccb_HDDY = 0;
		++CCBPtr;
	} while (--i);
	}
	
}

/**********************************

	Read a file from NVRAM or a memory card into buf

**********************************/

static LongWord RamFileSize;

int32 StdReadFile(char *fName,char *buf)
{
	int32 err;			/* Error code to return */
	Item fd;			/* Disk file referance */
	Item req;			/* IO request item */
	IOReq *reqp;		/* Pointer to IO request item */
	IOInfo params;		/* Parameter list for I/O information */
	DeviceStatus ds;	/* Struct for device status */
	
	fd = OpenDiskFile(fName);	/* Open the file */
	if (fd < 0) {				/* Error? */
		return fd;
	}
	
	req = CreateIOReq(NULL,0,fd,0);			/* Create an I/O item */
	reqp = (IOReq *)LookupItem(req);		/* Deref the item pointer */
	memset(&params,0,sizeof(IOInfo));		/* Blank the I/O record */
	memset(&ds,0,sizeof(DeviceStatus));		/* Blank the device status */
	params.ioi_Command = CMD_STATUS;		/* Make a status command */
	params.ioi_Recv.iob_Buffer = &ds;		/* Set the I/O buffer ptr */
	params.ioi_Recv.iob_Len = sizeof(DeviceStatus);	/* Set the length */
	err = DoIO(req,&params);				/* Perform the status I/O */
	if (err>=0) {			/* Status ok? */
		/* Try to read it in */

		/* Calc the read size based on blocks */
		RamFileSize = ds.ds_DeviceBlockCount * ds.ds_DeviceBlockSize;
		memset(&params,0,sizeof(IOInfo));		/* Zap the I/O info record */
		params.ioi_Command = CMD_READ;			/* Read command */
		params.ioi_Recv.iob_Len = RamFileSize;	/* Data length */
		params.ioi_Recv.iob_Buffer = buf;		/* Data buffer */
		err = DoIO(req,&params);				/* Read the file */
	}
	DeleteIOReq(req);		/* Release the IO request */
	CloseDiskFile(fd);		/* Close the disk file */
	return err;				/* Return the error code (If any) */
}

/**********************************

	Write out the prefs to the NVRAM

**********************************/

#define PREFWORD 0x4C57
static char PrefsName[] = "/NVRAM/DoomPrefs";		/* Save game name */

void WritePrefsFile(void)
{
	Word PrefFile[10];		/* Must match what's in ReadPrefsFile!! */
	Word CheckSum;			/* Checksum total */
	Word i;

	PrefFile[0] = PREFWORD;
	PrefFile[1] = StartSkill;
	PrefFile[2] = StartMap;
	PrefFile[3] = SfxVolume;
	PrefFile[4] = MusicVolume;
	PrefFile[5] = ControlType;
	PrefFile[6] = MaxLevel;
	PrefFile[7] = ScreenSize;
	PrefFile[8] = LowDetail;
	PrefFile[9] = 12345;		/* Init the checksum */
	i = 0;
	CheckSum = 0;
	do {
		CheckSum += PrefFile[i];		/* Make a simple checksum */
	} while (++i<10);
	PrefFile[9] = CheckSum;
	SaveAFile((Byte *)PrefsName,&PrefFile,sizeof(PrefFile));	/* Save the game file */
}

/**********************************

	Clear out the prefs file

**********************************/

void ClearPrefsFile(void)
{
	StartSkill = sk_medium;		/* Init the basic skill level */
	StartMap = 1;				/* Only allow playing from map #1 */
	SfxVolume = 15;				/* Init the sound effects volume */
	MusicVolume = 15;			/* Init the music volume */
	ControlType = 3;			/* Use basic joypad controls */
	MaxLevel = 1;				/* Only allow level 1 to select from */
	ScreenSize = 2;				/* Default screen size */
	LowDetail = FALSE;			/* Detail mode */
	WritePrefsFile();			/* Output the new prefs */
}

/**********************************

	Load in the standard prefs

**********************************/

void ReadPrefsFile(void)
{
	Word PrefFile[88];		/* Must match what's in WritePrefsFile!! */
	Word CheckSum;			/* Running checksum */
	Word i;

	if (StdReadFile(PrefsName,(char *) PrefFile)<0) {	/* Error reading? */
		ClearPrefsFile();		/* Clear it out */
		return;
	}

	i = 0;
	CheckSum = 12345;		/* Init the checksum */
	do {
		CheckSum+=PrefFile[i];	/* Calculate the checksum */
	} while (++i<9);

	if ((CheckSum != PrefFile[10-1]) || (PrefFile[0] !=PREFWORD)) {
		ClearPrefsFile();	/* Bad ID or checksum! */
		return;
	}
	StartSkill = (skill_t)PrefFile[1];
	StartMap = PrefFile[2];
	SfxVolume = PrefFile[3];
	MusicVolume = PrefFile[4];
	ControlType = PrefFile[5];
	MaxLevel = PrefFile[6];
	ScreenSize = PrefFile[7];
	LowDetail = PrefFile[8];
	if ((StartSkill >= (sk_nightmare+1)) ||
		(StartMap >= 27) ||
		(SfxVolume >= 16) ||
		(MusicVolume >= 16) ||
		(ControlType >= 6) ||
		(MaxLevel >= 26) ||
		(ScreenSize >= 6) ||
		(LowDetail>=2) ) {
		ClearPrefsFile();
	}
}

/**********************************

	Flush all the cached CCB's

**********************************/

static void FlushCCBs(void)
{
	MyCCB* NewCCB;

	NewCCB = CurrentCCB;
	if (NewCCB!=&CCBArray[0]) {
		--NewCCB;		/* Get the last used CCB */
		NewCCB->ccb_Flags |= CCB_LAST;	/* Mark as the last one */
		DrawCels(VideoItem,(CCB *)&CCBArray[0]);	/* Draw all the cels in one shot */
		CurrentCCB = &CCBArray[0];		/* Reset the empty entry */
	}
	SpanPtr = SpanArray;		/* Reset the floor texture pointer */
}

/**********************************

	Display the current framebuffer
	If < 1/15th second has passed since the last display, busy wait.
	15 fps is the maximum frame rate, and any faster displays will
	only look ragged.

**********************************/

void UpdateAndPageFlip(void)
{
	LongWord NewTick;

	FlushCCBs();
	if (DoWipe) {
		Word PrevPage;
		void *NewImage;
		void *OldImage;
		
		DoWipe = FALSE;
		NewImage = VideoPointer;	/* Pointer to the NEW image */
		PrevPage = WorkPage-1;	/* Get the currently displayed page */
		if (PrevPage==-1) {		/* Wrapped? */
			PrevPage = SCREENS-1;
		}
		SetMyScreen(PrevPage);		/* Set videopointer to display buffer */
		if (!PrevPage) {
			PrevPage=SCREENS;
		}
		--PrevPage;
		OldImage = (Byte *) &ScreenMaps[PrevPage][0];	/* Get work buffer */
			
			/* Copy the buffer from display to work */
		memcpy(OldImage,VideoPointer,320*200*2);		
		WipeDoom((LongWord *)OldImage,(LongWord *)NewImage);			/* Perform the wipe */
	}
	DisplayScreen(ScreenItems[WorkPage],0);		/* Display the hidden page */
	if (++WorkPage>=SCREENS) {		/* Next screen in line */
		WorkPage = 0;
	}
	SetMyScreen(WorkPage);		/* Set the 3DO vars */
	do {
		NewTick = ReadTick();	/* Get the time mark */
		LastTics = NewTick - LastTicCount;	/* Get the time elapsed */
	} while (!LastTics);		/* Hmmm, too fast?!?!? */
	LastTicCount = NewTick;				/* Save the time mark */
}

/**********************************

	Draw a shape centered on the screen
	Used for "Loading or Paused" pics

**********************************/

void DrawPlaque(Word RezNum)
{
	Word PrevPage;
	void *PicPtr;
	
	PrevPage = WorkPage-1;
	if (PrevPage==-1) {
		PrevPage = SCREENS-1;
	}
	FlushCCBs();		/* Flush pending draws */
	SetMyScreen(PrevPage);		/* Draw to the active screen */
	PicPtr = LoadAResource(RezNum);
	DrawShape(160-(GetShapeWidth(PicPtr)/2),80,PicPtr);
	FlushCCBs();		/* Make sure it's drawn */
	ReleaseAResource(RezNum);
	SetMyScreen(WorkPage);		/* Reset to normal */
}

/**********************************

	Main code entry

**********************************/

int main(void)
{
	InitTools();		/* Init the 3DO tool system */
	UpdateAndPageFlip();	/* Init the video display's vars */
	ReadPrefsFile();		/* Load defaults */
	D_DoomMain();		/* Start doom */
	return 0;
}

/**********************************

	Draw a scaled solid line

**********************************/

static void AddCCB(Word x,Word y,MyCCB* NewCCB)
{
	MyCCB* DestCCB;			/* Pointer to new CCB entry */
	LongWord TheFlags;		/* CCB flags */
	LongWord ThePtr;		/* Temp pointer to munge */

	DestCCB = CurrentCCB;		/* Copy pointer to local */
	if (DestCCB>=&CCBArray[CCBTotal]) {		/* Am I full already? */
		FlushCCBs();
		DestCCB = CCBArray;
	}
	TheFlags = NewCCB->ccb_Flags;		/* Preload the CCB flags */
	DestCCB->ccb_XPos = x<<16;		/* Set the x and y coord */
	DestCCB->ccb_YPos = y<<16;
	DestCCB->ccb_HDX = NewCCB->ccb_HDX;	/* Set the data for the CCB */
	DestCCB->ccb_HDY = NewCCB->ccb_HDY;
	DestCCB->ccb_VDX = NewCCB->ccb_VDX;
	DestCCB->ccb_VDY = NewCCB->ccb_VDY;
	DestCCB->ccb_PIXC = NewCCB->ccb_PIXC;
	DestCCB->ccb_PRE0 = NewCCB->ccb_PRE0;
	DestCCB->ccb_PRE1 = NewCCB->ccb_PRE1;

	ThePtr = (LongWord)NewCCB->ccb_SourcePtr;	/* Force absolute address */
	if (!(TheFlags&CCB_SPABS)) {
		ThePtr += ((LongWord)NewCCB)+12;	/* Convert relative to abs */
	}
	DestCCB->ccb_SourcePtr = (CelData *)ThePtr;	/* Save the source ptr */

	if (TheFlags&CCB_LDPLUT) {		/* Only load a new plut if REALLY needed */
		ThePtr = (LongWord)NewCCB->ccb_PLUTPtr;
		if (!(TheFlags&CCB_PPABS)) {		/* Relative plut? */
			ThePtr += ((LongWord)NewCCB)+16;	/* Convert relative to abs */
		}
		DestCCB->ccb_PLUTPtr = (void *)ThePtr;	/* Save the PLUT pointer */
	}
	DestCCB->ccb_Flags = (TheFlags & ~(CCB_LAST|CCB_NPABS)) | (CCB_SPABS|CCB_PPABS);
	++DestCCB;			/* Next CCB */
	CurrentCCB = DestCCB;	/* Save the CCB pointer */
}

/**********************************

	Draw a masked shape on the screen

**********************************/

void DrawMShape(Word x,Word y,void *ShapePtr)
{
	((MyCCB*)ShapePtr)->ccb_Flags &= ~CCB_BGND;	/* Enable masking */
	AddCCB(x,y,(MyCCB*)ShapePtr);		/* Draw the shape */
}

/**********************************

	Draw an unmasked shape on the screen

**********************************/

void DrawShape(Word x,Word y,void *ShapePtr)
{
	((MyCCB*)ShapePtr)->ccb_Flags |= CCB_BGND;	/* Disable masking */
	AddCCB(x,y,(MyCCB*)ShapePtr);		/* Draw the shape */
}

/**********************************

	Draw a solid colored line using the 3DO
	cel engine hardware. This replaces the generic code
	found in AMMain.c.
	My brain hurts.

**********************************/

void DrawLine(Word x1,Word y1,Word x2,Word y2,Word color)
{
	MyCCB* DestCCB;			/* Pointer to new CCB entry */

	DestCCB = CurrentCCB;		/* Copy pointer to local */
	if (DestCCB>=&CCBArray[CCBTotal]) {		/* Am I full already? */
		FlushCCBs();				/* Draw all the CCBs/Lines */
		DestCCB=CCBArray;
	}
	DestCCB->ccb_Flags = CCB_LDSIZE|CCB_LDPRS|
		CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|
		CCB_ACE|CCB_BGND|CCB_NOBLK;	/* ccb_flags */

	DestCCB->ccb_PIXC = 0x1F00;		/* PIXC control */
	DestCCB->ccb_PRE0 = 0x40000016;		/* Preamble */
	DestCCB->ccb_PRE1 = 0x03FF1000;		/* Second preamble */
	DestCCB->ccb_SourcePtr = (CelData *)0;	/* Save the source ptr */
	DestCCB->ccb_PLUTPtr = (void *)(color<<16);		/* Set the color pixel */

	if ((int)x2<(int)x1) {	/* By sorting the x and y's I can only draw lines */
		Word Temp;		/* in two types, x>=y or x<y */
		Temp = x1;		/* Swap the x's */
		x1 = x2;
		x2 = Temp;
		Temp = y1;		/* Swap the y's */
		y1 = y2;
		y2 = Temp;
	}

	y1=-y1;			/* The y's are upside down!! */
	y2=-y2;

	x2=(x2-x1)+1;	/* Get the DELTA value (Always positive) */
	y2=y2-y1;		/* But add 1 for inclusive size depending on sign */
	x1+=160;	/* Move the x coords to the CENTER of the screen */
	y1+=80;		/* Vertically flip and then CENTER the y */

	if (y2&0x8000) {	/* Negative y? */
		y2-=1;			/* Widen by 1 pixel */
		if (x2<(-y2)) {	/* Quadrant 7? */
			x1-=1;
			y1+=1;
			goto Quadrant7;	/* OK */
		}		/* Quadrant 6 */
		goto Quadrant6;		/* OK */
	}
	++y2;
	if (x2<y2) {	/* Quadrant 7? */
Quadrant7:
		DestCCB->ccb_HDX = 1<<20;
		DestCCB->ccb_HDY = 0<<20;
		DestCCB->ccb_VDX = x2<<16;
		DestCCB->ccb_VDY = y2<<16;
	} else {		/* Quadrant 6 */
		--y1;
Quadrant6:
		DestCCB->ccb_VDX = 0<<16;
		DestCCB->ccb_VDY = 1<<16;
		DestCCB->ccb_HDX = x2<<20;
		DestCCB->ccb_HDY = y2<<20;
	}
	
	DestCCB->ccb_XPos = x1<<16;		/* Set the x and y coord for start */
	DestCCB->ccb_YPos = y1<<16;

	++DestCCB;			/* Next CCB */
	CurrentCCB = DestCCB;	/* Save the CCB pointer */
}

/**********************************

	This code is functionally equivalent to the Burgerlib
	version except that it is using the cached CCB system.

**********************************/

void DrawARect(Word x1,Word y1,Word Width,Word Height,Word color)
{
	MyCCB* DestCCB;			/* Pointer to new CCB entry */

	DestCCB = CurrentCCB;		/* Copy pointer to local */
	if (DestCCB>=&CCBArray[CCBTotal]) {		/* Am I full already? */
		FlushCCBs();				/* Draw all the CCBs/Lines */
		DestCCB=CCBArray;
	}
	DestCCB->ccb_Flags = CCB_LDSIZE|CCB_LDPRS|
		CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|
		CCB_ACE|CCB_BGND|CCB_NOBLK;	/* ccb_flags */

	DestCCB->ccb_PIXC = 0x1F00;		/* PIXC control */
	DestCCB->ccb_PRE0 = 0x40000016;		/* Preamble */
	DestCCB->ccb_PRE1 = 0x03FF1000;		/* Second preamble */
	DestCCB->ccb_SourcePtr = (CelData *)0;	/* Save the source ptr */
	DestCCB->ccb_PLUTPtr = (void *)(color<<16);		/* Set the color pixel */
	DestCCB->ccb_XPos = x1<<16;		/* Set the x and y coord for start */
	DestCCB->ccb_YPos = y1<<16;
	DestCCB->ccb_HDX = Width<<20;		/* OK */
	DestCCB->ccb_HDY = 0<<20;
	DestCCB->ccb_VDX = 0<<16;
	DestCCB->ccb_VDY = Height<<16;
	++DestCCB;			/* Next CCB */
	CurrentCCB = DestCCB;	/* Save the CCB pointer */
}

/**********************************

	Ok boys amd girls, the follow functions are drawing primitives
	for Doom 3DO that take into account the fact that I draw all lines from
	back to front. This way I use painter's algorithm to do clipping since the
	3DO cel engine does not allow arbitrary clipping of any kind.
	
	On the first pass, I draw all the sky lines without any regard for clipping
	since no object can be behind the sky.
	
	Next I then sort the sprites from back to front.
	
	Then I draw the walls from back to front, mixing in all the sprites so
	that all shapes will clip properly using painter's algorithm.

**********************************/

/**********************************

	Drawing the sky is the easiest, so I'll do this first.
	The parms are, 
	tx_x = screen x coord for the virtual screen.
	colnum = index for which scan line to draw from the source image. Note that
		this number has all bits set so I must mask off the unneeded bits for
		texture wraparound.
		
	No light shading is used for the sky. The scale factor is a constant.
	
**********************************/

extern Word tx_x;
extern int tx_scale;

void DrawSkyLine(void)
{	
	Byte *Source;
	Word Colnum;
	MyCCB* DestCCB;			/* Pointer to new CCB entry */
	
	DestCCB = CurrentCCB;		/* Copy pointer to local */
	if (DestCCB>=&CCBArray[CCBTotal]) {		/* Am I full already? */
		FlushCCBs();				/* Draw all the CCBs/Lines */
		DestCCB = CCBArray;
	}
	Colnum = (((xtoviewangle[tx_x]+viewangle)>>ANGLETOSKYSHIFT)&0xFF)*64;
	Source = (Byte *)(*SkyTexture->data);	/* Index to the true shape */

	DestCCB[0].ccb_Flags = CCB_SPABS|CCB_LDSIZE|CCB_LDPRS|
	CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|
	CCB_ACE|CCB_BGND|CCB_NOBLK|CCB_PPABS|CCB_LDPLUT;	/* ccb_flags */
	DestCCB[0].ccb_PRE0 = 0x03;
	DestCCB[0].ccb_PRE1 = 0x3E005000|(128-1);	/* Project the pixels */
	DestCCB[0].ccb_PLUTPtr = Source;		/* Get the palette ptr */
	DestCCB[0].ccb_SourcePtr = (CelData *)&Source[Colnum+32];	/* Get the source ptr */
	DestCCB[0].ccb_XPos = tx_x<<16;		/* Set the x and y coord for start */
	DestCCB[0].ccb_YPos = 0<<16;
	DestCCB[0].ccb_HDX = 0<<20;		/* Convert 6 bit frac to CCB scale */
	DestCCB[0].ccb_HDY = SkyScales[ScreenSize];	/* Video stretch factor */
	DestCCB[0].ccb_VDX = 1<<16;
	DestCCB[0].ccb_VDY = 0<<16;
	DestCCB[0].ccb_PIXC = 0x1F00;		/* PIXC control */
	++DestCCB;			/* Next CCB */
	CurrentCCB = DestCCB;	/* Save the CCB pointer */
}

/**********************************

	Drawing the wall columns are a little trickier, so I'll do this next.
	The parms are, 
	tx_x = screen x coord for the virtual screen.
	y = screen y coord for the virtual screen.
	bottom = screen y coord for the BOTTOM of the pixel run. Subtract from top
		to get the exact destination pixel run count.
	colnum = index for which scan line to draw from the source image. Note that
		this number has all bits set so I must mask off the unneeded bits for
		texture wraparound.
		
	No light shading is used. The scale factor is a constant.
	
**********************************/

void DrawWallColumn(Word y,Word Colnum,Byte *Source,Word Run)
{	
	MyCCB* DestCCB;			/* Pointer to new CCB entry */
	Word Colnum7;
	
	DestCCB = CurrentCCB;		/* Copy pointer to local */
	if (DestCCB>=&CCBArray[CCBTotal]) {		/* Am I full already? */
		FlushCCBs();				/* Draw all the CCBs/Lines */
		DestCCB = CCBArray;
	}

	Colnum7 = Colnum & 7;	/* Get the pixel skip */
	Colnum = Colnum>>1;		/* Pixel to byte offset */
	Colnum += 32;			/* Index past the PLUT */
	Colnum &= ~3;			/* Long word align the source */
	DestCCB[0].ccb_Flags = CCB_SPABS|CCB_LDSIZE|CCB_LDPRS|
	CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|
	CCB_ACE|CCB_BGND|CCB_NOBLK|CCB_PPABS|CCB_LDPLUT|CCB_USEAV;	/* ccb_flags */
	DestCCB[0].ccb_PRE0 = (Colnum7<<24)|0x03;
	DestCCB[0].ccb_PRE1 = 0x3E005000|(Colnum7+Run-1);	/* Project the pixels */
	DestCCB[0].ccb_PLUTPtr = Source;		/* Get the palette ptr */
	DestCCB[0].ccb_SourcePtr = (CelData *)&Source[Colnum];	/* Get the source ptr */
	DestCCB[0].ccb_XPos = tx_x<<16;		/* Set the x and y coord for start */
	DestCCB[0].ccb_YPos = (y<<16)+0xFF00;
	DestCCB[0].ccb_HDX = 0<<20;		/* Convert 6 bit frac to CCB scale */
	DestCCB[0].ccb_HDY = (tx_scale<<11);
	DestCCB[0].ccb_VDX = 1<<16;
	DestCCB[0].ccb_VDY = 0<<16;
	DestCCB[0].ccb_PIXC = LightTable[tx_texturelight>>LIGHTSCALESHIFT];		/* PIXC control */
	
	++DestCCB;			/* Next CCB */
	CurrentCCB = DestCCB;	/* Save the CCB pointer */
}

/**********************************

	Drawing the floor and ceiling is the hardest, so I'll do this last.
	The parms are, 
	x = screen x coord for the virtual screen. Must be offset
		to the true screen coords.
	top = screen y coord for the virtual screen. Must also be offset
		to the true screen coords.
	bottom = screen y coord for the BOTTOM of the pixel run. Subtract from top
		to get the exact destination pixel run count.
	colnum = index for which scan line to draw from the source image. Note that
		this number has all bits set so I must mask off the unneeded bits for
		texture wraparound.

	SpanPtr is a pointer to the SpanArray buffer, this is where I store the
	processed floor textures.
	No light shading is used. The scale factor is a constant.
	
**********************************/

void DrawFloorColumn(Word ds_y,Word ds_x1,Word Count,LongWord xfrac,
	LongWord yfrac,Fixed ds_xstep,Fixed ds_ystep)
{
	Byte *DestPtr;
	MyCCB *DestCCB;

	DestCCB = CurrentCCB;		/* Copy pointer to local */
	if (DestCCB>=&CCBArray[CCBTotal]) {		/* Am I full already? */
		FlushCCBs();				/* Draw all the CCBs/Lines */
		DestCCB=CCBArray;
	}
	DestPtr = SpanPtr;
	DrawASpan(Count,xfrac,yfrac,ds_xstep,ds_ystep,DestPtr);

	DestCCB->ccb_Flags = CCB_SPABS|CCB_LDSIZE|CCB_LDPRS|
	CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|
	CCB_ACE|CCB_BGND|CCB_NOBLK|CCB_PPABS|CCB_LDPLUT|CCB_USEAV;	/* ccb_flags */

	DestCCB->ccb_PRE0 = 0x00000005;		/* Preamble (Coded 8 bit) */
	DestCCB->ccb_PRE1 = 0x3E005000|(Count-1);		/* Second preamble */
	DestCCB->ccb_SourcePtr = (CelData *)DestPtr;	/* Save the source ptr */
	DestCCB->ccb_PLUTPtr = PlaneSource;		/* Get the palette ptr */
	DestCCB->ccb_XPos = ds_x1<<16;		/* Set the x and y coord for start */
	DestCCB->ccb_YPos = ds_y<<16;
	DestCCB->ccb_HDX = 1<<20;		/* OK */
	DestCCB->ccb_HDY = 0<<20;
	DestCCB->ccb_VDX = 0<<16;
	DestCCB->ccb_VDY = 1<<16;	
	DestCCB->ccb_PIXC = LightTable[tx_texturelight>>LIGHTSCALESHIFT];			/* PIXC control */
	
	Count = (Count+3)&(~3);		/* Round to nearest longword */
	DestPtr += Count;
	SpanPtr = DestPtr;
	++DestCCB;			/* Next CCB */
	CurrentCCB = DestCCB;	/* Save the CCB pointer */
}


/**********************************

	Perform a "Doom" like screen wipe
	I require that VideoPointer is set to the current screen

**********************************/

#define WIPEWIDTH 320		/* Width of the 3DO screen to wipe */
#define WIPEHEIGHT 200

static void WipeDoom(LongWord *OldScreen,LongWord *NewScreen)
{
	LongWord Mark;	/* Last time mark */
	Word TimeCount;	/* Elapsed time since last mark */
	Word i,x;
	Word Quit;		/* Finish flag */
	int delta;		/* YDelta (Must be INT!) */
	LongWord *Screenad;		/* I use short pointers so I can */
	LongWord *SourcePtr;		/* move in pixel pairs... */
	int YDeltaTable[WIPEWIDTH/2];	/* Array of deltas for the jagged look */

/* First thing I do is to create a ydelta table to */
/* allow the jagged look to the screen wipe */

	delta = -GetRandom(15);	/* Get the initial position */
	YDeltaTable[0] = delta;	/* Save it */
	x = 1;
	do {
		delta += (GetRandom(2)-1);	/* Add -1,0 or 1 */
		if (delta>0) {		/* Too high? */
			delta = 0;
		}
		if (delta == -16) {	/* Too low? */
			delta = -15;
		}
		YDeltaTable[x] = delta;	/* Save the delta in table */
	} while (++x<(WIPEWIDTH/2));	/* Quit? */

/* Now perform the wipe using ReadTick to generate a time base */
/* Do NOT go faster than 30 frames per second */

	Mark = ReadTick()-2;	/* Get the initial time mark */
	do {
		do {
			TimeCount = ReadTick()-Mark;	/* Get the time mark */
		} while (TimeCount<(TICKSPERSEC/30));			/* Enough time yet? */
		Mark+=TimeCount;		/* Adjust the base mark */
		TimeCount/=(TICKSPERSEC/30);	/* Math is for 30 frames per second */

/* Adjust the YDeltaTable "TimeCount" times to adjust for slow machines */

		Quit = TRUE;		/* Assume I already am finished */
		do {
			x = 0;		/* Start at the left */
			do {
				delta = YDeltaTable[x];		/* Get the delta */
				if (delta<WIPEHEIGHT) {	/* Line finished? */
					Quit = FALSE;		/* I changed one! */
					if (delta < 0) {
						++delta;		/* Slight delay */
					} else if (delta < 16) {
						delta = delta<<1;	/* Double it */
						++delta;
					} else {
						delta+=8;		/* Constant speed */
 						if (delta>WIPEHEIGHT) {
							delta=WIPEHEIGHT;
						}
					}
					YDeltaTable[x] = delta;	/* Save new delta */
				}
			} while (++x<(WIPEWIDTH/2));
		} while (--TimeCount);		/* All tics accounted for? */

/* Draw a frame of the wipe */

		x = 0;			/* Init the x coord */
		do {
			Screenad = (LongWord *)&VideoPointer[x*8];	/* Dest pointer */
			i = YDeltaTable[x];		/* Get offset */
			if ((int)i<0) {	/* Less than zero? */
				i = 0;		/* Make it zero */
			}
			i>>=1;		/* Force even for 3DO weirdness */
			if (i) {
				TimeCount = i;
				SourcePtr = &NewScreen[x*2];	/* Fetch from source */
				do {
					Screenad[0] = SourcePtr[0];	/* Copy 2 longwords */
					Screenad[1] = SourcePtr[1];
					Screenad+=WIPEWIDTH;
					SourcePtr+=WIPEWIDTH;
				} while (--TimeCount);
			}
			if (i<(WIPEHEIGHT/2)) {		/* Any of the old image to draw? */
				i = (WIPEHEIGHT/2)-i;
				SourcePtr = &OldScreen[x*2];
				do {
					Screenad[0] = SourcePtr[0];	/* Copy 2 longwords */
					Screenad[1] = SourcePtr[1];
					Screenad+=WIPEWIDTH;
					SourcePtr+=WIPEWIDTH;
				} while (--i);
			}
		} while (++x<(WIPEWIDTH/2));
	} while (!Quit);		/* Wipe finished? */
}

/**********************************

	This routine will draw a scaled sprite during the game.
	It is called when there is no onscreen clipping needed or 
	if the only clipping is to the screen bounds.

**********************************/

void DrawSpriteNoClip(vissprite_t *vis)
{
	patch_t	*patch;		/* Pointer to the actual sprite record */
	Word ColorMap;
	int x;
	
	patch = (patch_t *)LoadAResource(vis->PatchLump);	
	patch =(patch_t *) &((Byte *)patch)[vis->PatchOffset];

	((LongWord *)patch)[7] = 0;
	((LongWord *)patch)[10] = 0;
	((LongWord *)patch)[8] = vis->yscale<<4;
	ColorMap = vis->colormap;
	if (ColorMap&0x8000) {
		((LongWord *)patch)[13] = 0x9C81;
	} else {
		((LongWord *)patch)[13] = LightTable[(ColorMap&0xFF)>>LIGHTSCALESHIFT];
	}
	if (ColorMap&0x4000) {
		x = vis->x2;
		((LongWord *)patch)[9] = -vis->xscale;
	} else {
		x = vis->x1;
		((LongWord *)patch)[9] = vis->xscale;
	}
	DrawMShape(x+ScreenXOffset,vis->y1+ScreenYOffset,&patch->Data);
	ReleaseAResource(vis->PatchLump);
}

/**********************************

	This routine will draw a scaled sprite during the game.
	It is called when there is onscreen clipping needed so I
	use the global table spropening to get the top and bottom clip
	bounds.
	
	I am passed the screen clipped x1 and x2 coords.

**********************************/

static Byte *SpritePLUT;
static Word SpriteY;
static Word SpriteYScale;
static Word SpritePIXC;
static Word SpritePRE0;
static Word SpritePRE1;
static Byte *StartLinePtr;
static Word SpriteWidth;

static void OneSpriteLine(Word x1,Byte *SpriteLinePtr)
{
	MyCCB *DestCCB;

	DestCCB = CurrentCCB;		/* Copy pointer to local */
	if (DestCCB>=&CCBArray[CCBTotal]) {		/* Am I full already? */
		FlushCCBs();				/* Draw all the CCBs/Lines */
		DestCCB=CCBArray;
	}
	DestCCB->ccb_Flags = CCB_SPABS|CCB_LDSIZE|CCB_LDPRS|CCB_PACKED|
	CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|
	CCB_ACE|CCB_BGND|CCB_NOBLK|CCB_PPABS|CCB_LDPLUT;	/* ccb_flags */

	DestCCB->ccb_PIXC = SpritePIXC;			/* PIXC control */
	DestCCB->ccb_PRE0 = SpritePRE0;		/* Preamble (Coded 8 bit) */
	DestCCB->ccb_PRE1 = SpritePRE1;		/* Second preamble */
	DestCCB->ccb_SourcePtr = (CelData *)SpriteLinePtr;	/* Save the source ptr */
	DestCCB->ccb_PLUTPtr = SpritePLUT;		/* Get the palette ptr */
	DestCCB->ccb_XPos = (x1+ScreenXOffset)<<16;		/* Set the x and y coord for start */
	DestCCB->ccb_YPos = SpriteY;
	DestCCB->ccb_HDX = 0<<20;		/* OK */
	DestCCB->ccb_HDY = SpriteYScale;
	DestCCB->ccb_VDX = 1<<16;
	DestCCB->ccb_VDY = 0<<16;
	++DestCCB;			/* Next CCB */
	CurrentCCB = DestCCB;	/* Save the CCB pointer */
}

static void OneSpriteClipLine(Word x1,Byte *SpriteLinePtr,int Clip,int Run)
{
	MyCCB *DestCCB;

	DrawARect(0,191,Run,1,BLACK);
	DestCCB = CurrentCCB;		/* Copy pointer to local */
	if (DestCCB>=&CCBArray[CCBTotal-1]) {		/* Am I full already? */
		FlushCCBs();				/* Draw all the CCBs/Lines */
		DestCCB=CCBArray;
	}
	DestCCB->ccb_Flags = CCB_SPABS|CCB_LDSIZE|CCB_LDPRS|CCB_PACKED|
	CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|
	CCB_ACE|CCB_BGND|CCB_PPABS|CCB_LDPLUT;	/* ccb_flags */

	DestCCB->ccb_PIXC = 0x1F00;			/* PIXC control */
	DestCCB->ccb_PRE0 = SpritePRE0;		/* Preamble (Coded 8 bit) */
	DestCCB->ccb_PRE1 = SpritePRE1;		/* Second preamble */
	DestCCB->ccb_SourcePtr = (CelData *)SpriteLinePtr;	/* Save the source ptr */
	DestCCB->ccb_PLUTPtr = SpritePLUT;		/* Get the palette ptr */
	DestCCB->ccb_XPos = -(Clip<<16);		/* Set the x and y coord for start */
	DestCCB->ccb_YPos = 191<<16;
	DestCCB->ccb_HDX = SpriteYScale;		/* OK */
	DestCCB->ccb_HDY = 0<<20;
	DestCCB->ccb_VDX = 0<<16;
	DestCCB->ccb_VDY = 1<<16;
	++DestCCB;			/* Next CCB */
	
	DestCCB->ccb_Flags = CCB_SPABS|CCB_LDSIZE|CCB_LDPRS|
	CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|
	CCB_ACE|CCB_NOBLK|CCB_PPABS;	/* ccb_flags */

	DestCCB->ccb_PIXC = SpritePIXC;			/* PIXC control */
	DestCCB->ccb_PRE0 = 0x00000016;		/* Preamble (Uncoded 16) */
	DestCCB->ccb_PRE1 = 0x9E001800+(Run-1);		/* Second preamble */
	DestCCB->ccb_SourcePtr = (CelData *)CelLine190;	/* Save the source ptr */
	DestCCB->ccb_XPos = (x1+ScreenXOffset)<<16;		/* Set the x and y coord for start */
	DestCCB->ccb_YPos = SpriteY+(Clip<<16);
	DestCCB->ccb_HDX = 0<<20;		/* OK */
	DestCCB->ccb_HDY = 1<<20;
	DestCCB->ccb_VDX = 1<<15;		/* Need 15 to fix the LFORM bug */
	DestCCB->ccb_VDY = 0<<16;
	++DestCCB;			/* Next CCB */
	
	CurrentCCB = DestCCB;	/* Save the CCB pointer */
}

static Byte *CalcLine(Fixed XFrac)
{
	Byte *DataPtr;
	
	DataPtr = StartLinePtr;
	XFrac>>=FRACBITS;
	if (XFrac<=0) {		/* Left clip failsafe */
		return DataPtr;
	}
	if (XFrac>=SpriteWidth) {	/* Clipping failsafe */
		XFrac=SpriteWidth-1;
	}
	do {
		Word Offset;
		Offset = DataPtr[0]+2;
		DataPtr = &DataPtr[Offset*4];
	} while (--XFrac);
	return DataPtr;
}

void DrawSpriteClip(Word x1,Word x2,vissprite_t *vis)
{
	Word y,MaxRun;
	Word y2;
	Word top,bottom;
	patch_t *patch;
	Fixed XStep,XFrac;
	
	patch = (patch_t *)LoadAResource(vis->PatchLump);	/* Get shape data */
	patch =(patch_t *) &((Byte *)patch)[vis->PatchOffset];	/* Get true pointer */
	SpriteYScale = vis->yscale<<4;		/* Get scale Y factor */
	SpritePLUT = &((Byte *)patch)[64];	/* Get pointer to PLUT */
	SpritePRE0 = ((Word *)patch)[14]&~(0xFF<<6);	/* Only 1 row allowed! */
	SpritePRE1 = ((Word *)patch)[15];		/* Get the proper height */
	y = ((Word *)patch)[3];		/* Get offset to the sprite shape data */
	StartLinePtr = &((Byte *)patch)[y+16];	/* Get pointer to first line of data */
	SpriteWidth = GetShapeHeight(&((Word *)patch)[1]);
	SpritePIXC = (vis->colormap&0x8000) ? 0x9C81 : LightTable[(vis->colormap&0xFF)>>LIGHTSCALESHIFT];
	y = vis->y1;
	SpriteY = (y+ScreenYOffset)<<16;	/* Unmolested Y coord */
	y2 = vis->y2;
	MaxRun = y2-y;
	
	if ((int)y<0) {
		y = 0;
	}
	if ((int)y2>=(int)ScreenHeight) {
		y2 = ScreenHeight;
	}
	XFrac = 0;
	XStep = 0xFFFFFFFFUL/(LongWord)vis->xscale;	/* Get the recipocal for the X scale */
	if (vis->colormap&0x4000) {
		XStep = -XStep;		/* Step in the opposite direction */
		XFrac = (SpriteWidth<<FRACBITS)-1;
	}
	if (vis->x1!=x1) {		/* How far should I skip? */
		XFrac += XStep*(x1-vis->x1);
	}
	do {
		top = spropening[x1];		/* Get the opening to the screen */
		if (top==ScreenHeight) {		/* Not clipped? */
			OneSpriteLine(x1,CalcLine(XFrac));
		} else {
			bottom = top&0xff;
			top >>=8;
			if (top<bottom) {		/* Valid run? */
				if (y>=top && y2<bottom) {
					OneSpriteLine(x1,CalcLine(XFrac));
				} else {
					int Run;
					int Clip;
					
					Clip = top-vis->y1;		/* Number of pixels to clip */
					Run = bottom-top;		/* Allowable run */
					if (Clip<0) {		/* Overrun? */
						Run += Clip;	/* Remove from run */
						Clip = 0;
					}
					if (Run>0) {		/* Still visible? */
						if (Run>MaxRun) {		/* Too big? */
							Run = MaxRun;		/* Force largest... */
						}
						OneSpriteClipLine(x1,CalcLine(XFrac),Clip,Run);
					}
				}
			}
		}
		XFrac+=XStep;
	} while (++x1<=x2);
}

/**********************************

	Draw a sprite in the center of the screen.
	This is used for the finale.
	(Speed is NOT an issue here...)

**********************************/

void DrawSpriteCenter(Word SpriteNum)
{
	Word x,y;
	patch_t *patch;
	LongWord Offset;

	patch = (patch_t *)LoadAResource(SpriteNum>>FF_SPRITESHIFT);	/* Get the sprite group */
	Offset = ((LongWord *)patch)[SpriteNum & FF_FRAMEMASK];
	if (Offset&PT_NOROTATE) {		/* Do I rotate? */
		patch = (patch_t *) &((Byte *)patch)[Offset & 0x3FFFFFFF];		/* Get pointer to rotation list */
		Offset = ((LongWord *)patch)[0];		/* Use the rotated offset */
	}
	patch = (patch_t *)&((Byte *)patch)[Offset & 0x3FFFFFFF];	/* Get pointer to patch */
	
	x = patch->leftoffset;		/* Get the x and y offsets */
	y = patch->topoffset;
	x = 80-x;			/* Center on the screen */
	y = 90-y;
	((LongWord *)patch)[7] = 0;		/* Compensate for sideways scaling */
	((LongWord *)patch)[10] = 0;
	if (Offset&PT_FLIP) {
		((LongWord *)patch)[8] = -0x2<<20;	/* Reverse horizontal */
		x+=GetShapeHeight(&patch->Data);	/* Adjust the x coord */
	} else {
		((LongWord *)patch)[8] = 0x2<<20;	/* Normal horizontal */
	}
	((LongWord *)patch)[9] = 0x2<<16;		/* Double vertical */
	DrawMShape(x*2,y*2,&patch->Data);		/* Scale the x and y and draw */
	ReleaseAResource(SpriteNum>>FF_SPRITESHIFT);	/* Let go of the resource */
}

/**********************************

	Called when memory is REALLY low!
	This is an OOMQueue callback

**********************************/

static void LowMemCode(Word Stage)
{
	FlushCCBs();		/* Purge all CCB's */
}

/**********************************

	Set the hardware clip rect to the actual game screen
	
**********************************/

void EnableHardwareClipping(void)
{
	FlushCCBs();		/* Failsafe */
	SetClipWidth(VideoItem,ScreenWidth);
	SetClipHeight(VideoItem,ScreenHeight);		/* I only want 200 lines */
	SetClipOrigin(VideoItem,ScreenXOffset,ScreenYOffset);		/* Set the clip top for the screen */
}

/**********************************

	Restore the clip rect to normal
	
**********************************/

void DisableHardwareClipping(void)
{
	FlushCCBs();		/* Failsafe */
	SetClipOrigin(VideoItem,0,0);		/* Set the clip top for the screen */
	SetClipWidth(VideoItem,320);
	SetClipHeight(VideoItem,200);		/* I only want 200 lines */
}


/**********************************

	Draw a screen color overlay if needed
	
**********************************/

void DrawColors(void)
{
	MyCCB* DestCCB;			/* Pointer to new CCB entry */
	player_t *player;
	Word ccb,color;
	Word red,green,blue;
	
	player = &players;
	if (player->powers[pw_invulnerability] > 240		/* Full strength */
		|| (player->powers[pw_invulnerability]&16) ) {	/* Flashing... */
		color = 0x7FFF<<16;
		ccb = CCB_LDSIZE|CCB_LDPRS|CCB_PXOR|
		CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|
		CCB_ACE|CCB_BGND|CCB_NOBLK;
		goto DrawIt;
	}
	
	red = player->damagecount;		/* Get damage inflicted */
	green = player->bonuscount>>2;
	red += green;
	blue = 0;
	
	if (player->powers[pw_ironfeet] > 240		/* Radiation suit? */
		|| (player->powers[pw_ironfeet]&16) ) {	/* Flashing... */
		green = 10;			/* Add some green */
	}

	if (player->powers[pw_strength]			/* Berserker pack? */
		&& (player->powers[pw_strength]< 255) ) {
		color = 255-player->powers[pw_strength];
		color >>= 4;
		red+=color;		/* Feeling good! */
	}

	if (red>=32) {
		red = 31;
	}
	if (green>=32) {
		green =31;
	}
	if (blue>=32) {
		blue = 31;
	}

	color = (red<<10)|(green<<5)|blue;
	
	if (!color) {
		return;
	}
	color <<=16;
	ccb = CCB_LDSIZE|CCB_LDPRS|
		CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|
		CCB_ACE|CCB_BGND|CCB_NOBLK;

DrawIt:
	DestCCB = CurrentCCB;		/* Copy pointer to local */
	if (DestCCB>=&CCBArray[CCBTotal]) {		/* Am I full already? */
		FlushCCBs();				/* Draw all the CCBs/Lines */
		DestCCB=CCBArray;
	}
	
	DestCCB->ccb_Flags =ccb;	/* ccb_flags */
	DestCCB->ccb_PIXC = 0x1F80;		/* PIXC control */
	DestCCB->ccb_PRE0 = 0x40000016;		/* Preamble */
	DestCCB->ccb_PRE1 = 0x03FF1000;		/* Second preamble */
	DestCCB->ccb_SourcePtr = (CelData *)0;	/* Save the source ptr */
	DestCCB->ccb_PLUTPtr = (void *)color;		/* Set the color pixel */
	DestCCB->ccb_XPos = ScreenXOffset<<16;		/* Set the x and y coord for start */
	DestCCB->ccb_YPos = ScreenYOffset<<16;
	DestCCB->ccb_HDX = ScreenWidth<<20;		/* OK */
	DestCCB->ccb_HDY = 0<<20;
	DestCCB->ccb_VDX = 0<<16;
	DestCCB->ccb_VDY = ScreenHeight<<16;
	++DestCCB;			/* Next CCB */
	CurrentCCB = DestCCB;	/* Save the CCB pointer */
}

#if 0
/**********************************

	This will allow screen shots to be taken.
	REMOVE FOR FINAL BUILD!!!

**********************************/

Word LastJoyButtons[4];		/* Save the previous joypad bits */
static Word FileNum;
static Short OneLine[640];

Word ReadJoyButtons(Word PadNum)
{
	char FileName[20];
	ControlPadEventData ControlRec;
	Short *OldImage;
	Short *DestImage;

	GetControlPad(PadNum+1,FALSE,&ControlRec);		/* Read joypad */
	if (PadNum<4) {
		if (((ControlRec.cped_ButtonBits^LastJoyButtons[PadNum]) &
			ControlRec.cped_ButtonBits)&PadC) {
			Word i,j,PrevPage;
			
			sprintf(FileName,"Art%d.RAW16",FileNum);
			++FileNum;
			PrevPage = WorkPage-1;	/* Get the currently displayed page */
			if (PrevPage==-1) {		/* Wrapped? */
				PrevPage = SCREENS-1;
			}
			OldImage = (Short *) &ScreenMaps[PrevPage][0];	/* Get work buffer */
			i = 0;
			DestImage = OldImage;
			do {
				memcpy(OneLine,DestImage,320*2*2);
				j = 0;
				do {
					DestImage[j] = OneLine[j*2];
					DestImage[j+320] = OneLine[(j*2)+1];
				} while (++j<320);
				DestImage+=640;
			} while (++i<100);
			WriteMacFile(FileName,OldImage,320*200*2);
		}
		LastJoyButtons[PadNum] = (Word)ControlRec.cped_ButtonBits;
	}
	return (Word)ControlRec.cped_ButtonBits;		/* Return the data */
}
#endif
