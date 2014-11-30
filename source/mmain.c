#include "Doom.h"

#define CURSORX 50		/* X coord of skull cursor */
#define AREAY 66
#define DIFFICULTYY 116
#define OPTIONSY 166

typedef enum {
	level,
	difficulty,
	options,
	NUMMENUITEMS
} menu_t;

enum {		/* Enum to index to the shapes */
	DIFFSHAPE1,
	DIFFSHAPE2,
	DIFFSHAPE3,
	DIFFSHAPE4,
	DIFFSHAPE5,
	AREASHAPE,
	DIFFSHAPE
};

static Word cursorframe;	/* Skull animation frame */
static Word cursorcount;	/* Time mark to animate the skull */
static Word cursorpos;		/* Y position of the skull */
static Word movecount;		/* Time mark to move the skull */
static Word playermap;		/* Map requested */
static Word playerskill;	/* Requested skill */
static LongWord SleepMark;	/* Time from last access */
static Word CursorYs[NUMMENUITEMS] = {AREAY-2,DIFFICULTYY-2,OPTIONSY-2};
static Word OptionActive;

/**********************************

	Init memory needed for the main game menu

**********************************/

void M_Start(void)
{
	cursorcount = 0;		/* Init the animation timer */
	cursorframe = 0;		/* Init the animation frame */
	cursorpos = 0;			/* Topmost y position */
	playerskill = StartSkill;	/* Init the skill level */
	playermap = StartMap;	/* Init the starting map */
	SleepMark = ReadTick();
	OptionActive = FALSE;	/* Option screen on */
}

/**********************************

	Release memory used by the main menu

**********************************/

void M_Stop(void)
{
	WritePrefsFile();		/* Save the current prefs */
}

/**********************************

	Execute every tick

**********************************/

Word M_Ticker(void)
{
	Word buttons;

	buttons = JoyPadButtons;	/* Get the joypad buttons */

/* Exit menu if button press */

	if (TotalGameTicks > (TICKSPERSEC/2) &&		/* Minimum time... */
		((buttons & PadStart) || 		/* Start always works! */
		((buttons & (PadA|PadB|PadC|PadStart)) && (cursorpos!=options)))) {
		StartMap = playermap;	/* set map number */
		StartSkill = (skill_t)playerskill;	/* Set skill level */
		return ga_completed;		/* done with menu */
	}
	
	if (buttons) {			/* If buttons are held down then reset the timer */
		SleepMark = ReadTick();
	}
	
	if (OptionActive) {
		O_Control(0);
		if (NewJoyPadButtons&PadX) {
			OptionActive = FALSE;
		}
		return ga_nothing;
	}
	
	if ((NewJoyPadButtons&PadX) ||		/* Pressed abort? */
		((ReadTick()-SleepMark)>=(TICKSPERSEC*15))) {
		return ga_died;					/* Exit now */
	}

/* Animate skull */

	cursorcount += ElapsedTime;		/* Add time */
	if (cursorcount>=(TICKSPERSEC/4)) {	/* Time to toggle the shape? */
		cursorframe ^= 1;
		cursorcount = 0;		/* Reset the count */
	}

/* Check for movement */

	if (! (buttons & (PadUp|PadDown|PadLeft|PadRight|PadA|PadB|PadC|PadD) ) ) {
		movecount = TICKSPERSEC;	/* Move immediately on next press */
	} else {
		movecount += ElapsedTime;	/* Time unit */
		if ( (movecount >= (TICKSPERSEC/3)) ||		/* Allow slow */
			(cursorpos == level && movecount >= (TICKSPERSEC/5))) {	/* Fast? */
			movecount = 0;		/* Reset the timer */
			if (buttons & PadDown) {
				++cursorpos;
				if (cursorpos>=NUMMENUITEMS) {		/* Off the bottom? */
					cursorpos = 0;
				}
			}
			if (buttons & PadUp) {		/* Going up? */
				if (!cursorpos) {		/* At the top already? */
					cursorpos = NUMMENUITEMS;
				}
				--cursorpos;
			}

			switch (cursorpos) {
			case level:				/* Select level to start with */
				if (buttons & PadRight) {
					if (playermap < MaxLevel) {
						++playermap;
					}
				}
				if (buttons & PadLeft) {
					if (playermap!=1) {
						--playermap;
					}
				}
				break;
			case difficulty:		/* Select game difficulty */
				if (buttons & PadRight) {
					if (playerskill < sk_nightmare) {
						++playerskill;
					}
				}
				if (buttons & PadLeft) {
					if (playerskill) {
						--playerskill;
					}
				}
				break;
			case options:
				if (buttons & (PadA|PadB|PadC|PadD|PadRight|PadLeft)) {
					OptionActive = TRUE;
				}
			}
		}
	}
	return ga_nothing;		/* Don't quit! */
}

/**********************************

	Draw the main menu

**********************************/

void M_Drawer(void)
{
	void *Shapes;		/* Pointer to shape array */
	
/* Draw background */

	DrawRezShape(0,0,rMAINDOOM);
	
	if (OptionActive) {
		O_Drawer();
	} else {
	
		Shapes = LoadAResource(rMAINMENU);	/* Load shape group */

/* Draw new skull */

		DrawMShape(CURSORX,CursorYs[cursorpos],GetShapeIndexPtr(LoadAResource(rSKULLS),cursorframe));
		ReleaseAResource(rSKULLS);

/* Draw start level information */

		PrintBigFont(CURSORX+24,AREAY,(Byte *)"Level");
		PrintNumber(CURSORX+40,AREAY+20,playermap,0);

/* Draw difficulty information */

		DrawMShape(CURSORX+24,DIFFICULTYY,GetShapeIndexPtr(Shapes,DIFFSHAPE));
		DrawMShape(CURSORX+40,DIFFICULTYY+20,GetShapeIndexPtr(Shapes,playerskill));

/* Draw the options screen */

		PrintBigFont(CURSORX+24,OPTIONSY,(Byte *)"Options Menu");
		ReleaseAResource(rMAINMENU);
		UpdateAndPageFlip();			/* Update and exit */
	}
}
