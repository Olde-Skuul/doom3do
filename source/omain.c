#include "Doom.h"
#include <String.h>

#define CURSORX 45		/* X coord for skulls */
#define SLIDERX 106		/* X coord for slider bars */
#define SLIDESTEP 6		/* Adjustment for volume to screen coord */
#define JOYPADX 90		/* X coord for joypad text */

#define SFXVOLY 60		/* Y coord for SFX volume control */
#define MUSICVOLY 120	/* Y coord for Music volume control */
#define JOYPADY 40		/* Y coord for joypad control */
#define SIZEY 140		/* Y coord for screen size control */

/* Action buttons can be set to PadA, PadB, or PadC */

enum {		/* Control option count */
	SFU,SUF,FSU,FUS,USF,UFS,NUMCONTROLOPTIONS
};

enum {		/* Menu items to select from */
	soundvol,		/* Volume */
	musicvol,		/* Music volume */
	controls,		/* Control settings */
	size,			/* Screen size settings */
	NUMMENUITEMS
};

enum {
	BAR,		/* Sub shapes for scroll bar */
	HANDLE
};

static Word cursorframe;		/* Skull animation frame */
static Word cursorcount;		/* Time mark to animate the skull */
static Word cursorpos;			/* Y position of the skull */
static Word movecount;			/* Time mark to move the skull */
static Word CursorYs[NUMMENUITEMS] = {SFXVOLY-2,MUSICVOLY-2,JOYPADY-2,SIZEY-2};

static Byte SpeedTxt[] = "Speed";		/* Local ASCII */
static Byte FireTxt[] = "Fire";
static Byte UseTxt[] = "Use";
static Byte *buttona[NUMCONTROLOPTIONS] =
		{SpeedTxt,SpeedTxt,FireTxt,FireTxt,UseTxt,UseTxt};
static Byte *buttonb[NUMCONTROLOPTIONS] =
		{FireTxt,UseTxt,SpeedTxt,UseTxt,SpeedTxt,FireTxt};
static Byte *buttonc[NUMCONTROLOPTIONS] =
		{UseTxt,FireTxt,UseTxt,SpeedTxt,FireTxt,SpeedTxt};

static Word configuration[NUMCONTROLOPTIONS][3] = {
	{PadA,PadB,PadC},
	{PadA,PadC,PadB},
	{PadB,PadA,PadC},
	{PadC,PadA,PadB},
	{PadB,PadC,PadA},
	{PadC,PadB,PadA}
};

/**********************************

	Init the button settings from the control type

**********************************/

static void SetButtonsFromControltype(void)
{
	Word *TablePtr;

    TablePtr = &configuration[ControlType][0];	/* Init table */
	PadSpeed = TablePtr[0];		/* Init the joypad settings */
	PadAttack =	TablePtr[1];
	PadUse = TablePtr[2];
	SetSfxVolume(SfxVolume);		/* Set the system volumes */
	SetMusicVolume(MusicVolume);	/* Set the music volume */
	InitMathTables();				/* Handle the math tables */
}

/**********************************

	Init the option screens
	Called on powerup.

**********************************/

void O_Init(void)
{
/* The prefs has set controltype, so set buttons from that */

	SetButtonsFromControltype();		/* Init the joypad settings */
	cursorcount = 0;		/* Init skull cursor state */
	cursorframe = 0;
	cursorpos = 0;
}

/**********************************

	Button bits can be eaten by clearing them in JoyPadButtons
	Called by player code.
	
**********************************/

void O_Control(player_t *player)
{
	Word buttons;

	buttons = JoyPadButtons;

	if (NewJoyPadButtons & PadX) {		/* Toggled the option screen? */
		if (player) {
			player->AutomapFlags ^= AF_OPTIONSACTIVE;	/* Toggle the flag */
			if (!(player->AutomapFlags & AF_OPTIONSACTIVE)) {	/* Shut down? */
				SetButtonsFromControltype();	/* Set the memory */
				WritePrefsFile();		/* Save new settings to NVRAM */
			}
		} else {
			SetButtonsFromControltype();	/* Set the memory */
			WritePrefsFile();		/* Save new settings to NVRAM */
		}
	}
	
	if (player) {
		if ( !(player->AutomapFlags & AF_OPTIONSACTIVE) ) {	/* Can I execute? */
			return;		/* Exit NOW! */
		}
	}
	
/* Clear buttons so game player isn't moving around */

	JoyPadButtons = buttons&PadX;	/* Leave option status alone */

/* animate skull */

	cursorcount += ElapsedTime;
	if (cursorcount >= (TICKSPERSEC/4)) {	/* Time up? */
		cursorframe ^= 1;		/* Toggle the frame */
		cursorcount = 0;		/* Reset the timer */
	}

/* Check for movement */

	if (! (buttons & (PadUp|PadDown|PadLeft|PadRight) ) ) {
		movecount = TICKSPERSEC;		// move immediately on next press
	} else {
		movecount += ElapsedTime;
		if ( (movecount >= (TICKSPERSEC/3)) ||		/* Allow slow */
			(cursorpos < controls && movecount >= (TICKSPERSEC/5))) {	/* Fast? */
			movecount = 0;		/* Reset timer*/
			
			/* Try to move the cursor up or down... */
			
			if (buttons & PadDown) {		
				++cursorpos;
				if (cursorpos >= NUMMENUITEMS) {
					cursorpos = 0;
				}
			}
			if (buttons & PadUp) {
				if (!cursorpos) {
					cursorpos = NUMMENUITEMS;
				}
				--cursorpos;
			}
			
			switch (cursorpos) {		/* Adjust the control */
			case controls:			/* Joypad? */	
				if (buttons & PadRight) {
					if (ControlType<(NUMCONTROLOPTIONS-1)) {
						++ControlType;
					}
				}
				if (buttons & PadLeft) {
					if (ControlType) {
						--ControlType;
					}
				}
				break;
			case soundvol:			/* Sound volume? */
				if (buttons & PadRight)	{
					if (SfxVolume<15) {
						++SfxVolume;
						S_StartSound(0,sfx_pistol);
					}
				}
				if (buttons & PadLeft) {
					if (SfxVolume) {
						--SfxVolume;
						S_StartSound(0,sfx_pistol);
					}
				}
				SetSfxVolume(SfxVolume);		/* Set the system volumes */
				break;
			case musicvol:			/* Music volume? */
				if (buttons & PadRight)	{
					if (MusicVolume<15) {
						++MusicVolume;
					}
				}
				if (buttons & PadLeft) {
					if (MusicVolume) {
						--MusicVolume;
					}
				}
				SetMusicVolume(MusicVolume);
				break;
			case size:			/* Screen size */
				if (buttons & PadLeft)	{
					if (ScreenSize<(6-1)) {
						++ScreenSize;
						if (player) {
							InitMathTables();				/* Handle the math tables */
						}
					}
				}
				if (buttons & PadRight) {
					if (ScreenSize) {		/* Can it grow? */
						if (LowDetail || (ScreenSize>=3)) {		/* Only 2-5? */
							--ScreenSize;
							if (player) {
								InitMathTables();				/* Handle the math tables */
							}
						}
					}
				}
			}	
		}
	}
}

/**********************************

	Draw the option screen

**********************************/

void O_Drawer(void)
{
	Word offset;
	void *Shapes;
	
/* Erase old and Draw new cursor frame */

	DrawMShape(CURSORX,CursorYs[cursorpos],GetShapeIndexPtr(LoadAResource(rSKULLS),cursorframe));
	ReleaseAResource(rSKULLS);
	Shapes = LoadAResource(rSLIDER);

/* Draw menu text */

	PrintBigFontCenter(160,10,(Byte *)"Options");

	if (cursorpos<controls) {
		PrintBigFontCenter(160,SFXVOLY,(Byte *)"Sound Volume");
		PrintBigFontCenter(160,MUSICVOLY,(Byte *)"Music Volume");

/* Draw scroll bars */

		DrawMShape(SLIDERX,SFXVOLY+20,GetShapeIndexPtr(Shapes,BAR));
		DrawMShape(SLIDERX,MUSICVOLY+20,GetShapeIndexPtr(Shapes,BAR));
		offset = SfxVolume * SLIDESTEP;
		DrawMShape(SLIDERX+5+offset,SFXVOLY+20,GetShapeIndexPtr(Shapes,HANDLE));
		offset = MusicVolume * SLIDESTEP;
		DrawMShape(SLIDERX+5+offset,MUSICVOLY+20,GetShapeIndexPtr(Shapes,HANDLE));
	
/* Draw joypad info */

	} else {
		PrintBigFontCenter(160,JOYPADY,(Byte *)"Controls");
		PrintBigFont(JOYPADX+10,JOYPADY+20,(Byte *)"A");
		PrintBigFont(JOYPADX+10,JOYPADY+40,(Byte *)"B");
		PrintBigFont(JOYPADX+10,JOYPADY+60,(Byte *)"C");
		PrintBigFont(JOYPADX+40,JOYPADY+20,buttona[ControlType]);
		PrintBigFont(JOYPADX+40,JOYPADY+40,buttonb[ControlType]);
		PrintBigFont(JOYPADX+40,JOYPADY+60,buttonc[ControlType]);
		PrintBigFontCenter(160,SIZEY,(Byte *)"Screen Size");
		DrawMShape(SLIDERX,SIZEY+20,GetShapeIndexPtr(Shapes,BAR));
		if (ScreenSize<2) {
			LowDetail = TRUE;
		}
		if (LowDetail) {
			offset = (5-ScreenSize) * 18;
		} else {
			offset = (5-ScreenSize) * 30;
		}
		DrawMShape(SLIDERX+5+offset,SIZEY+20,GetShapeIndexPtr(Shapes,HANDLE));
	}
	ReleaseAResource(rSLIDER);
	UpdateAndPageFlip();
}
