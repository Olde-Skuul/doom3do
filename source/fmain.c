#include "Doom.h"
#include <String.h>

#define CASTCOUNT 8

typedef enum {
	fin_endtext,
	fin_charcast
} final_e;

static Byte *CastNames[] = {		/* Names of all the critters */
	(Byte *)"Zombieman",
	(Byte *)"Shotgun Guy",
	(Byte *)"Imp",
	(Byte *)"Demon",
	(Byte *)"Lost Soul",
	(Byte *)"Cacodemon",
	(Byte *)"Baron of Hell",
	(Byte *)"Our Hero"
};

static mobjinfo_t *CastOrder[] = {	/* Pointer to the critter's base information */
	&mobjinfo[MT_POSSESSED],
	&mobjinfo[MT_SHOTGUY],
	&mobjinfo[MT_TROOP],
	&mobjinfo[MT_SERGEANT],
	&mobjinfo[MT_SKULL],
	&mobjinfo[MT_HEAD],
	&mobjinfo[MT_BRUISER],
	&mobjinfo[MT_PLAYER]
};

static mobjinfo_t *CastInfo;	/* Info for the current cast member */
static Word CastNum;		/* Which cast member is being displayed */
static Word CastTics;		/* Speed to animate the cast members */
static state_t *CastState;	/* Pointer to current state */
static Word CastFrames;		/* Number of frames of animation played */
static final_e status;		/* State of the display? */

#define TEXTTIME (TICKSPERSEC/10)		/* Tics to display letters */
#define STARTX 8			/* Starting x and y */
#define STARTY 8
static Boolean CastAttacking;	/* Currently attacking? */
static Boolean CastDeath;	/* Playing the death scene? */
static Boolean CastonMelee;	/* Type of attack to play */
static Word TextIndex;		/* Index to the opening text */
static Word TextDelay;		/* Delay before next char */

static Byte EndTextString[] =
	"     id software\n"
	"     salutes you!\n"
	"\n"
	"  the horrors of hell\n"
	"  could not kill you.\n"
	"  their most cunning\n"
	"  traps were no match\n"
	"  for you. you have\n"
	"  proven yourself the\n"
	"  best of all!\n"
	"\n"
	"  congratulations!";

/**********************************

	Print a string in a large font.
	NOTE : The strings must be lower case ONLY!

**********************************/

static void F_PrintString(Word text_x,Word text_y,Byte *string)
{
	Word index;
	Word val;

	index = 0;		/* Index into the string */
	for (;;) {		/* Stay forever */
		val = string[index];		/* Get first char */
		if (!val || val=='\n') {	/* End of line? */
			string[index] = 0;		/* Mark end of line */
			PrintBigFont(text_x,text_y,string);	/* Draw the string without newline */
			if (!val) {			/* Done now? */
				break;
			}
			string[index] = val;	/* Restore the string */
			string+=index+1;		/* Set the new start */
			index=-1;			/* Reset the start index */
			text_y += 15;		/* Next line down */
		}
		++index;
	}
}

/**********************************

	Load all data for the finale

**********************************/

void F_Start(void)
{
	S_StartSong(Song_final,TRUE);		/* Play the end game music */

	status = fin_endtext;		/* END TEXT PRINTS FIRST */
	TextIndex = 0;				/* At the beginning */
	TextDelay = 0;				/* Init the delay */
	CastNum = 0;		/* Start at the first monster */
	CastInfo = CastOrder[CastNum];
	CastState = CastInfo->seestate;
	CastTics = CastState->Time;		/* Init the time */
	CastDeath = FALSE;		/* Not dead */
	CastFrames = 0;			/* No frames shown */
	CastonMelee = FALSE;
	CastAttacking = FALSE;
}

/**********************************

	Release all memory for the finale

**********************************/

void F_Stop(void)
{
}

/**********************************

	Handle joypad info

**********************************/

Word F_Ticker(void)
{
	Word Temp;
/* Check for press a key to kill actor */

	if (status == fin_endtext) {		/* Am I printing text? */
		if (NewJoyPadButtons&(PadA|PadB|PadC) && (TotalGameTicks >= (3*TICKSPERSEC))) {
			status = fin_charcast;		/* Continue to the second state */
			S_StartSound(0,CastInfo->seesound);	/* Ohhh.. */
		}
		return ga_nothing;		/* Don't exit */
	}

	if (!CastDeath) {			/* Not dead? */
		if (NewJoyPadButtons&(PadA|PadB|PadC)) {	/* go into death frame */
			Temp = CastInfo->deathsound;		/* Get the sound when the actor is killed */
			if (Temp) {
				S_StartSound(0,Temp);
			}
			CastDeath = TRUE;		/* Death state */
			CastState = CastInfo->deathstate;
			CastTics = CastState->Time;
			CastFrames = 0;
			CastAttacking = FALSE;
		}
	}

/* Advance state */

	if (CastTics>ElapsedTime) {
		CastTics-=ElapsedTime;
		return ga_nothing;		/* Not time to change state yet */
	}

	if (CastState->Time == -1 || !CastState->nextstate) {
		/* switch from deathstate to next monster */
		++CastNum;
		if (CastNum>=CASTCOUNT) {
			CastNum = 0;
		}
		CastDeath = FALSE;
		CastInfo = CastOrder[CastNum];
		Temp = CastInfo->seesound;
		if (Temp) {
			S_StartSound (0,Temp);
		}
		CastState = CastInfo->seestate;
		CastFrames = 0;
	} else {	/* just advance to next state in animation */
		if (CastState == &states[S_PLAY_ATK1]) {
			goto stopattack;	// Oh, gross hack!
		}
		CastState = CastState->nextstate;
		++CastFrames;

/* sound hacks.... */

		{
			Word st;
			st = CastState - states;

			switch (st) {
			case S_POSS_ATK2:
				Temp = sfx_pistol;
				break;
			case S_SPOS_ATK2:
				Temp = sfx_shotgn;
				break;
			case S_TROO_ATK3:
				Temp = sfx_claw;
				break;
			case S_SARG_ATK2:
				Temp = sfx_sgtatk;
				break;
			case S_BOSS_ATK2:
			case S_HEAD_ATK2:
				Temp = sfx_firsht;
				break;
			case S_SKULL_ATK2:
				Temp = sfx_sklatk;
				break;
			default:
				Temp = 0;
			}
			if (Temp) {
				S_StartSound(0,Temp);
			}
		}
	}

	if (CastFrames == 12) {		/* go into attack frame */
		CastAttacking = TRUE;
		if (CastonMelee) {
			CastState=CastInfo->meleestate;
		} else {
			CastState=CastInfo->missilestate;
		}
		CastonMelee ^= TRUE;		/* Toggle the melee state */
		if (!CastState) {
			if (CastonMelee) {
				CastState=CastInfo->meleestate;
			} else {
				CastState=CastInfo->missilestate;
			}
		}
	}

	if (CastAttacking) {
		if (CastFrames == 24
			|| CastState == CastInfo->seestate) {
stopattack:
			CastAttacking = FALSE;
			CastFrames = 0;
			CastState = CastInfo->seestate;
		}
	}

	CastTics = CastState->Time;		/* Get the next time */
	if (CastTics == -1) {
		CastTics = (TICKSPERSEC/4);		/* 1 second timer */
	}
	return ga_nothing;		/* finale never exits */
}

/**********************************

	Draw the frame for the finale

**********************************/

void F_Drawer(void)
{
	DrawRezShape(0,0,rBACKGRNDBROWN);		/* Draw the background */
	
	if (status==fin_endtext) {
		Word Temp;
		Temp = EndTextString[TextIndex];		/* Get the final char */
		EndTextString[TextIndex] = 0;			/* End the string here */
		F_PrintString(STARTX,STARTY,EndTextString);	/* Print the string */
		EndTextString[TextIndex] = Temp;	/* Restore the string */
		TextDelay+=ElapsedTime;		/* How much time has gone by? */
		if (TextDelay>=TEXTTIME) {
			TextDelay -= TEXTTIME;		/* Adjust the time */
			if (Temp) {		/* Already at the end? */
				++TextIndex;
			}
		} 
	} else {
		PrintBigFontCenter(160,20,CastNames[CastNum]);	/* Print the name */
		DrawSpriteCenter(CastState->SpriteFrame);		/* Draw the sprite */
	}
	UpdateAndPageFlip();		/* Show the frame */
}
