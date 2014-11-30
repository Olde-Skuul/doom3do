#include "Doom.h"

/**********************************

	Local structures

**********************************/

#define BUTTONTIME TICKSPERSEC		/* 1 second */

typedef enum {		/* Positions for the button */
	top,			/* Top button */
	middle,			/* Middle button */
	bottom			/* Bottom button */
} bwhere_e;

typedef struct {		/* Struct to describe a switch */
	line_t *line;		/* Line that the button is on */
	Word btexture;		/* Texture # */
	Word btimer;		/* Time before switching back */
	bwhere_e where;		/* Vertical position of the button */
} button_t;

Word NumSwitches;		/* Number of switches * 2 */
Word SwitchList[] = {
	rSW1BRN1-rT_START,rSW2BRN1-rT_START,			/* Before,After */
	rSW1GARG-rT_START,rSW2GARG-rT_START,
	rSW1GSTON-rT_START,rSW2GSTON-rT_START,
	rSW1HOT-rT_START,rSW2HOT-rT_START,
	rSW1STAR-rT_START,rSW2STAR-rT_START,
	rSW1WOOD-rT_START,rSW2WOOD-rT_START
};

/**********************************

	Think logic for a button

**********************************/

static void T_Button(button_t *button)
{
	/* Do buttons */

	if (button->btimer>ElapsedTime) {	/* Time up? */
		button->btimer-=ElapsedTime;	/* Adjust timer */
	} else {
		line_t *line;
		side_t *MySide;

		line = button->line;
		MySide = line->SidePtr[0];	/* Get the side record */
		switch(button->where) {
		case top:
			MySide->toptexture = button->btexture;
			break;
		case middle:
			MySide->midtexture = button->btexture;
			break;
		case bottom:
			MySide->bottomtexture =	button->btexture;
			break;
		}
		S_StartSound(&line->frontsector->SoundX,sfx_swtchn);
		RemoveThinker(button);  /* unlink and free */
	}
}

/**********************************

	Init the switch list so that textures can swap
	depending on the state of a switch

**********************************/

void P_InitSwitchList(void)
{
	NumSwitches = sizeof(SwitchList)/sizeof(Word);
}

/**********************************

	Start a button counting down till it turns off.

**********************************/

static void P_StartButton(line_t *line,bwhere_e w,Word texture,Word time)
{
	button_t *button;

	button = (button_t *)AddThinker(T_Button,sizeof(button_t));
	button->line = line;
	button->where = w;
	button->btexture = texture;
	button->btimer = time;
}

/**********************************

	Function that changes wall texture.
	Tell it if switch is ok to use again (1=yes, it's a button).

**********************************/

void P_ChangeSwitchTexture(line_t *line,Boolean useAgain)
{
	Word texTop;		/* Cached texture numbers */
	Word texMid;
	Word texBot;
	Word i;
	Word sound;		/* Sound effect to play */
	side_t *MySide;	/* Pointer to the side struct */
	Fixed *SoundOrg;

	if (!useAgain) {
		line->special = 0;
	}
	MySide = line->SidePtr[0];
	SoundOrg = &line->frontsector->SoundX;

	texTop = MySide->toptexture;
	texMid = MySide->midtexture;
	texBot = MySide->bottomtexture;

	sound = sfx_swtchn;
	if (line->special == 11) {		/* EXIT SWITCH? */
		sound = sfx_swtchx;
	}

	i = 0;
	do {
		if (SwitchList[i] == texTop) {
			S_StartSound(SoundOrg,sound);
			MySide->toptexture = SwitchList[i^1];
			if (useAgain) {
				P_StartButton(line,top,texTop,BUTTONTIME);
			}
			return;
		} else if (SwitchList[i] == texMid) {
			S_StartSound(SoundOrg,sound);
			MySide->midtexture = SwitchList[i^1];
			if (useAgain) {
				P_StartButton(line,middle,texMid,BUTTONTIME);
			}
			return;
		} else if (SwitchList[i] == texBot) {
			S_StartSound(SoundOrg,sound);
			MySide->bottomtexture = SwitchList[i^1];
			if (useAgain) {
				P_StartButton(line,bottom,texBot,BUTTONTIME);
			}
			return;
		}
	} while (++i<NumSwitches);
}

/**********************************

	Called when a thing uses a special line
	Only the front sides of lines are usable

**********************************/

Boolean P_UseSpecialLine(mobj_t *thing,line_t *line)
{
	/* Switches that other things can activate */

	if (!thing->player) {		/* Monster? */
		if (line->flags & ML_SECRET) {
			return FALSE;		/* never open secret doors */
		}
		switch(line->special) {
		default:
			return FALSE;
		case 1:		/* MANUAL DOOR RAISE */
#if 0			/* Don't let monsters open locked doors */
		case 32:	/* MANUAL BLUE */
		case 33:	/* MANUAL RED */
		case 34:	/* MANUAL YELLOW */
#endif
				;		/* No effect */
		}
	}

	/* do something */

	switch (line->special) {

    /* Normal doors */
	case 1:			// Vertical Door
	case 31:		// Manual door open
	case 26:		// Blue Card Door Raise
	case 32:		// Blue Card door open
	case 99:		// Blue Skull Door Open
	case 106:		// Blue Skull Door Raise
	case 27:		// Yellow Card Door Raise
	case 34:		// Yellow Card door open
	case 105:		// Yellow Skull Door Open
	case 108:		// Yellow Skull Door Raise
	case 28:		// Red Card Door Raise
	case 33:		// Red Card door open
	case 100:		// Red Skull Door Open
	case 107:		// Red Skull Door Raise
		EV_VerticalDoor(line,thing);
		break;

	/* Buttons */

	case 42:		// Close Door
		if (EV_DoDoor(line,close)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 43:		// Lower Ceiling to Floor
		if (EV_DoCeiling(line,lowerToFloor)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 45:		// Lower Floor to Surrounding floor height
		if (EV_DoFloor(line,lowerFloor)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 60:		// Lower Floor to Lowest
		if (EV_DoFloor(line,lowerFloorToLowest)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 61:		// Open Door
		if (EV_DoDoor(line,open)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 62:		// PlatDownWaitUpStay
		if (EV_DoPlat(line,downWaitUpStay,1)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 63:		// Raise Door
		if (EV_DoDoor(line,normaldoor)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 64:		// Raise Floor to ceiling
		if (EV_DoFloor(line,raiseFloor)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 66:		// Raise Floor 24 and change texture
		if (EV_DoPlat(line,raiseAndChange,24)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 67:		// Raise Floor 32 and change texture
		if (EV_DoPlat(line,raiseAndChange,32)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 65:		// Raise Floor Crush
		if (EV_DoFloor(line,raiseFloorCrush)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 68:		// Raise Plat to next highest floor and change texture
		if (EV_DoPlat(line,raiseToNearestAndChange,0)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 69:		// Raise Floor to next highest floor
		if (EV_DoFloor(line, raiseFloorToNearest)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;
	case 70:		// Turbo Lower Floor
		if (EV_DoFloor(line,turboLower)) {
			P_ChangeSwitchTexture(line,TRUE);
		}
		break;

	/* Switches (One shot buttons) */

	case 7:			// Build Stairs
		if (EV_BuildStairs(line)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 9:			// Change Donut
		if (EV_DoDonut(line)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 11:		// Exit level
		G_ExitLevel();
		P_ChangeSwitchTexture(line,FALSE);
		break;
	case 14:		// Raise Floor 32 and change texture
		if (EV_DoPlat(line,raiseAndChange,32)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 15:		// Raise Floor 24 and change texture
		if (EV_DoPlat(line,raiseAndChange,24)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 18:		// Raise Floor to next highest floor
		if (EV_DoFloor(line, raiseFloorToNearest)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 20:		// Raise Plat next highest floor and change texture
		if (EV_DoPlat(line,raiseToNearestAndChange,0)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 21:		// PlatDownWaitUpStay
		if (EV_DoPlat(line,downWaitUpStay,0)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 23:		// Lower Floor to Lowest
		if (EV_DoFloor(line,lowerFloorToLowest)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 29:		// Raise Door
		if (EV_DoDoor(line,normaldoor)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 41:		// Lower Ceiling to Floor
		if (EV_DoCeiling(line,lowerToFloor)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 71:		// Turbo Lower Floor
		if (EV_DoFloor(line,turboLower)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 49:		// Lower Ceiling And Crush
		if (EV_DoCeiling(line,lowerAndCrush)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 50:		// Close Door
		if (EV_DoDoor(line,close)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 51:		// Secret EXIT
		G_SecretExitLevel();
		P_ChangeSwitchTexture(line,FALSE);
		break;
	case 55:		// Raise Floor Crush
		if (EV_DoFloor(line,raiseFloorCrush))
			P_ChangeSwitchTexture(line,FALSE);
		break;
	case 101:		// Raise Floor
		if (EV_DoFloor(line,raiseFloor)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 102:		// Lower Floor to Surrounding floor height
		if (EV_DoFloor(line,lowerFloor)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	case 103:		// Open Door
		if (EV_DoDoor(line,open)) {
			P_ChangeSwitchTexture(line,FALSE);
		}
		break;
	}
	return TRUE;
}
