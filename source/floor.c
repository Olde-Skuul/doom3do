#include "Doom.h"

/**********************************

	Local structs

**********************************/

typedef struct {		/* Describe a moving floor */
	sector_t *sector;	/* Sector connected to */
	Fixed floordestheight;	/* Destination height */
	Fixed speed;		/* Speed of motion */
	Word newspecial;	/* Special event number */
	Word texture;		/* Texture when at the bottom */
	int direction;		/* Direction of travel */
	floor_e type;		/* Type of floor */
	Boolean crush;		/* Can it crush you? */
} floormove_t;

#define	FLOORSPEED (FRACUNIT*3)	/* Standard floor motion speed */

/**********************************

	Move a plane (floor or ceiling) and check for crushing
	Called by Doors.c, Plats.c, Ceilng.c and Floor.c
	If ChangeSector returns true, then something is obstructing
	the moving platform and it has to stop or bounce back.

**********************************/

result_e T_MovePlane(sector_t *sector,Fixed speed,
			Fixed dest,Boolean crush,Boolean Ceiling,int direction)
{
	Fixed lastpos;		/* Previous height */

	if (!Ceiling) {
		lastpos = sector->floorheight;		/* Save the previous height */
		if (direction==-1) {			/* Down */
			if ((lastpos - speed) < dest) {	/* Moved too fast? */
				sector->floorheight = dest;	/* Set the new height */
				if (ChangeSector(sector,crush)) {	/* Set the new plane */
					sector->floorheight=lastpos;	/* Restore the height */
					ChangeSector(sector,crush);	/* Put if back */
				}
				return pastdest;		/* Went too far! (Stop motion) */
			} else {
				sector->floorheight = lastpos-speed;	/* Adjust the motion */
				if (ChangeSector(sector,crush)) {	/* Place it */
					sector->floorheight = lastpos;	/* Put it back */
					ChangeSector(sector,crush);	/* Crush... */
					return crushed;		/* Squishy!! */
				}
			}
		} else if (direction==1) {		/* Up */
			if ((lastpos + speed) > dest) {
				sector->floorheight = dest;
				if (ChangeSector(sector,crush)) {	/* Try to move it */
					sector->floorheight = lastpos;	/* Put it back! */
					ChangeSector(sector,crush);
				}
				return pastdest;		/* I got to the end... */
			} else {	/* Could get crushed */
				sector->floorheight = lastpos+speed;
				if (ChangeSector(sector,crush)) {
					if (!crush) {		/* If it can't crush, put it back */
						sector->floorheight = lastpos;	/* Mark */
						ChangeSector(sector,crush);	/* Put it back */
					}
					return crushed;
				}
			}
		}
    } else {		/* Ceiling */
		lastpos = sector->ceilingheight;
		if (direction==-1) {		/* Down */
			if ((lastpos - speed) < dest) {
				sector->ceilingheight = dest;
				if (ChangeSector(sector,crush)) {
					sector->ceilingheight = lastpos;
					ChangeSector(sector,crush);
				}
				return pastdest;
			} else {	/* Could get crushed */
				sector->ceilingheight = lastpos-speed;
				if (ChangeSector(sector,crush)) {
					if (!crush) {
						sector->ceilingheight = lastpos;
						ChangeSector(sector,crush);
					}
					return crushed;
				}
			}
		} else if (direction==1) {		/* Up */
			if ((lastpos + speed) > dest) {		/* Moved too far? */
				sector->ceilingheight = dest;	/* Set the dest */
				if (ChangeSector(sector,crush)) {
					sector->ceilingheight = lastpos;	/* Restore it */
					ChangeSector(sector,crush);
				}
				return pastdest;
			} else {
				sector->ceilingheight = lastpos+speed;	/* Move it */
				ChangeSector(sector,crush);		/* Squish if needed */
			}
		}
	}
	return moveok;
}

/**********************************

	Move a floor to it's destination (Up or down)
	This is a thinker function.

**********************************/

static void T_MoveFloor(floormove_t *floor)
{
	result_e res;

	res = T_MovePlane(floor->sector,floor->speed,	/* Do the move */
			floor->floordestheight,floor->crush,FALSE,floor->direction);
	if (Tick4) {		/* Time for a sound? */
		S_StartSound(&floor->sector->SoundX,sfx_stnmov);
	}
	if (res == pastdest) {		/* Floor reached it's destination? */
		floor->sector->specialdata = 0;	/* Remove the floor */
		if (floor->direction == 1) {	/* Which way? */
			if (floor->type==donutRaise) {
				floor->sector->special = floor->newspecial;	/* Set the special type */
				floor->sector->FloorPic = floor->texture;	/* Set the new texture */
			}
		} else if (floor->direction == -1) {
			if (floor->type==lowerAndChange) {
				floor->sector->special = floor->newspecial;
				floor->sector->FloorPic = floor->texture;
			}
		}
		RemoveThinker(floor);		/* Remove the floor record */
	}
}

/**********************************

	Create a moving floor

**********************************/

Boolean EV_DoFloor(line_t *line,floor_e floortype)
{
	Word secnum;		/* Sector number */
	Boolean rtn;		/* True if I created a floor */
	Word i;
	sector_t *sec;		/* Pointer to sector */
	floormove_t	*floor;	/* Pointer to floor record */

	rtn = FALSE;		/* Assume no entry */
	secnum = -1;
	while ((secnum = P_FindSectorFromLineTag(line,secnum)) !=-1) {
		sec = &sectors[secnum];		/* Get pointer to sector */

		/* Already moving?  If so, keep going... */
		if (sec->specialdata) {		/* Already has a floor attached? */
			continue;
		}

		/* New floor thinker */

		rtn = TRUE;		/* I created a floor */
		floor = (floormove_t *)AddThinker(T_MoveFloor,sizeof(floormove_t));
		sec->specialdata = floor;		/* Mark the sector */
		floor->type = floortype;		/* Save the type of floor */
		floor->crush = FALSE;			/* Assume it can't crush */
		floor->sector = sec;			/* Set the current sector */
		floor->speed = FLOORSPEED;		/* Assume normal speed */

		switch(floortype) {				/* Handle all the special cases */
		case lowerFloor:
			floor->floordestheight = P_FindHighestFloorSurrounding(sec);
			floor->direction = -1;		/* Go down */
			break;
		case lowerFloorToLowest:
			floor->floordestheight = P_FindLowestFloorSurrounding(sec);
			floor->direction = -1;		/* Go down */
			break;
		case turboLower:
			floor->floordestheight = (8*FRACUNIT) +	P_FindHighestFloorSurrounding(sec);
			floor->speed = FLOORSPEED * 4;	/* Fast speed */
			floor->direction = -1;		/* Go down */
			break;
		case raiseFloorCrush:
			floor->crush = TRUE;		/* Enable crushing */
		case raiseFloor:
			floor->direction = 1;		/* Go up */
			floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
			if (floor->floordestheight > sec->ceilingheight) {	/* Too high? */
				floor->floordestheight = sec->ceilingheight;	/* Set maximum */
			}
			break;
		case raiseFloorToNearest:
			floor->direction = 1;		/* Go up */
			floor->floordestheight = P_FindNextHighestFloor(sec,sec->floorheight);
			break;
		case raiseFloor24AndChange:	/* Raise 24 pixels and change texture */
			sec->FloorPic = line->frontsector->FloorPic;
			sec->special = line->frontsector->special;
		case raiseFloor24:			/* Just raise 24 pixels */
			floor->direction = 1;	/* Go up */
			floor->floordestheight = floor->sector->floorheight + (24<<FRACBITS);
			break;
		case raiseToTexture:
			{
			Word minsize;
			side_t *side;
			Word Height;

			floor->direction = 1;
			i = 0;
			minsize = 32767U;		/* Maximum height */
			while (i<sec->linecount) {
				if (twoSided(sec,i)) {		/* Only process two sided lines */
					side = getSide(sec,i,0);		/* Get the first side */
					if (!(side->bottomtexture&0x8000)) {	/* Valid texture? */
						Height = TextureInfo[side->bottomtexture].height;
						if (Height < minsize) {
							minsize = Height;
						}
					}
					side = getSide(sec,i,1);		/* Get the second side */
					if (!(side->bottomtexture&0x8000)) {	/* Valid texture? */
						Height = TextureInfo[side->bottomtexture].height;
						if (Height < minsize) {
							minsize = Height;
						}
					}
				}
				++i;		/* Next count */
			}
			floor->floordestheight = floor->sector->floorheight + ((Fixed)minsize<<FRACBITS);	/* Set the height */
			}
			break;
		case lowerAndChange:
			floor->direction = -1;
			floor->floordestheight = P_FindLowestFloorSurrounding(sec);
			floor->texture = sec->FloorPic;
			i = 0;
			while (i<sec->linecount) {
				if (twoSided(sec,i)) {	/* Only process two sided lines */
					if (getSide(sec,i,0)->sector == sec) {
						sec = getSector(sec,i,1);	/* Get the opposite side */
					} else {
						sec = getSector(sec,i,0);
					}
					floor->texture = sec->FloorPic;	/* Get the texture */
					floor->newspecial = sec->special;
					break;
				}
				++i;
			}
		}
	}
	return rtn;
}

/**********************************

	Build a staircase!

**********************************/

Boolean EV_BuildStairs(line_t *line)
{
	Boolean Stay;		/* Flag to break the stair building loop */
	Boolean rtn;	/* Return value */
	Word secnum;	/* Sector number found */
	floormove_t	*floor;	/* Pointer to new floor struct */
	Fixed height;	/* Height of new floor */
	sector_t *sec;	/* Current sector */
	sector_t *tsec;	/* Working sector */
	Word texture;	/* Texture to attach to the stairs */
	Word i;			/* Temp */

	rtn = FALSE;		/* Assume no thinkers made */
	secnum = -1;
	while ((secnum = P_FindSectorFromLineTag(line,secnum)) != -1) {
		sec = &sectors[secnum];		/* Get the base sector pointer */

		/* Already moving? If so, try another one */
		if (sec->specialdata) {
			continue;
		}

		/* New floor thinker */

		rtn = TRUE;
		height = sec->floorheight + (8<<FRACBITS);	/* Go up 8 pixels */
		floor = (floormove_t *)AddThinker(T_MoveFloor,sizeof(floormove_t));
		sec->specialdata = floor;		/* Attach the record */
		floor->direction = 1;		/* Move up */
		floor->sector = sec;		/* Set the proper sector */
		floor->speed = FLOORSPEED/2;	/* Normal speed */
		floor->floordestheight = height;	/* Set the new height */
		texture = sec->FloorPic;		/* Cache the texture for the stairs */

		/* Find next sector to raise */
		/* 1. Find 2-sided line with same sector side[0] */
		/* 2. Other side is the next sector to raise */

		do {
			Stay = FALSE;			/* Assume I fall out */
			for (i=0;i<sec->linecount;++i) {
				if (!twoSided(sec,i)) {		/* Two sided line? */
					continue;
				}

				tsec = (sec->lines[i])->frontsector;	/* Is this the sector? */
				if (sec != tsec) {	/* Not a match! */
					continue;
				}
				tsec = (sec->lines[i])->backsector;	/* Get the possible dest */
				if (tsec->FloorPic != texture) {	/* Not the same texture? */
					continue;
				}
				height += (8<<FRACBITS);		/* Increase the height */
				if (tsec->specialdata) {		/* Busy already? */
					continue;
				}
				sec = tsec;			/* I continue from here */
				floor = (floormove_t *)AddThinker(T_MoveFloor,sizeof(floormove_t));
				sec->specialdata = floor;	/* Attach this floor */
				floor->direction = 1;	/* Go up */
				floor->sector = sec;	/* Set the sector I will affect */
				floor->speed = FLOORSPEED/2;	/* Slow speed */
				floor->floordestheight = height;
				Stay = TRUE;			/* I linked to a sector */
				break;			/* Restart the loop */
			}
		} while (Stay);
	}
	return rtn;		/* Did I do it? */
}

/**********************************

	Create a moving floor in the form of a donut

**********************************/

Boolean EV_DoDonut(line_t *line)
{
	sector_t *s1;
	sector_t *s2;
	sector_t *s3;
	Word secnum;
	Boolean rtn;
	Word i;
	floormove_t *floor;

	secnum = -1;
	rtn = FALSE;
	while ((secnum = P_FindSectorFromLineTag(line,secnum)) != -1) {
		s1 = &sectors[secnum];

		/* ALREADY MOVING?  IF SO, KEEP GOING... */
		if (s1->specialdata) {
			continue;
		}
		rtn = TRUE;
		s2 = getNextSector(s1->lines[0],s1);
		i = 0;
		do {
			if (!(s2->lines[i]->flags & ML_TWOSIDED) ||
				(s2->lines[i]->backsector == s1)) {
				continue;
			}
			s3 = s2->lines[i]->backsector;	/* Get the back sector */

			/* Spawn rising slime */

			floor = (floormove_t *)AddThinker(T_MoveFloor,sizeof(floormove_t));
			s2->specialdata = floor;
			floor->type = donutRaise;
			floor->crush = FALSE;
			floor->direction = 1;	/* Going up */
			floor->sector = s2;
			floor->speed = FLOORSPEED / 2;
			floor->texture = s3->FloorPic;
			floor->newspecial = 0;
			floor->floordestheight = s3->floorheight;

			/* Spawn lowering donut-hole */

			floor = (floormove_t *)AddThinker(T_MoveFloor,sizeof(floormove_t));
			s1->specialdata = floor;
			floor->type = lowerFloor;
			floor->crush = FALSE;
			floor->direction = -1;	/* Going down */
			floor->sector = s1;
			floor->speed = FLOORSPEED / 2;
			floor->floordestheight = s3->floorheight;
			break;
		} while (++i<s2->linecount);
	}
	return rtn;
}
