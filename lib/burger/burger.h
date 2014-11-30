/********************************

	Burger Bill's universal library
	3DO version
	Also available for Mac, Apple IIgs, IBM PC

********************************/

#ifndef __BURGER__
#define __BURGER__

#ifndef __TYPES_H
#include <Types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int Word;
typedef unsigned char Byte;
typedef unsigned long LongWord;
typedef unsigned short Short;
typedef long Frac;
typedef long Fixed;
typedef double extended;
/*typedef unsigned char Boolean; */

typedef struct {
	Word top,left,bottom,right;
} Rect;

#define __3DO__
#define __BIGENDIAN__
#define __MOUSE__
#define __JOYPAD__
#define TICKSPERSEC 60

#define LoadIntelLong(x) SwapULong(x)
#define LoadMotoLong(x) x
#define LoadIntelShort(x) SwapShort(x)
#define LoadMotoShort(x) x
#define LoadIntelUShort(x) SwapUShort(x)
#define LoadMotoUShort(x) x

#define BLACK 0x0000
#define DARKGREY 0x39CE
#define BROWN 0x4102
#define PURPLE 0x3898
#define BLUE 0x001E
#define DARKGREEN 0x0200
#define ORANGE 0x79C0
#define RED 0x6800
#define BEIGE 0x7A92
#define YELLOW 0x7BC0
#define GREEN 0x0380
#define LIGHTBLUE 0x235E
#define LILAC 0x6A9E
#define PERIWINKLE 0x3A1E
#define LIGHTGREY 0x6318
#define WHITE 0x7FFF
#define SfxActive 1
#define MusicActive 2
#define PauseActive 4

#define SetAuxType(x,y)
#define SetFileType(x,y)

/* In Graphics */

extern Word YTable[240];
extern Byte *VideoPointer;
extern Word VideoWidth;
extern Word ScreenWidth;
extern Word ScreenHeight;
extern void InitYTable(void);
extern Word GetAPixel(Word x,Word y);
extern void SetAPixel(Word x,Word y,Word Color);
extern void DrawShape(Word x,Word y,void *ShapePtr);
extern void DrawMShape(Word x,Word y,void *ShapePtr);
extern void EraseShape(Word x,Word y,void *ShapePtr);
extern void EraseMBShape(Word x, Word y,void *ShapePtr,void *BackPtr);
extern void DrawARect(Word x,Word y,Word Width,Word Height,Word Color);
extern Word GetShapeWidth(void *ShapePtr);
extern Word GetShapeHeight(void *ShapePtr);
extern void *GetShapeIndexPtr(void *ShapeArrayPtr,Word Index);
extern void DrawXMShape(Word x,Word y,void *ShapePtr);
extern Word TestMShape(Word x,Word y,void *ShapePtr);
extern Word TestMBShape(Word x,Word y,void *ShapePtr,void *BackPtr);
extern void ClearTheScreen(Word Color);
extern void DrawRezShape(Word x,Word y,Word RezNum);
extern void DrawRezCenterShape(Word RezNum);

/* In Font */

typedef struct {
	Word FontX;
	Word FontY;
	Word FontLoaded;
	Word FontInvisible;
	Word FontOrMask[16];
} FontState_t;

extern Word FontX;
extern Word FontY;
extern void *FontPointer;
extern Byte *FontWidths;
extern Word FontHeight;
extern Word FontFirst;
extern Word FontLast;
extern Word FontLoaded;
extern Word FontInvisible;
extern Word FontOrMask[16];
extern void FontSaveState(FontState_t *StatePtr);
extern void FontRestoreState(FontState_t *StatePtr);
extern void DrawAString(void *TextPtr);
extern Word WidthAString(void *TextPtr);
#define SetFontXY(x,y) FontSetXY(x,y)
extern void FontSetXY(Word x,Word y);
extern void DrawAChar(Word Char);
extern void InstallAFont(Word FontNum);
extern void FontUseZero(void);
extern void FontUseMask(void);
extern void FontSetColor(Word ColorNum,Word Color);

/* In Palette */

#define FadeToBlack() FadeToBlackX()		/* The 3DO library has a call like this */
extern Byte CurrentPalette[32*4];
extern void SetAPalette(Word ResID);
extern void SetAPalettePtr(void *PalettePtr);
extern void SetAPalettePtr2(Word Start,Word Count,void *PalettePtr);
extern void FadeTo(Word ResID);
extern void FadeToPtr(void *PalettePtr);
extern void FadeToBlackX(void);
#define SetBorderColor(Color)		/* No borders on 3DO */
#define GetBorderColor() 0

/* Input handlers */

#define PadDown			0x80000000
#define PadUp			0x40000000
#define PadRight		0x20000000
#define PadLeft			0x10000000
#define PadA			0x08000000
#define PadB			0x04000000
#define PadC			0x02000000
#define PadD			0x00000000
#define PadStart		0x01000000
#define PadX			0x00800000
#define PadRightShift	0x00400000
#define PadLeftShift	0x00200000
#define PadXLeft		0x00100000
#define PadXRight		0x00080000

#define RatLeft			0x80000000
#define RatMiddle		0x40000000
#define RatRight		0x20000000

extern Word LastJoyButtons[4];		/* Save the previous joypad bits */
extern Word LastMouseButton;		/* Save the previous mouse hit */

extern void (*MyKbhitCallBack)(void);
extern Word (*MyGetchCallBack)(Word);
extern void InterceptKey(void);
extern volatile Byte KeyArray[128];
extern Word KeyModifiers;
extern Word MyGetch(void);
extern Word MyKbhit(void);
extern void FlushKeys(void);
extern Word GetAKey(void);
extern Word WaitAKey(void);
extern Word ReadJoyButtons(Word Which);
extern Word ReadMouseButtons(void);
extern void ReadMouseAbs(Word *x,Word *y);
extern void ReadMouseDelta(int *x,int *y);
extern Boolean DetectMouse(void);
extern Boolean MousePresent;

/* Memory Handlers */

#define MMStageCompact 0
#define MMStagePurge 1

extern LongWord MaxMemSize;			/* Maximum memory the program will take */
extern LongWord MinReservedSize;	/* Minimum memory for OS */
extern Word MinHandles;		/* Number of handles to create */
extern void (*MemPurgeCallBack)(Word Stage);	/* Callback before memory purging */
extern void InitMemory(void);
extern void **AllocAHandle(LongWord MemSize);
extern void **AllocAHandle2(LongWord MemSize,Word Flag);
extern void DeallocAHandle(void **MemHandle);
extern void *AllocAPointer(LongWord MemSize);
extern void DeallocAPointer(void *MemPtr);
extern void **ReallocAHandle(void **MemHandle);
extern void **FindAHandle(void *MemPtr);
extern void *LockAHandle(void **MemHandle);
extern void UnlockAHandle(void **MemHandle);
extern void CompactHandles(void);
extern void PurgeHandles(Word Priority);
extern LongWord GetAHandleSize(void **MemHandle);
extern LongWord GetTotalFreeMem(void);
extern void SetHandlePurgeFlag(void **MemHandle,Word Flag);
extern void DumpHandles(void);

/* Resource Handlers */

extern void InitResource(void);
extern Word OpenAResourceFile(Byte *FileName);
extern void CloseAResourceFile(Word RefNum);
extern void *LoadAResource(Word ResID);
extern void *LoadAResource2(Word ResID,Word Type);
extern void **LoadAResourceHandle(Word ResID);
extern void **LoadAResourceHandle2(Word ResID,Word Type);
extern void ReleaseAResource(Word ResID);
extern void ReleaseAResource2(Word ResID,Word Type);
extern void KillAResource(Word ResID);
extern void KillAResource2(Word ResID,Word Type);
extern void DetachAResource(Word ResID);
extern void DetachAResource2(Word ResID,Word Type);

/* Misc routines */

extern void Randomize(void);
extern Word GetRandom(Word MaxVal);
extern Short SwapUShort(Short Val);
extern short SwapShort(short Val);
extern long SwapLong(long Val);
extern LongWord SwapULong(LongWord Val);
extern void Fatal(char *FatalMsg);
extern void NonFatal(char *ErrorMsg);
extern void SetErrBombFlag(Boolean Flag);
extern char *ErrorMsg;
extern void LongWordToAscii(LongWord Input,Byte *AsciiPtr);
extern void longToAscii(long Input,Byte *AsciiPtr);
extern Word SaveAFile(Byte *FileName,void *data,LongWord Length);
extern void *LoadAFile(Byte *FileName);

/* Time and Events */

extern LongWord LastTick;
extern void WaitTick(void);
extern void WaitTicks(Word TickCount);
extern Word WaitTicksEvent(Word TickCount);
extern Word WaitEvent(void);
extern LongWord ReadTick(void);

/* Sound effects */

#define VOICECOUNT 4		/* Number of voices for sound effects */
extern Word SampleSound[VOICECOUNT];	/* Sound number being playing in this channel */
extern Word SamplePriority[VOICECOUNT];	/* Priority chain */
extern Item AllSamples[];		/* Array of sound effect items to play */
extern Word AllRates[];

extern Word MusicVolume;			/* Volume for music */
extern Word SfxVolume;		/* Volume for SFX */
extern Word LeftVolume;
extern Word RightVolume;

extern void InitMusicPlayer(char *MusicDSP);
extern void InitSoundPlayer(char *SoundDSP,Word Rate);
extern void LockMusic(void);
extern void UnlockMusic(void);
extern void PauseSound(void);
extern void ResumeSound(void);
extern void PauseMusic(void);
extern void ResumeMusic(void);
extern void StopSound(Word SoundNum);
extern void PlaySound(Word SoundNum);
extern void PlaySong(Word SongNum);		/* Play a song */
extern void SetSfxVolume(Word NewVolume);
extern void SetMusicVolume(Word NewVolume);
extern Word KilledSong;

/* Misc funcs */
extern void Halt(void);
extern void ShowMCGA(Byte *VideoPtr,Byte *SourcePtr,Word *ColorPtr);
extern void DLZSS(unsigned char *Dest,unsigned char *Src, unsigned long Length);
extern Word SystemState;

extern void *AllocSomeMem(LongWord Size);
extern void FreeSomeMem(void *MemPtr);

/* 3DO Specific Convience routines */

typedef struct {
	LongWord Offset;	/* Offset into the file */
	LongWord Length;	/* Length of the data */
	void **MemPtr;		/* Entry in memory */
} MyRezEntry2;

typedef struct {
	LongWord Type;		/* Data type */
	LongWord RezNum;	/* Resource number */
	LongWord Count;		/* Number of entries */
	MyRezEntry2 Array[1];	/* First entry */
} MyRezEntry;			/* Entry in memory */

typedef struct {
	Byte Name[4];		/* BRGR Signature */
	LongWord Count;		/* Number of resource entries */
	LongWord MemSize;	/* Bytes the header will occupy */
} MyRezHeader;

#define HANDLELOCK 0x80		/* Lock flags */
#define HANDLEFIXED 0x40	/* Fixed memory flag */
#define HANDLEPURGEBITS 0x01	/* Allow purge flag */

typedef struct MyHandle {
	void *MemPtr;			/* Pointer to true memory */
	LongWord Length;		/* Length of memory */
	LongWord Flags;			/* Memory flags */
	struct MyHandle *NextHandle;	/* Next handle in the chain */
	struct MyHandle *PrevHandle;
} MyHandle;

extern char RezFileName[];	/* Default resource filename "REZFILE" */
extern MyRezEntry2 *ScanRezMap(Word RezNum,Word Type);
extern Item VideoItem;			/* 3DO Specific! */
extern Item VideoScreen;		/* 3DO Specific! */

/**********************************

    3DO Hacks

**********************************/

extern int abs(int Val);		/* 3DO doesn't have these ANSI library routines */
extern int atexit(void (*func)(void));
extern void exit2(int errcode);
extern void Show3DOLogo(void);
#define exit(x) exit2(x)		/* Make SURE that Burger.h is first!! */

#ifdef __cplusplus
};
#endif

#endif
