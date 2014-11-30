#include "Doom.h"
#include <String.h>

Word NumFlatAnims;		/* Number of flat anims */
anim_t FlatAnims[] = {
	{rNUKAGE3-rF_START,rNUKAGE1-rF_START,rNUKAGE1-rF_START},
	{rFWATER4-rF_START,rFWATER1-rF_START,rFWATER1-rF_START},
	{rLAVA4-rF_START,rLAVA1-rF_START,rLAVA1-rF_START}
};

static Word numlinespecials;		/* Number of line specials */
static line_t **linespeciallist;	/* Pointer to array of line pointers */

/**********************************

    Init the picture animations for floor textures

**********************************/

void P_InitPicAnims(void)
{
	NumFlatAnims = sizeof(FlatAnims)/sizeof(anim_t);		/* Set the number */
}

/**********************************

	Will return a side_t* given the number of the current sector,
		the line number, and the side (0/1) that you want.

**********************************/

side_t *getSide(sector_t *sec,Word line,Word side)
{
	return (sec->lines[line])->SidePtr[side];
}

/**********************************

	Will return a sector_t* given the number of the current sector,
		the line number and the side (0/1) that you want.

**********************************/

sector_t *getSector(sector_t *sec,Word line,Word side)
{
	return (sec->lines[line])->SidePtr[side]->sector;
}

/**********************************

	Given the sector pointer and the line number, will tell you whether
	the line is two-sided or not.

**********************************/

Boolean twoSided(sector_t *sec,Word line)
{
	if ((sec->lines[line])->flags & ML_TWOSIDED) {
		return TRUE;
	}
	return FALSE;
}

/**********************************

	Return sector_t * of sector next to current. NULL if not two-sided line

**********************************/

sector_t *getNextSector(line_t *line,sector_t *sec)
{
	if (!(line->flags & ML_TWOSIDED)) {
		return 0;
	}

	if (line->frontsector == sec) {		/* Going backwards? */
		return line->backsector;		/* Go the other way */
	}
	return line->frontsector;		/* Use the front */
}

/**********************************

	Find the lowest floor height in surrounding sectors

**********************************/

Fixed P_FindLowestFloorSurrounding(sector_t *sec)
{
	Word i;
	line_t **check;
	sector_t *other;
	Fixed floor;

	floor = sec->floorheight;	/* Get the current floor */
	i = sec->linecount;
	if (i) {		/* Any lines */
		check = sec->lines;	/* Get the first pointer */
		do {
			other = getNextSector(check[0],sec);	/* Connected? */
			if (other) {
				if (other->floorheight < floor) {	/* Check the floor */
					floor = other->floorheight;	/* Lower floor */
				}
			}
			++check;
		} while (--i);
	}
	return floor;		/* Return the height */
}

/**********************************

	Find highest floor height in surrounding sectors

**********************************/

Fixed P_FindHighestFloorSurrounding(sector_t *sec)
{
	Word i;
	line_t **check;
	sector_t *other;
	Fixed floor;

    floor = -500*FRACUNIT;		/* Assume very high */
    i = sec->linecount;			/* Get the count */
    if (i) {
    	check = sec->lines;	/* Init the line pointer */
    	do {
			other = getNextSector(check[0],sec);
			if (other) {
				if (other->floorheight > floor) {
					floor = other->floorheight;	/* Get the new floor */
				}
			}
			++check;
		} while (--i);
	}
	return floor;
}

/**********************************

	Find next highest floor in surrounding sectors

**********************************/

Fixed P_FindNextHighestFloor(sector_t *sec,Fixed currentheight)
{
	Word i;
	line_t **check;
	sector_t *other;
	Fixed height;		/* Value to return */

	height = 0x7FFFFFFF;		/* Init to the maximum Fixed */
	i = sec->linecount;			/* Any lines? */
	if (i) {
		check = sec->lines;		/* Init line pointer */
		do {
			other = getNextSector(check[0],sec);	/* Connecting sector? */
			if (other) {
				if (other->floorheight>currentheight) {	/* Higher than current? */
					if (other->floorheight<height) {	/* Lower than result? */
						height = other->floorheight;	/* Change result */
					}
				}
			}
			++check;
		} while (--i);
	}
    return height;		/* Return the answer */
}

/**********************************

	Find lowest ceiling in the surrounding sectors

**********************************/

Fixed P_FindLowestCeilingSurrounding(sector_t *sec)
{
	Word i;
	line_t **check;
	sector_t *other;
	Fixed height;

	height = 0x7FFFFFFF;		/* Maximum height */
	i = sec->linecount;
	if (i) {
		check = sec->lines;		/* Get the line pointer */
		do {
			other = getNextSector(check[0],sec);	/* Next sector in list */
			if (other) {
				if (other->ceilingheight < height) {	/* Lower? */
					height = other->ceilingheight;	/* Set the new height */
				}
			}
			++check;
		} while (--i);
	}
	return height;
}

/**********************************

	Find highest ceiling in the surrounding sectors

**********************************/

Fixed P_FindHighestCeilingSurrounding(sector_t *sec)
{
	Word i;
	line_t **check;
	sector_t *other;
	Fixed height;

	height = 0;		/* Lowest ceiling possible */
	i = sec->linecount;
	if (i) {
		check = sec->lines;	/* Get pointer to line pointers */
		do {
			other = getNextSector(check[0],sec);
			if (other) {
				if (other->ceilingheight > height) {	/* Higher? */
					height = other->ceilingheight;		/* Save the highest */
				}
			}
			++check;
		} while (--i);
	}
	return height;		/* Return highest */
}

/**********************************

	Return next sector # that line tag refers to

**********************************/

Word P_FindSectorFromLineTag(line_t	*line,Word start)
{
	Word tag;
	sector_t *sec;

	++start;
	if (start<numsectors) {	/* Valid? */
	    tag = line->tag;	/* Cache the tag */
		sec = &sectors[start];	/* Get the sector pointer */
		do {
			if (sec->tag == tag) {	/* Tag match? */
				return start;		/* Return the index */
			}
			++sec;		/* Next pointer */
		} while (++start<numsectors);	/* Keep counting */
	}
	return -1;		/* No good... */
}

/**********************************

	Find minimum light from an adjacent sector

**********************************/

Word P_FindMinSurroundingLight(sector_t *sector,Word max)
{
	Word i;
	Word min;
	line_t **check;
	sector_t *other;

	min = max;		/* Assume answer */
	i = sector->linecount;
	if (i) {			/* Any lines? */
		check = sector->lines;
		do {
			other = getNextSector(check[0],sector);
			if (other) {
				if (other->lightlevel < min) {
					min = other->lightlevel;		/* Get darker */
				}
			}
			++check;
		} while (--i);
	}
	return min;
}

/**********************************

	Called every time a thing origin is about to cross
	a line with a non 0 special

**********************************/

void P_CrossSpecialLine(line_t *line,mobj_t *thing)
{

	/* Triggers that other things can activate */

	if (!thing->player) {	/* Not a player? */
		switch(line->special) {
		default:	/* None of the above? */
			return;	/* Exit */
		case 39:	/* TELEPORT TRIGGER */
		case 97:	/* TELEPORT RETRIGGER */
		case 4:		/* RAISE DOOR */
		case 10:	/* PLAT DOWN-WAIT-UP-STAY TRIGGER */
		case 88:	/* PLAT DOWN-WAIT-UP-STAY RETRIGGER */
			;		/* Null event */
		}
	}

/**********************************

    The first group of triggers all clear line->special
    so that they can't be triggered again.
    The second groun leaves line->special alone so
    triggering can occur at will.

**********************************/

	switch (line->special) {
	case 2:			/* Open Door */
		EV_DoDoor(line,open);
		line->special = 0;
		break;
	case 3:			/* Close Door */
		EV_DoDoor(line,close);
		line->special = 0;
		break;
	case 4:			/* Raise Door */
		EV_DoDoor(line,normaldoor);
		line->special = 0;
		break;
	case 5:			/* Raise Floor */
		EV_DoFloor(line,raiseFloor);
		line->special = 0;
		break;
	case 6:			/* Fast Ceiling Crush & Raise */
		EV_DoCeiling(line,fastCrushAndRaise);
		line->special = 0;
		break;
	case 8:			/* Build Stairs */
		EV_BuildStairs(line);
		line->special = 0;
		break;
	case 10:		/* PlatDownWaitUp */
		EV_DoPlat(line,downWaitUpStay,0);
		line->special = 0;
		break;
	case 12:		/* Light Turn On - brightest near */
		EV_LightTurnOn(line,0);
		line->special = 0;
		break;
	case 13:		/* Light Turn On 255 */
		EV_LightTurnOn(line,255);
		line->special = 0;
		break;
	case 16:		/* Close Door 30 */
		EV_DoDoor(line,close30ThenOpen);
		line->special = 0;
		break;
	case 17:		/* Start Light Strobing */
		EV_StartLightStrobing(line);
		line->special = 0;
		break;
	case 19:		/* Lower Floor */
		EV_DoFloor(line,lowerFloor);
		line->special = 0;
		break;
	case 22:		/* Raise floor to nearest height and change texture */
		EV_DoPlat(line,raiseToNearestAndChange,0);
		line->special = 0;
		break;
	case 25:		/* Ceiling Crush and Raise */
		EV_DoCeiling(line,crushAndRaise);
		line->special = 0;
		break;
	case 30:		/* Raise floor to shortest texture height */
					/* on either side of lines */
		EV_DoFloor(line,raiseToTexture);
		line->special = 0;
		break;
	case 35:		/* Lights Very Dark */
		EV_LightTurnOn(line,35);
		line->special = 0;
		break;
	case 36:		/* Lower Floor (TURBO) */
		EV_DoFloor(line,turboLower);
		line->special = 0;
		break;
	case 37:		/* LowerAndChange */
		EV_DoFloor(line,lowerAndChange);
		line->special = 0;
		break;
	case 38:		/* Lower Floor To Lowest */
		EV_DoFloor(line,lowerFloorToLowest);
		line->special = 0;
		break;
	case 39:		/* TELEPORT! */
		EV_Teleport(line,thing);
		line->special = 0;
		break;
	case 40:		/* RaiseCeilingLowerFloor */
		EV_DoCeiling(line,raiseToHighest);
		EV_DoFloor(line,lowerFloorToLowest);
		line->special = 0;
		break;
	case 44:		/* Ceiling Crush */
		EV_DoCeiling(line,lowerAndCrush);
		line->special = 0;
		break;
	case 52:		/* EXIT! */
		G_ExitLevel();
		line->special = 0;
		break;
	case 53:		/* Perpetual Platform Raise */
		EV_DoPlat(line,perpetualRaise,0);
		line->special = 0;
		break;
	case 54:		/* Platform Stop */
		EV_StopPlat(line);
		line->special = 0;
		break;
	case 56:		/* Raise Floor Crush */
		EV_DoFloor(line,raiseFloorCrush);
		line->special = 0;
		break;
	case 57:		/* Ceiling Crush Stop */
		EV_CeilingCrushStop(line);
		line->special = 0;
		break;
	case 58:		/* Raise Floor 24 */
		EV_DoFloor(line,raiseFloor24);
		line->special = 0;
		break;
	case 59:		/* Raise Floor 24 And Change */
		EV_DoFloor(line,raiseFloor24AndChange);
		line->special = 0;
		break;
	case 104:		/* Turn lights off in sector(tag) */
		EV_TurnTagLightsOff(line);
		line->special = 0;
		break;

	/* These are restartable, do not affect line->special */

	case 72:		/* Ceiling Crush */
		EV_DoCeiling(line,lowerAndCrush);
		break;
	case 73:		/* Ceiling Crush and Raise */
		EV_DoCeiling(line,crushAndRaise);
		break;
	case 74:		/* Ceiling Crush Stop */
		EV_CeilingCrushStop(line);
		break;
	case 75:		/* Close Door */
		EV_DoDoor(line,close);
		break;
	case 76:		/* Close Door 30 */
		EV_DoDoor(line,close30ThenOpen);
		break;
	case 77:		/* Fast Ceiling Crush & Raise */
		EV_DoCeiling(line,fastCrushAndRaise);
		break;
	case 79:		/* Lights Very Dark */
		EV_LightTurnOn(line,35);
		break;
	case 80:		/* Light Turn On - brightest near */
		EV_LightTurnOn(line,0);
		break;
	case 81:		/* Light Turn On 255 */
		EV_LightTurnOn(line,255);
		break;
	case 82:		/* Lower Floor To Lowest */
		EV_DoFloor(line,lowerFloorToLowest);
		break;
	case 83:		/* Lower Floor */
		EV_DoFloor(line,lowerFloor);
		break;
	case 84:		/* LowerAndChange */
		EV_DoFloor(line,lowerAndChange);
		break;
	case 86:		/* Open Door */
		EV_DoDoor(line,open);
		break;
	case 87:		/* Perpetual Platform Raise */
		EV_DoPlat(line,perpetualRaise,0);
		break;
	case 88:		/* PlatDownWaitUp */
		EV_DoPlat(line,downWaitUpStay,0);
		break;
	case 89:		/* Platform Stop */
		EV_StopPlat(line);
		break;
	case 90:		/* Raise Door */
		EV_DoDoor(line,normaldoor);
		break;
	case 91:		/* Raise Floor */
		EV_DoFloor(line,raiseFloor);
		break;
	case 92:		/* Raise Floor 24 */
		EV_DoFloor(line,raiseFloor24);
		break;
	case 93:		/* Raise Floor 24 And Change */
		EV_DoFloor(line,raiseFloor24AndChange);
		break;
	case 94:		/* Raise Floor Crush */
		EV_DoFloor(line,raiseFloorCrush);
		break;
	case 95:		/* Raise floor to nearest height and change texture */
		EV_DoPlat(line,raiseToNearestAndChange,0);
		break;
	case 96:		/* Raise floor to shortest texture height */
					/* on either side of lines */
		EV_DoFloor(line,raiseToTexture);
		break;
	case 97:		/* TELEPORT! */
		EV_Teleport(line,thing);
		break;
	case 98:		/* Lower Floor (TURBO) */
		EV_DoFloor(line,turboLower);
		break;
    }
}


/**********************************

	Called when a thing shoots a special line

**********************************/

void P_ShootSpecialLine(mobj_t *thing,line_t *line)
{
	/* Impacts that other things can activate */

	if (!thing->player) {
		if (line->special!=46) {	/* Open door impact */
			return;
		}
	}

	switch(line->special) {
	case 24:		/* RAISE FLOOR */
		EV_DoFloor(line,raiseFloor);
		P_ChangeSwitchTexture(line,FALSE);
		break;
	case 46:		/* OPEN DOOR */
		EV_DoDoor(line,open);
		P_ChangeSwitchTexture(line,TRUE);
		break;
	case 47:		/* RAISE FLOOR NEAR AND CHANGE */
		EV_DoPlat(line,raiseToNearestAndChange,0);
		P_ChangeSwitchTexture(line,FALSE);
	}
}


/**********************************

	Called every tic frame that the player origin is in a special sector
	Used for radioactive slime.

**********************************/

void PlayerInSpecialSector(player_t *player,sector_t *sector)
{
	Word Damage;

	if (player->mo->z != sector->floorheight) {
		return;		/* not all the way down yet */
	}
	Damage = 0;		/* No damage taken */
	switch (sector->special) {
	case 5:		/* HELLSLIME DAMAGE */
		Damage = 10;
		break;
	case 7:		/* NUKAGE DAMAGE */
		Damage = 5;
		break;
	case 16:	/* SUPER HELLSLIME DAMAGE */
	case 4:		/* STROBE HURT */
		Damage = 20;
		if (GetRandom(255)<5) {		/* Chance it didn't hurt */
			Damage|=0x8000;		/* Suit didn't help! */
		}
		break;
	case 9:		/* Found a secret sector */
		++player->secretcount;
		sector->special = 0;		/* Remove the special */
	}
	if (Damage && Tick1) {		/* Time for pain */
		if (Damage&0x8000 || !player->powers[pw_ironfeet]) {	/* Inflict? */
			DamageMObj(player->mo,0,0,Damage&0x7FFF);
		}
	}
}

/**********************************

	Animate planes, scroll walls, etc

**********************************/

void P_UpdateSpecials(void)
{
	Word i;

	/* Animate flats and textures globaly */

	if (Tick4) {		/* Time yet? */
		anim_t *AnimPtr;
		i = 0;
		AnimPtr = FlatAnims;	/* Index to the flat anims */
		do {
			++AnimPtr->CurrentPic;		/* Next picture index */
			if (AnimPtr->CurrentPic >= AnimPtr->LastPicNum+1) {	/* Off the end? */
				AnimPtr->CurrentPic = AnimPtr->BasePic;		/* Reset the animation */
			}
					/* Set the frame */
			FlatTranslation[AnimPtr->LastPicNum] = FlatInfo[AnimPtr->CurrentPic];
			++AnimPtr;
		} while (++i<NumFlatAnims);
	}

	/* Animate line specials */

	i = numlinespecials;
	if (i) {
		line_t **line;
		line = linespeciallist;
		do {
			line_t *theline;
			theline = line[0];		/* Get the pointer */
			if (theline->special==48) {	/* Effect firstcol scroll */
	                	/* Scroll it */
				theline->SidePtr[0]->textureoffset += FRACUNIT;
			}
			++line;
		} while (--i);
	}
}

/**********************************

	After the map has been loaded, scan for specials that
	spawn thinkers

**********************************/

void SpawnSpecials(void)
{
	sector_t *sector;
	Word i;

	/* Init special SECTORs */

	PurgeLineSpecials();		/* Make SURE they are gone */
	sector = sectors;
	i = 0;
	do {
		switch(sector->special) {
		case 1:		/* FLICKERING LIGHTS */
			P_SpawnLightFlash(sector);
			break;
		case 2:		/* STROBE FAST */
			P_SpawnStrobeFlash(sector,FASTDARK,FALSE);
			break;
		case 3:		/* STROBE SLOW */
			P_SpawnStrobeFlash(sector,SLOWDARK,FALSE);
			break;
		case 8:		/* GLOWING LIGHT */
			P_SpawnGlowingLight(sector);
			break;
		case 9:		/* SECRET SECTOR */
			++SecretsFoundInLevel;
			break;
		case 10:	/* DOOR CLOSE IN 30 SECONDS */
			P_SpawnDoorCloseIn30(sector);
			break;
		case 12:	/* SYNC STROBE SLOW */
			P_SpawnStrobeFlash(sector,SLOWDARK,TRUE);
			break;
		case 13:	/* SYNC STROBE FAST */
			P_SpawnStrobeFlash(sector,FASTDARK,TRUE);
			break;
		case 14:	/* DOOR RAISE IN 5 MINUTES */
			P_SpawnDoorRaiseIn5Mins(sector);
		}
		++sector;
	} while (++i<numsectors);

	/* Init line EFFECTs, first pass, count the effects detected */

	numlinespecials = 0;		/* No specials found */
	i = numlines;			/* Get the line count */
	if (i) {
		line_t *line;
		line = lines;		/* Traverse the list */
		do {
			if (line->special==48) {		/* EFFECT FIRSTCOL SCROLL+ */
				++numlinespecials;		/* Inc the count */
			}
			++line;
		} while (--i);		/* All done? */
	}
	if (numlinespecials) {		/* Any found? */
		line_t *line;
		line_t **linelist;

		/* Get memory for the list */
        linelist = (line_t **)AllocAPointer(sizeof(line_t*)*numlinespecials);
        linespeciallist = linelist;		/* Save the pointer */
		i = numlines;
		line = lines;		/* Reset the count */
		do {
			if (line->special==48) {		/* EFFECT FIRSTCOL SCROLL+ */
				linelist[0] = line;		/* Store the pointer */
				++linelist;
			}
			++line;		/* Next line to scan */
		} while (--i);
	}
}

/**********************************

	Release the memory for line specials

**********************************/

void PurgeLineSpecials(void)
{
	if (linespeciallist) {		/* Is there a valid pointer? */
		DeallocAPointer(linespeciallist);	/* Release it */
		linespeciallist = 0;
		numlinespecials = 0;	/* No lines */
	}
}
