#include "Doom.h"

Boolean trymove2;		/* Result from P_TryMove2 */
Boolean floatok;		/* if true, move would be ok if within tmfloorz - tmceilingz */
Fixed tmfloorz;		/* Current floor z for P_TryMove2 */
Fixed tmceilingz;		/* Current ceiling z for P_TryMove2 */
mobj_t *movething;		/* Either a skull/missile target or a special pickup */
line_t *blockline;		/* Might be a door that can be opened */

static Fixed		oldx, oldy;
static Fixed		tmbbox[4];
static Word	tmflags;
static Fixed tmdropoffz;	/* Lowest point contacted */
static subsector_t *newsubsec;	/* Dest subsector */


/*
===================
=
= P_TryMove2
=
= Attempt to move to a new position, crossing special lines unless MF_TELEPORT
= is set
=
===================
*/

void P_TryMove2(void)
{
	trymove2 = FALSE;		// until proven otherwise
	floatok = FALSE;

	oldx = tmthing->x;
	oldy = tmthing->y;

	PM_CheckPosition();

	if (checkposonly) {
		checkposonly = FALSE;
		return;
	}

	if (!trymove2)
		return;

	if ( !(tmthing->flags & MF_NOCLIP) ) {
		trymove2 = FALSE;

		if (tmceilingz - tmfloorz < tmthing->height)
			return;			// doesn't fit
		floatok = TRUE;
		if ( !(tmthing->flags&MF_TELEPORT)
			&&tmceilingz - tmthing->z < tmthing->height)
			return;			// mobj must lower itself to fit
		if ( !(tmthing->flags&MF_TELEPORT)
			&& tmfloorz - tmthing->z > 24*FRACUNIT )
			return;			// too big a step up
		if ( !(tmthing->flags&(MF_DROPOFF|MF_FLOAT))
			&& tmfloorz - tmdropoffz > 24*FRACUNIT )
			return;			// don't stand over a dropoff
	}

//
// the move is ok, so link the thing into its new position
//
	UnsetThingPosition(tmthing);

	tmthing->floorz = tmfloorz;
	tmthing->ceilingz = tmceilingz;
	tmthing->x = tmx;
	tmthing->y = tmy;

	SetThingPosition (tmthing);

	trymove2 = TRUE;

	return;
}

static Word PM_CrossCheck(line_t *ld) 
{
	if (PM_BoxCrossLine (ld))	{
		if (!PIT_CheckLine(ld)) {
			return FALSE;
		}
	}
	return TRUE;
}
/*
==================
=
= PM_CheckPosition
=
= This is purely informative, nothing is modified (except things picked up)

in:
tmthing		a mobj_t (can be valid or invalid)
tmx,tmy		a position to be checked (doesn't need relate to the mobj_t->x,y)

out:

newsubsec
floorz
ceilingz
tmdropoffz		the lowest point contacted (monsters won't move to a dropoff)
movething

==================
*/

void PM_CheckPosition (void)
{
	int			xl,xh,yl,yh,bx,by;

	tmflags = tmthing->flags;

	tmbbox[BOXTOP] = tmy + tmthing->radius;
	tmbbox[BOXBOTTOM] = tmy - tmthing->radius;
	tmbbox[BOXRIGHT] = tmx + tmthing->radius;
	tmbbox[BOXLEFT] = tmx - tmthing->radius;

	newsubsec = PointInSubsector(tmx,tmy);

//
// the base floor / ceiling is from the subsector that contains the
// point.  Any contacted lines the step closer together will adjust them
//
	tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
	tmceilingz = newsubsec->sector->ceilingheight;

	++validcount;

	movething = 0;
	blockline = 0;

	if ( tmflags & MF_NOCLIP )
	{
		trymove2 = TRUE;
		return;
	}

//
// check things first, possibly picking things up
// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
// into mapblocks based on their origin point, and can overlap into adjacent
// blocks by up to MAXRADIUS units
//
	xl = (tmbbox[BOXLEFT] - BlockMapOrgX - MAXRADIUS)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - BlockMapOrgX + MAXRADIUS)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - BlockMapOrgY - MAXRADIUS)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - BlockMapOrgY + MAXRADIUS)>>MAPBLOCKSHIFT;

	if (xl<0)
		xl = 0;
	if (yl<0)
		yl = 0;
	if (xh>=BlockMapWidth)
		xh = BlockMapWidth-1;
	if (yh>=BlockMapHeight)
		yh = BlockMapHeight-1;

	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++)
			if (!BlockThingsIterator(bx,by,PIT_CheckThing)) {
				trymove2 = FALSE;
				return;
			}

//
// check lines
//
	xl = (tmbbox[BOXLEFT] - BlockMapOrgX)>>MAPBLOCKSHIFT;
	xh = (tmbbox[BOXRIGHT] - BlockMapOrgX)>>MAPBLOCKSHIFT;
	yl = (tmbbox[BOXBOTTOM] - BlockMapOrgY)>>MAPBLOCKSHIFT;
	yh = (tmbbox[BOXTOP] - BlockMapOrgY)>>MAPBLOCKSHIFT;

	if (xl<0)
		xl = 0;
	if (yl<0)
		yl = 0;
	if (xh>=BlockMapWidth)
		xh = BlockMapWidth-1;
	if (yh>=BlockMapHeight)
		yh = BlockMapHeight-1;

	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++)
			if (!BlockLinesIterator(bx,by,PM_CrossCheck)) {
				trymove2 = FALSE;
				return;
			}

	trymove2 = TRUE;
	return;
}

//=============================================================================


/*
=================
=
= PM_BoxCrossLine
=
=================
*/

Boolean PM_BoxCrossLine (line_t *ld)
{
	Fixed		x1, y1, x2, y2;
	Fixed		lx, ly;
	Fixed		ldx, ldy;
	Fixed		dx1,dy1;
	Fixed		dx2,dy2;
	Boolean		side1, side2;

	if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
	||	tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
	||	tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
	||	tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
		return FALSE;

	y1 = tmbbox[BOXTOP];
	y2 = tmbbox[BOXBOTTOM];

	if (ld->slopetype == ST_POSITIVE) {
		x1 = tmbbox[BOXLEFT];
		x2 = tmbbox[BOXRIGHT];
	} else {
		x1 = tmbbox[BOXRIGHT];
		x2 = tmbbox[BOXLEFT];
	}

	lx = ld->v1.x;
	ly = ld->v1.y;
	ldx = (ld->v2.x-lx)>>16;
	ldy = (ld->v2.y-ly)>>16;

	dx1 = (x1 - lx)>>16;
	dy1 = (y1 - ly)>>16;
	dx2 = (x2 - lx)>>16;
	dy2 = (y2 - ly)>>16;

	side1 = ldy*dx1 < dy1*ldx;
	side2 = ldy*dx2 < dy2*ldx;

	return side1 != side2;
}

//=============================================================================


/*
==================
=
= PIT_CheckLine
=
= Adjusts tmfloorz and tmceilingz as lines are contacted
==================
*/

Boolean PIT_CheckLine (line_t *ld)
{
	Fixed		pm_opentop, pm_openbottom;
	Fixed		pm_lowfloor;
	sector_t	*front, *back;

// a line has been hit

/*
=
= The moving thing's destination position will cross the given line.
= If this should not be allowed, return false.
*/
	if (!ld->backsector)
		return FALSE;		// one sided line

	if (!(tmthing->flags & MF_MISSILE) )
	{
		if ( ld->flags & ML_BLOCKING )
			return FALSE;		// explicitly blocking everything
		if ( !tmthing->player && ld->flags & ML_BLOCKMONSTERS )
			return FALSE;		// block monsters only
	}


	front = ld->frontsector;
	back = ld->backsector;

	if (front->ceilingheight == front->floorheight
	|| back->ceilingheight == back->floorheight)
	{
		blockline = ld;
		return FALSE;			// probably a closed door
	}

	if (front->ceilingheight < back->ceilingheight)
		pm_opentop = front->ceilingheight;
	else
		pm_opentop = back->ceilingheight;
	if (front->floorheight > back->floorheight)
	{
		pm_openbottom = front->floorheight;
		pm_lowfloor = back->floorheight;
	}
	else
	{
		pm_openbottom = back->floorheight;
		pm_lowfloor = front->floorheight;
	}

// adjust floor / ceiling heights
	if (pm_opentop < tmceilingz)
		tmceilingz = pm_opentop;
	if (pm_openbottom > tmfloorz)
		tmfloorz = pm_openbottom;
	if (pm_lowfloor < tmdropoffz)
		tmdropoffz = pm_lowfloor;

	return TRUE;
}

/*
==================
=
= PIT_CheckThing
=
==================
*/

Word PIT_CheckThing (mobj_t *thing)
{
	Fixed		blockdist;
	int			delta;

	if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE) ))
		return TRUE;
	blockdist = thing->radius + tmthing->radius;

	delta = thing->x - tmx;
	if (delta < 0)
		delta = -delta;
	if (delta >= blockdist)
		return TRUE;		// didn't hit it
	delta = thing->y - tmy;
	if (delta < 0)
		delta = -delta;
	if (delta >= blockdist)
		return TRUE;		// didn't hit it

	if (thing == tmthing)
		return TRUE;		// don't clip against self

//
// check for skulls slamming into things
//
	if (tmthing->flags & MF_SKULLFLY)
	{
		movething = thing;
		return FALSE;		// stop moving
	}


//
// missiles can hit other things
//
	if (tmthing->flags & MF_MISSILE)
	{
	// see if it went over / under
		if (tmthing->z > thing->z + thing->height)
			return TRUE;		// overhead
		if (tmthing->z+tmthing->height < thing->z)
			return TRUE;		// underneath
		if (tmthing->target->InfoPtr == thing->InfoPtr)
		{		// don't hit same species as originator
			if (thing == tmthing->target)
				return TRUE;
			if (thing->InfoPtr != &mobjinfo[MT_PLAYER])
				return FALSE;	// explode, but do no damage
			// let players missile other players
		}
		if (! (thing->flags & MF_SHOOTABLE) )
			return !(thing->flags & MF_SOLID);		// didn't do any damage

	// damage / explode
		movething = thing;
		return FALSE;			// don't traverse any more
	}

//
// check for special pickup
//
	if ( (thing->flags&MF_SPECIAL) && (tmflags&MF_PICKUP) )
	{
		movething = thing;
		return TRUE;
	}

	return !(thing->flags & MF_SOLID);
}


