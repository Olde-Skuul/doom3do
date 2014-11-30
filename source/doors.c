#include "Doom.h"

/**********************************

	Local structures

**********************************/

#define	VDOORSPEED (6<<FRACBITS)	/* Speed to open a vertical door */
#define	VDOORWAIT ((TICKSPERSEC*14)/3)	/* Door time to wait before closing 4.6 seconds */

typedef struct {
	sector_t *sector;	/* Sector being modified */
	Fixed topheight;	/* Topmost height */
	Fixed speed;		/* Speed of door motion */
	int direction;		/* 1 = up, 0 = waiting at top, -1 = down */
	Word topwait;		/* tics to wait at the top */
						/* (keep in case a door going down is reset) */
	Word topcountdown;	/* when it reaches 0, start going down */
	vldoor_e type;		/* Type of door */
} vldoor_t;


/**********************************

	Think logic for doors

**********************************/

static void T_VerticalDoor(vldoor_t *door)
{
	result_e res;

	switch(door->direction) {
	case 0:		/* Waiting or in stasis */
		if (door->topcountdown>ElapsedTime) {
			door->topcountdown-=ElapsedTime;
		} else {
			door->topcountdown=0;		/* Force zero */
			switch(door->type) {
			case normaldoor:
				door->direction = -1;	/* Time to go back down */
				S_StartSound(&door->sector->SoundX,sfx_dorcls);
				break;
			case close30ThenOpen:
				door->direction = 1;
				S_StartSound(&door->sector->SoundX,sfx_doropn);
			}
		}
		break;
	case 2:		/* INITIAL WAIT */
		if (door->topcountdown>ElapsedTime) {
			door->topcountdown-=ElapsedTime;
		} else {
			door->topcountdown=0;		/* Force zero */
			if (door->type == raiseIn5Mins) {
				door->direction = 1;
				door->type = normaldoor;
				S_StartSound(&door->sector->SoundX,sfx_doropn);
			}
		}
		break;
	case -1:	/* DOWN */
		res = T_MovePlane(door->sector,door->speed,
			door->sector->floorheight,FALSE,TRUE,door->direction);
		if (res == pastdest) {		/* Finished closing? */
			switch(door->type) {
			case normaldoor:
			case close:
				door->sector->specialdata = 0;	/* Remove it */
				RemoveThinker(door);  /* unlink and free */
				break;
			case close30ThenOpen:
				door->direction = 0;		/* Waiting */
				door->topcountdown = (TICKSPERSEC*30);
			}
		} else if (res == crushed) {
			door->direction = 1;		/* Move back up */
			S_StartSound(&door->sector->SoundX,sfx_doropn);
		}
		break;
	case 1:		/* UP */
		res = T_MovePlane(door->sector,door->speed,
			door->topheight,FALSE,TRUE,door->direction);
		if (res == pastdest) {		/* Fully opened? */
			switch(door->type) {
			case normaldoor:
				door->direction = 0; /* wait at top */
				door->topcountdown = door->topwait;		/* Reset timer */
				break;
			case close30ThenOpen:
			case open:
				door->sector->specialdata = 0;
				RemoveThinker(door);  /* unlink and free */
			}
		}
	}
}

/**********************************

	Move a door up/down and all around!

**********************************/

Boolean EV_DoDoor(line_t *line,vldoor_e type)
{
	Word secnum;
	Boolean rtn;
	sector_t *sec;
	vldoor_t *door;

	secnum = -1;
	rtn = FALSE;
	while ((secnum = P_FindSectorFromLineTag(line,secnum)) != -1) {
		sec = &sectors[secnum];
		if (sec->specialdata) {		/* Already something here? */
			continue;
		}

		/* new door thinker */

		rtn = TRUE;
		door = (vldoor_t *)AddThinker(T_VerticalDoor,sizeof(vldoor_t));
		sec->specialdata = door;
		door->sector = sec;
		door->type = type;		/* Save the type */
		door->speed = VDOORSPEED;		/* Save the speed */
		door->topwait = VDOORWAIT;		/* Save the initial delay */
		switch(type) {
		case close:
			door->topheight = P_FindLowestCeilingSurrounding(sec);
			door->topheight -= 4*FRACUNIT;
			door->direction = -1;		/* Down */
			S_StartSound(&door->sector->SoundX,sfx_dorcls);
			break;
		case close30ThenOpen:
			door->topheight = sec->ceilingheight;
			door->direction = -1;		/* Down */
			S_StartSound(&door->sector->SoundX,sfx_dorcls);
			break;
		case normaldoor:
		case open:
			door->direction = 1;		/* Up */
			door->topheight = P_FindLowestCeilingSurrounding(sec);
			door->topheight -= 4*FRACUNIT;
			if (door->topheight != sec->ceilingheight) {
				S_StartSound(&door->sector->SoundX,sfx_doropn);
			}
		}
	}
	return rtn;
}

/**********************************

	Open a door manually, no tag value

**********************************/

void EV_VerticalDoor(line_t *line,mobj_t *thing)
{
	player_t *player;
	sector_t *sec;
	vldoor_t *door;

/* Check for locks */

	player = thing->player;		/* Is this a player? */
	if (player) {				/* Only player's have trouble with locks */
		Word i;

		i = -1;			/* Don't quit! */
		switch(line->special) {
		case 26:		/* Blue Card Lock */
		case 32:
		case 99:		/* Blue Skull Lock */
		case 106:
			if (!player->cards[it_bluecard] && !player->cards[it_blueskull]) {
				i = (line->special<99) ? it_bluecard : it_blueskull;
			}
			break;
		case 27:		/* Yellow Card Lock */
		case 34:
		case 105:		/* Yellow Skull Lock */
		case 108:
			if (!player->cards[it_yellowcard] && !player->cards[it_yellowskull]) {
				i = (line->special<105) ? it_yellowcard : it_yellowskull;
			}
			break;
		case 28:		/* Red Card Lock */
		case 33:
		case 100:		/* Red Skull Lock */
		case 107:
			if (!player->cards[it_redcard] && !player->cards[it_redskull]) {
				i = (line->special<100) ? it_redcard : it_redskull;
			}
			break;
		}
		if (i!=-1) {
			S_StartSound(&thing->x,sfx_oof);	/* Play the sound */
			stbar.tryopen[i] = TRUE;		/* Trigger on status bar */
			return;
		}
	}

	/* if the sector has an active thinker, use it */

	sec = line->backsector;	/* Get the sector pointer */
	if (sec->specialdata) {
		door = (vldoor_t *)sec->specialdata;	/* Use existing */
		switch(line->special) {
		case   1:	// ONLY FOR "RAISE" DOORS, NOT "OPEN"s
		case  26:	// BLUE CARD
		case  27:	// YELLOW CARD
		case  28:	// RED CARD
		case 106:	// BLUE SKULL
		case 108:	// YELLOW SKULL
		case 107:	// RED SKULL
			if (door->direction == -1) {	/* Closing? */
				door->direction = 1;	/* go back up */
			} else if (thing->player) {		/* Only players make it close */
				door->direction = -1;	/* start going down immediately */
			}
			return;		/* Exit now */
		}
	}

	/* for proper sound */

	switch (line->special) {
	case 1:		/* NORMAL DOOR SOUND */
	case 31:
		S_StartSound(&sec->SoundX,sfx_doropn);
		break;
	default:	/* LOCKED DOOR SOUND */
		S_StartSound(&sec->SoundX,sfx_doropn);
		break;
	}

	/* new door thinker */

	door = (vldoor_t *)AddThinker(T_VerticalDoor,sizeof(vldoor_t));
	sec->specialdata = door;
	door->sector = sec;
	door->direction = 1;		/* Going up! */
	door->speed = VDOORSPEED;		/* Set the speed */
	door->topwait = VDOORWAIT;

	switch(line->special) {
	case 1:
	case 26:
	case 27:
	case 28:
		door->type = normaldoor;		/* Normal open/close door */
		break;
	case 31:
	case 32:
	case 33:
	case 34:
		door->type = open;		/* Open forever */
	}
	/* Find the top and bottom of the movement range */

	door->topheight = P_FindLowestCeilingSurrounding(sec)-(4<<FRACBITS);
}

/**********************************

	Spawn a door that closes after 30 seconds

**********************************/

void P_SpawnDoorCloseIn30 (sector_t *sec)
{
	vldoor_t *door;

	door = (vldoor_t *)AddThinker(T_VerticalDoor,sizeof(vldoor_t));
	sec->specialdata = door;
	sec->special = 0;
	door->sector = sec;
	door->direction = 0;		/* Standard wait */
	door->type = normaldoor;
	door->speed = VDOORSPEED;
	door->topcountdown = (30*TICKSPERSEC);
}

/**********************************

	Spawn a door that opens after 5 minutes

**********************************/

void P_SpawnDoorRaiseIn5Mins(sector_t *sec)
{
	vldoor_t *door;

	door = (vldoor_t *)AddThinker(T_VerticalDoor,sizeof(vldoor_t));
	sec->specialdata = door;
	sec->special = 0;
	door->sector = sec;
	door->direction = 2;		/* Initial wait */
	door->type = raiseIn5Mins;
	door->speed = VDOORSPEED;
	door->topheight = P_FindLowestCeilingSurrounding(sec)-(4<<FRACBITS);
	door->topwait = VDOORWAIT;
	door->topcountdown = (5*60*TICKSPERSEC);
}
