#include "Doom.h"

static mobj_t *CheckThingMo;		/* Used for PB_CheckThing */
static Fixed testx, testy;
static Fixed testfloorz, testceilingz, testdropoffz;
static subsector_t *testsubsec;
static line_t *ceilingline;

static mobj_t *hitthing;
static Fixed testbbox[4];		/* Bounding box for tests */
static int testflags;

/**********************************

	Float up or down at a set speed, used by flying monsters
	
**********************************/

static void FloatChange(mobj_t *mo)
{
	mobj_t *Target;
	Fixed dist, delta;
	
	Target = mo->target;		/* Get the target object */	
	delta = (Target->z + (mo->height>>1)) - mo->z;	/* Get the height differance */

	dist = GetApproxDistance(Target->x - mo->x,Target->y - mo->y);	/* Distance to target */
	delta = delta*3;		/* Mul by 3 for a fudge factor */
	
	if (delta<0) {		/* Delta is signed... */
		if (dist < (-delta) ) {		/* Negate */
			mo->z -= FLOATSPEED;	/* Adjust the height */
		}
		return;
	}	 
	if (dist < delta) {		/* Normal compare */
		mo->z += FLOATSPEED;	/* Adjust the height */
	}
}


/**********************************

	Move a critter in the Z axis
	
**********************************/

static Word P_ZMovement(mobj_t *mo)
{

	mo->z += mo->momz;		/* Basic z motion */

	if ( (mo->flags & MF_FLOAT) && mo->target) {	/* float down towards target if too close */
		FloatChange(mo);
	}

//
// clip movement
//
	if (mo->z <= mo->floorz) {	// hit the floor
		if (mo->momz < 0) {
			mo->momz = 0;
		}
		mo->z = mo->floorz;
		if (mo->flags & MF_MISSILE) {
			ExplodeMissile(mo);
			return TRUE;
 		}
	} else if (! (mo->flags & MF_NOGRAVITY) ) {
		if (!mo->momz) {		/* Not falling */
			mo->momz = -GRAVITY*2;	/* Fall faster */
		} else {
			mo->momz -= GRAVITY;
		}
	}

	if (mo->z + mo->height > mo->ceilingz) {	// hit the ceiling
		if (mo->momz > 0) {
			mo->momz = 0;
		}
		mo->z = mo->ceilingz - mo->height;
		if (mo->flags & MF_MISSILE) {
			ExplodeMissile(mo);
			return TRUE;
		}
	}
	return FALSE;
}


/*
=================
=
= PB_BoxCrossLine
=
=================
*/

static Boolean PB_BoxCrossLine(line_t *ld)
{
	Fixed x1, x2;
	Fixed lx, ly;
	Fixed ldx, ldy;
	Fixed dx1,dy1;
	Fixed dx2,dy2;
	Boolean side1, side2;

	if (testbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
	||	testbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
	||	testbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
	||	testbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
		return FALSE;

	if (ld->slopetype == ST_POSITIVE) {
		x1 = testbbox[BOXLEFT];
		x2 = testbbox[BOXRIGHT];
	} else {
		x1 = testbbox[BOXRIGHT];
		x2 = testbbox[BOXLEFT];
	}

	lx = ld->v1.x;
	ly = ld->v1.y;
	ldx = (ld->v2.x-ld->v1.x)>>16;
	ldy = (ld->v2.y-ld->v1.y)>>16;

	dx1 = (x1 - lx)>>16;
	dy1 = (testbbox[BOXTOP] - ly)>>16;
	dx2 = (x2 - lx)>>16;
	dy2 = (testbbox[BOXBOTTOM] - ly)>>16;

	side1 = ldy*dx1 < dy1*ldx;
	side2 = ldy*dx2 < dy2*ldx;

	return side1 != side2;
}


/*
==================
=
= PB_CheckLine
=
= Adjusts testfloorz and testceilingz as lines are contacted
==================
*/

static Boolean PB_CheckLine (line_t *ld)
{
	Fixed opentop, openbottom;
	Fixed lowfloor;
	sector_t	*front, *back;

/*
=
= The moving thing's destination position will cross the given line.
= If this should not be allowed, return FALSE.
*/
	if (!ld->backsector)
		return FALSE;		// one sided line

	if ( !(testflags & MF_MISSILE)
	&& (ld->flags & (ML_BLOCKING | ML_BLOCKMONSTERS) ) )
			return FALSE;		// explicitly blocking


	front = ld->frontsector;
	back = ld->backsector;

	if (front->ceilingheight < back->ceilingheight)
		opentop = front->ceilingheight;
	else
		opentop = back->ceilingheight;
	if (front->floorheight > back->floorheight)
	{
		openbottom = front->floorheight;
		lowfloor = back->floorheight;
	}
	else
	{
		openbottom = back->floorheight;
		lowfloor = front->floorheight;
	}

// adjust floor / ceiling heights
	if (opentop < testceilingz)
	{
		testceilingz = opentop;
		ceilingline = ld;
	}
	if (openbottom > testfloorz)
		testfloorz = openbottom;
	if (lowfloor < testdropoffz)
		testdropoffz = lowfloor;

	return TRUE;
}

static Word PB_CrossCheck(line_t *ld) 
{
	if (PB_BoxCrossLine(ld)) {
		if (!PB_CheckLine(ld)) {
			return FALSE;
		}
	}
	return TRUE;
}




/*
==================
=
= PB_CheckThing
=
==================
*/

static Word PB_CheckThing (mobj_t *thing)
{
	Fixed		blockdist;
	int			delta;
	mobj_t *mo;

	if (!(thing->flags & MF_SOLID )) {
		return TRUE;
	}
	mo = CheckThingMo;
	blockdist = thing->radius + mo->radius;

	delta = thing->x - testx;
	if (delta < 0)
		delta = -delta;
	if (delta >= blockdist)
		return TRUE;		// didn't hit it
	delta = thing->y - testy;
	if (delta < 0)
		delta = -delta;
	if (delta >= blockdist)
		return TRUE;		// didn't hit it

	if (thing == mo)
		return TRUE;		// don't clip against self

//
// check for skulls slamming into things
//
	if (testflags & MF_SKULLFLY) {
		hitthing = thing;
		return FALSE;		// stop moving
	}


//
// missiles can hit other things
//
	if (testflags & MF_MISSILE) {
	// see if it went over / under
		if (mo->z > thing->z + thing->height)
			return TRUE;		// overhead
		if (mo->z+mo->height < thing->z)
			return TRUE;		// underneath
		if (mo->target->InfoPtr == thing->InfoPtr) {	// don't hit same species as originator
			if (thing == mo->target)
				return TRUE;	// don't explode on shooter
			if (thing->InfoPtr != &mobjinfo[MT_PLAYER])
				return FALSE;	// explode, but do no damage
			// let players missile other players
		}
		if (! (thing->flags & MF_SHOOTABLE) )
			return !(thing->flags & MF_SOLID);		// didn't do any damage

	// damage / explode
		hitthing = thing;
		return FALSE;			// don't traverse any more
	}

	return !(thing->flags & MF_SOLID);
}

/*
==================
=
= PB_CheckPosition
=
= This is purely informative, nothing is modified (except things picked up)

in:
basething		a mobj_t
testx,testy		a position to be checked (doesn't need relate to the mobj_t->x,y)

out:

testsubsec
floorz
ceilingz
testdropoffz		the lowest point contacted (monsters won't move to a dropoff)
hitthing

==================
*/

static Word PB_CheckPosition(mobj_t *mo)
{
	int			xl,xh,yl,yh,bx,by;
	int			r;

	testflags = mo->flags;

	r = mo->radius;
	testbbox[BOXTOP] = testy + r;
	testbbox[BOXBOTTOM] = testy - r;
	testbbox[BOXRIGHT] = testx + r;
	testbbox[BOXLEFT] = testx - r;

//
// the base floor / ceiling is from the subsector that contains the
// point.  Any contacted lines the step closer together will adjust them
//
	testsubsec = PointInSubsector(testx,testy);
	testfloorz = testdropoffz = testsubsec->sector->floorheight;
	testceilingz = testsubsec->sector->ceilingheight;

	++validcount;

	ceilingline = 0;
	hitthing = 0;

//
// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
// into mapblocks based on their origin point, and can overlap into adjacent
// blocks by up to MAXRADIUS units
//
	xl = (testbbox[BOXLEFT] - BlockMapOrgX - MAXRADIUS)>>MAPBLOCKSHIFT;
	xh = (testbbox[BOXRIGHT] - BlockMapOrgX + MAXRADIUS)>>MAPBLOCKSHIFT;
	yl = (testbbox[BOXBOTTOM] - BlockMapOrgY - MAXRADIUS)>>MAPBLOCKSHIFT;
	yh = (testbbox[BOXTOP] - BlockMapOrgY + MAXRADIUS)>>MAPBLOCKSHIFT;

	if (xl<0) {
		xl = 0;
	}
	if (yl<0) {
		yl = 0;
	}
	if (xh>=BlockMapWidth) {
		xh = BlockMapWidth-1;
	}
	if (yh>=BlockMapHeight) {
		yh = BlockMapHeight-1;
	}

	CheckThingMo = mo;			/* Store for PB_CheckThing */
	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++) {
			if (!BlockThingsIterator(bx,by,PB_CheckThing))
				return FALSE;
			if (!BlockLinesIterator(bx,by,PB_CrossCheck))
				return FALSE;
		}

	return TRUE;
}



/*
===================
=
= PB_TryMove
=
= Attempt to move to a new position
=
===================
*/

static Boolean PB_TryMove(int tryx,int tryy,mobj_t *mo)
{
	testx = tryx;
	testy = tryy;

	if (!PB_CheckPosition(mo))
		return FALSE;		// solid wall or thing

	if (testceilingz - testfloorz < mo->height)
		return FALSE;			// doesn't fit
	if ( testceilingz - mo->z < mo->height)
		return FALSE;			// mobj must lower itself to fit
	if ( testfloorz - mo->z > 24*FRACUNIT )
		return FALSE;			// too big a step up
	if ( !(testflags&(MF_DROPOFF|MF_FLOAT))
		&& testfloorz - testdropoffz > 24*FRACUNIT )
		return FALSE;			// don't stand over a dropoff

//
// the move is ok, so link the thing into its new position
//

	UnsetThingPosition(mo);
	mo->floorz = testfloorz;
	mo->ceilingz = testceilingz;
	mo->x = tryx;
	mo->y = tryy;
	SetThingPosition(mo);
	return TRUE;
}

/*
===================
=
= P_XYMovement
=
===================
*/

#define	STOPSPEED		0x1000
#define	FRICTION		0xd240

static Word P_XYMovement (mobj_t *mo)
{
	Fixed xleft, yleft;
	Fixed xuse, yuse;

//
// cut the move into chunks if too large
//

	xleft = xuse = mo->momx & ~7;
	yleft = yuse = mo->momy & ~7;

	while (xuse > MAXMOVE || xuse < -MAXMOVE
	|| yuse > MAXMOVE || yuse < -MAXMOVE)
	{
		xuse >>= 1;
		yuse >>= 1;
	}

	while (xleft || yleft) {
		xleft -= xuse;
		yleft -= yuse;
		if (!PB_TryMove(mo->x + xuse, mo->y + yuse,mo)) {
		// blocked move
			if (mo->flags & MF_SKULLFLY) {
				L_SkullBash(mo,hitthing);
				return TRUE;
			}

			if (mo->flags & MF_MISSILE) {	// explode a missile
				if (ceilingline && ceilingline->backsector &&
					ceilingline->backsector->CeilingPic==-1) {	// hack to prevent missiles exploding against the sky
					P_RemoveMobj(mo);
					return TRUE;
				}
				L_MissileHit(mo,hitthing);
				return TRUE;
			}

			mo->momx = mo->momy = 0;
			return FALSE;
		}
	}

//
// slow down
//
	if (mo->flags & (MF_MISSILE | MF_SKULLFLY ) )
		return FALSE;		// no friction for missiles ever

	if (mo->z > mo->floorz)
		return FALSE;		// no friction when airborne

	if (mo->flags & MF_CORPSE)
		if (mo->floorz != mo->subsector->sector->floorheight)
			return FALSE;			// don't stop halfway off a step

	if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED
	&& mo->momy > -STOPSPEED && mo->momy < STOPSPEED) {
		mo->momx = 0;
		mo->momy = 0;
	} else {
		mo->momx = (mo->momx>>8)*(FRICTION>>8);
		mo->momy = (mo->momy>>8)*(FRICTION>>8);
	}
	return FALSE;
}


/**********************************

	Process all the critter logic
	
**********************************/

static void P_MobjThinker(mobj_t *mobj)
{
	Word Time;
	Word SightFlag;

	if (mobj->momx || mobj->momy) {		/* Any horizontal momentum? */
		if (P_XYMovement(mobj)) {	/* Move it */
			return;			/* Exit if state change */
		}
	}

	if ( (mobj->z != mobj->floorz) || mobj->momz) {	/* Any z momentum? */
		if (P_ZMovement(mobj)) {		/* Move it (Or fall) */
			return;			/* Exit if state change */
		}
	}

	Time = ElapsedTime;
	SightFlag = TRUE;
	do {
		if (mobj->tics == -1) {		/* Never time out? */
			return;
		}
		if (mobj->tics>Time) {
			mobj->tics-=Time;	/* Count down */
			return;
		}
		if (SightFlag && mobj->flags & MF_COUNTKILL) {	/* Can it be killed? */
			SightFlag = FALSE;
			mobj->flags &= ~MF_SEETARGET;	/* Assume I don't see a target */
			if (mobj->target) {	/* Do I have a target? */
				if (CheckSight(mobj,mobj->target)) {
					mobj->flags |= MF_SEETARGET;
				}
			}
		}
		Time-=mobj->tics;
		if (!SetMObjState(mobj,mobj->state->nextstate)) {	/* Next object state */
			return;
		}
	} while (Time);
}


/**********************************

	Execute base think logic for the critters every tic
	
**********************************/

void P_RunMobjBase(void)
{
	mobj_t *mo;
	mo = mobjhead.next;
	if (mo!=&mobjhead) {
		do {	/* End of the list? */
			mobj_t *NextMo;		/* In case it's deleted! */
			NextMo = mo->next;
			if (!mo->player) {		/* Don't handle players */
				P_MobjThinker(mo);	/* Execute the code */
			}
			mo = NextMo;			/* Next in the list */
		} while (mo!=&mobjhead);
	}
}

