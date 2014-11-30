#include "Doom.h"

/**********************************

	Local structures

**********************************/

#define	PLATSPEED (2<<FRACBITS)		/* Speed of platform motion */
#define	PLATWAIT (3*TICKSPERSEC)	/* Delay in seconds before platform motion */

typedef enum {		/* Current action for the platform */
	up,
	down,
	waiting,
	in_stasis
} plat_e;

typedef struct plat_t {		/* Structure for a moving platform */
	sector_t *sector;	/* Sector the platform will modify */
	struct plat_t *next;	/* Next entry in the linked list */
	Fixed speed;		/* Speed of motion */
	Fixed low;			/* Lowest Y point */
	Fixed high;			/* Highest Y point */
	Word wait;			/* Wait in ticks before moving back (Zero = never) */
	Word count;			/* Running count (If zero then wait forever!) */
	Word tag;			/* Event ID tag */
	plat_e status;		/* Current status (up,down,wait,dead) */
	plat_e oldstatus;	/* Previous status */
	plattype_e type;	/* Type of platform */
	Boolean crush;		/* Can it crush things? */
} plat_t;

static plat_t *MainPlatPtr;		/* Pointer to the first plat in list */

/**********************************

	Insert a new platform record into the linked list,
	use a simple insert into the head to add.

**********************************/

static void AddActivePlat(plat_t *PlatPtr)
{
	PlatPtr->next = MainPlatPtr;	/* Insert into the chain */
	MainPlatPtr = PlatPtr;		/* Set this one as the first */
}

/**********************************

	Remove a platform record from the linked list.
	I also call RemoveThinker to actually perform the
	memory disposal.

**********************************/

static void RemoveActivePlat(plat_t *PlatPtrKill)
{
	plat_t *PlatPtr;		/* Temp pointer */
	plat_t *PrevPtr;		/* Previous pointer (For linked list) */

	PrevPtr = 0;			/* Init the previous pointer */
	PlatPtr = MainPlatPtr;	/* Get the main list entry */
	while (PlatPtr) {		/* Failsafe! */
		if (PlatPtr==PlatPtrKill) {	/* Master pointer matches? */
			PlatPtr = PlatPtr->next;	/* Get the next link */
			if (!PrevPtr) {				/* First one in chain? */
				MainPlatPtr = PlatPtr;	/* Make the next one the new head */
			} else {
				PrevPtr->next = PlatPtr;	/* Remove the link */
			}
			break;		/* Get out of the loop */
		}
		PrevPtr = PlatPtr;		/* Make the current pointer the previous one */
		PlatPtr = PlatPtr->next;	/* Get the next link */
    }
	PlatPtrKill->sector->specialdata = 0;	/* Unlink from the sector */
	RemoveThinker(PlatPtrKill);		/* Remove the thinker */
}

/**********************************

	Move a plat up and down
	There are only 4 states a platform can be in...
	Going up, Going down, waiting to either go up or down or finally
	not active and needing an external event to trigger it.

**********************************/

static void T_PlatRaise(plat_t *plat)
{
	result_e res;		/* ok, crushed, pastdest */

	switch(plat->status) {		/* State of the platform */
	case up:		/* Going up? */
		res = T_MovePlane(plat->sector,plat->speed,plat->high,plat->crush,FALSE,1);
		if (plat->type == raiseAndChange ||
			plat->type == raiseToNearestAndChange) {
			if (Tick2) {		/* Make the rumbling sound */
				S_StartSound(&plat->sector->SoundX,sfx_stnmov);
			}
		}

		if (res == crushed && !plat->crush) {		/* Crushed something? */
			plat->count = plat->wait;	/* Get the delay time */
			plat->status = down;		/* Going the opposite direction */
			S_StartSound(&plat->sector->SoundX,sfx_pstart);
		} else if (res == pastdest) {	/* Moved too far? */
			plat->count = plat->wait;	/* Reset the timer */
			plat->status = waiting;		/* Make it wait */
			S_StartSound(&plat->sector->SoundX,sfx_pstop);
			switch(plat->type) {		/* What type of platform is it? */
			case downWaitUpStay:		/* Shall it stay here forever? */
			case raiseAndChange:		/* Change the texture and exit? */
				RemoveActivePlat(plat);	/* Remove it then */
			}
		}
		break;
	case down:		/* Going down? */
		res = T_MovePlane(plat->sector,plat->speed,plat->low,FALSE,FALSE,-1);
		if (res == pastdest) {		/* Moved too far */
			plat->count = plat->wait;	/* Set the delay count */
			plat->status = waiting;		/* Delay mode */
			S_StartSound(&plat->sector->SoundX,sfx_pstop);
		}
		break;
	case waiting:
		if (plat->count) {		/* If waiting will expire... */
			if (plat->count>ElapsedTime)	{		/* Time up? */
				plat->count-=ElapsedTime;	/* Remove the time (But leave 1) */
			} else {
				if (plat->sector->floorheight == plat->low) {	/* At the bottom? */
					plat->status = up;		/* Move up */
				} else {
					plat->status = down;	/* Move down */
				}
				S_StartSound(&plat->sector->SoundX,sfx_pstart);
			}
		}
	}
}

/**********************************

	Given a tag ID number, activate a platform that is currently
	inactive.

**********************************/

static void ActivateInStasis(Word tag)
{
	plat_t *PlatPtr;		/* Temp pointer */

	PlatPtr = MainPlatPtr;	/* Get the main list entry */
	while (PlatPtr) {		/* Scan all entries in the thinker list */
		if (PlatPtr->tag == tag && PlatPtr->status == in_stasis) {	/* Match? */
			PlatPtr->status = PlatPtr->oldstatus;	/* Restart the platform */
			ChangeThinkCode(PlatPtr,T_PlatRaise);	/* Reset code */
		}
		PlatPtr = PlatPtr->next;	/* Get the next link */
    }
}

/**********************************

	Trigger a platform. Perform the action for all platforms
	"amount" is only used for SOME platforms.

**********************************/

Boolean EV_DoPlat(line_t *line,plattype_e type,Word amount)
{
	plat_t *plat;		/* Pointer to new platform */
	Word secnum;		/* Which sector am I in? */
	Boolean rtn;		/* True if I created a platform */
	sector_t *sec;		/* Pointer to current sector */

	rtn = FALSE;		/* Assume false */

	/* Activate all <type> plats that are in_stasis */

	if (type==perpetualRaise) {
		ActivateInStasis(line->tag);		/* Reset the platforms */
	}

	secnum = -1;
	while ((secnum = P_FindSectorFromLineTag(line,secnum)) != -1) {
		sec = &sectors[secnum];	/* Get the sector pointer */
		if (sec->specialdata) {		/* Already has a platform? */
			continue;		/* Skip */
		}

		/* Find lowest & highest floors around sector */

		rtn = TRUE;		/* I created a sector */
		plat = (plat_t *)AddThinker(T_PlatRaise,sizeof(plat_t));		/* Add to the thinker list */
		plat->type = type;		/* Save the platform type */
		plat->sector = sec;		/* Save the sector pointer */
		plat->sector->specialdata = plat;	/* Point back to me... */
		plat->crush = FALSE;	/* Can't crush anything */
		plat->tag = line->tag;	/* Assume the line's ID */
		switch(type) {			/* Init vars based on type */

		case raiseToNearestAndChange:	/* Go up and stop */
			sec->special = 0;	/* If lava, then stop hurting the player */
			plat->high = P_FindNextHighestFloor(sec,sec->floorheight);
			goto RaisePart2;

		case raiseAndChange:	/* Raise a specific amount and stop */
			plat->high = sec->floorheight + (amount<<FRACBITS);
RaisePart2:
			plat->speed = PLATSPEED/2;		/* Slow speed */
			sec->FloorPic = line->frontsector->FloorPic;
			plat->wait = 0;		/* No delay before moving */
			plat->status = up;	/* Going up! */
			S_StartSound(&sec->SoundX,sfx_stnmov);	/* Begin move */
			break;
		case downWaitUpStay:
			plat->speed = PLATSPEED * 4;		/* Fast speed */
			plat->low = P_FindLowestFloorSurrounding(sec);	/* Lowest floor */
			if (plat->low > sec->floorheight) {
				plat->low = sec->floorheight;		/* Go to the lowest mark */
			}
			plat->high = sec->floorheight;		/* Allow to return */
			plat->wait = PLATWAIT;		/* Set the delay when it hits bottom */
			plat->status = down;		/* Go down */
			S_StartSound(&sec->SoundX,sfx_pstart);
			break;
		case perpetualRaise:
			plat->speed = PLATSPEED;		/* Set normal speed */
			plat->low = P_FindLowestFloorSurrounding(sec);
			if (plat->low > sec->floorheight) {
				plat->low = sec->floorheight;	/* Set lower mark */
			}
			plat->high = P_FindHighestFloorSurrounding(sec);
			if (plat->high < sec->floorheight) {
				plat->high = sec->floorheight;	/* Set highest mark */
			}
			plat->wait = PLATWAIT;		/* Delay when it hits bottom */
			plat->status = (plat_e) GetRandom(1);	/* Up or down */
			S_StartSound(&sec->SoundX,sfx_pstart);	/* Start */
			break;
		}
		AddActivePlat(plat);		/* Add the platform to the list */
	}
	return rtn;		/* Did I create one? */
}

/**********************************

	Deactivate a platform
	Only affect platforms that have the same ID tag
	as the line segment and also are active.

**********************************/

void EV_StopPlat(line_t *line)
{
	plat_t *PlatPtr;		/* Temp pointer */
	Word tag;				/* Get the ID tag */

	tag = line->tag;		/* Cache the tag */
	PlatPtr = MainPlatPtr;	/* Get the main list entry */
	while (PlatPtr) {		/* Scan all entries in the thinker list */
		if (PlatPtr->tag == tag && PlatPtr->status != in_stasis) {	/* Match? */
			PlatPtr->oldstatus = PlatPtr->status;	/* Save the platform's state */
			PlatPtr->status = in_stasis;	/* Now in stasis */
			ChangeThinkCode(PlatPtr,0);	/* Shut down */
		}
		PlatPtr = PlatPtr->next;	/* Get the next link */
    }
}

/**********************************

	Reset the master plat pointer
	Called from InitThinkers

**********************************/

void ResetPlats(void)
{
	MainPlatPtr = 0;		/* Forget about the linked list */
}
