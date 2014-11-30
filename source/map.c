#include "Doom.h"

mobj_t *linetarget;			/* Object that was targeted */
mobj_t *tmthing;			/* mobj_t to be checked */
Fixed tmx,tmy;			/* Temp x,y for a position to be checked */
Boolean checkposonly;		/* If true, just check the position, no actions */
mobj_t *shooter;			/* Source of a direct line shot */
angle_t attackangle;		/* Angle to target */
Fixed attackrange;		/* Range to target */
Fixed aimtopslope;		/* Range of slope to target weapon */
Fixed aimbottomslope;

static int usebbox[4];		/* Local box for BSP traversing */
static vector_t useline;	/* Temp subdivided line for targeting */
static line_t *closeline;	/* Line to target */
static Fixed closedist;	/* Distance to target */
static mobj_t *bombsource;	/* Explosion source */
static mobj_t *bombspot;	/* Explosion position */
static Word bombdamage;		/* Damage done by explosion */

/**********************************

Input:
tmthing	a mobj_t (can be valid or invalid)
tmx,tmy	a position to be checked (doesn't need relate to the mobj_t->x,y)

Output:
newsubsec	Subsector of the new position
floatok		if true, move would be ok if within tmfloorz - tmceilingz
floorz		New floor z
ceilingz	New ceiling z
tmdropoffz	the lowest point contacted (monsters won't move to a dropoff)
movething	thing collision

**********************************/

Boolean P_CheckPosition(mobj_t *thing, Fixed x, Fixed y)
{
	tmthing = thing;		/* Copy parms to globals */
	tmx = x;
	tmy = y;
	checkposonly = TRUE;		/* Only check the position */
	P_TryMove2();			/* See if I can move there... */
	return trymove2;		/* Return the result */
}

/**********************************

	Try to move to a new position and trigger special events

**********************************/

Boolean P_TryMove(mobj_t *thing, Fixed x, Fixed y)
{
	Word damage;
	mobj_t *latchedmovething;

	tmthing = thing;		/* Source xy */
	tmx = x;				/* New x,y */
	tmy = y;
	P_TryMove2();			/* Move to the new spot */

/* Pick up the specials */

	latchedmovething = movething;		/* Hit something? */

	if (latchedmovething) {
		/* missile bash into a monster */
		if (thing->flags & MF_MISSILE) {
			damage = (GetRandom(7)+1)*thing->InfoPtr->damage;	/* Ouch! */
			DamageMObj(latchedmovething,thing,thing->target,damage);
		/* skull bash into a monster */
		} else if (thing->flags & MF_SKULLFLY) {
			damage = (GetRandom(7)+1)*thing->InfoPtr->damage;
			DamageMObj(latchedmovething,thing,thing,damage);
			thing->flags &= ~MF_SKULLFLY;		/* Stop the skull from flying */
			thing->momx = thing->momy = thing->momz = 0;	/* No momentum */
			SetMObjState(thing,thing->InfoPtr->spawnstate);	/* Reset state */
		/* Try to pick it up */
		} else {
			TouchSpecialThing(latchedmovething,thing);
		}
	}
	return trymove2;		/* Return result */
}

/**********************************

	Routine used by BlockLinesIterator to check for
	line collision. I always return TRUE to check ALL lines

**********************************/

static Word PIT_UseLines(line_t *li)
{
	vector_t dl;
	Fixed frac;

/* check bounding box first */

	if (usebbox[BOXRIGHT] <= li->bbox[BOXLEFT]	/* Within the bounding box? */
	||	usebbox[BOXLEFT] >= li->bbox[BOXRIGHT]
	||	usebbox[BOXTOP] <= li->bbox[BOXBOTTOM]
	||	usebbox[BOXBOTTOM] >= li->bbox[BOXTOP] ) {
		return TRUE;			/* Nope, they don't collide */
	}

/* find distance along usetrace */

	MakeVector(li,&dl);			/* Convert true line to a divline struct */
	frac = InterceptVector(&useline,&dl);		/* How much do they intercept */
	if ((frac < 0) ||			/* Behind source? */
		(frac > closedist)) {	/* Too far away? */
		return TRUE;		/* No collision */
	}

/* The line is actually hit, find the distance */

	if (!li->special) {		/* Not a special line? */
		if (LineOpening(li)) {	/* See if it passes through */
			return TRUE;	/* keep going */
		}
	}
	closeline = li;		/* This is the line of travel */
	closedist = frac;	/* This is the length of the line */

	return TRUE;		/* Can't use for than one special line in a row */
}

/**********************************

	Looks for special lines in front of the player to activate
	Used when the player presses "Use" to open a door or such

**********************************/

void P_UseLines (player_t *player)
{
	Word angle;
	Fixed x1,y1,x2,y2;
	int	x,y,xl,xh,yl,yh;

	angle = player->mo->angle >> ANGLETOFINESHIFT;
	x1 = player->mo->x;		/* Get the source x,y */
	y1 = player->mo->y;
	x2 = x1 + (USERANGE>>FRACBITS)*finecosine[angle];	/* Get the dest X,Y */
	y2 = y1 + (USERANGE>>FRACBITS)*finesine[angle];

	useline.x = x1;		/* Create the useline record */
	useline.y = y1;
	useline.dx = x2-x1;	/* Delta x and y */
	useline.dy = y2-y1;

	if (useline.dx >= 0) {
		usebbox[BOXRIGHT] = x2;		/* Create the bounding box */
		usebbox[BOXLEFT] = x1;
	} else {
		usebbox[BOXRIGHT] = x1;
		usebbox[BOXLEFT] = x2;
	}

	if (useline.dy >= 0) {		/* Create the bounding box */
		usebbox[BOXTOP] = y2;
		usebbox[BOXBOTTOM] = y1;
	} else {
		usebbox[BOXTOP] = y1;
		usebbox[BOXBOTTOM] = y2;
	}

	yh = (usebbox[BOXTOP] - BlockMapOrgY)>>MAPBLOCKSHIFT;	/* Bounding box */
	yl = (usebbox[BOXBOTTOM] - BlockMapOrgY)>>MAPBLOCKSHIFT;
	xh = (usebbox[BOXRIGHT] - BlockMapOrgX)>>MAPBLOCKSHIFT;
	xl = (usebbox[BOXLEFT] - BlockMapOrgX)>>MAPBLOCKSHIFT;
	++xh;			/* For < compare later */
	++yh;

	closeline = 0;		/* No line found */
	closedist = FRACUNIT;	/* 1.0 units distance */
	++validcount;		/* Make unique sector mark */

	y = yl;
	do {
		x = xl;
		do {
			BlockLinesIterator(x,y,PIT_UseLines);		/* Check the lines */
		} while (++x<xh);
	} while (++y<yh);

/* check closest line */

	if (closeline) {	/* Line nearby? */
		if (!closeline->special) {		/* Is it special? */
			S_StartSound(&player->mo->x,sfx_noway);	/* Make the grunt sound */
		} else {
			P_UseSpecialLine (player->mo, closeline);	/* Activate the special */
		}
	}
}

/**********************************

	Routine used by BlockThingsIterator to check for
	damage to anyone within range of an explosion.
	Source is the creature that caused the explosion at spot
	Always return TRUE to check ALL lines

**********************************/

static Word PIT_RadiusAttack(mobj_t *thing)
{
	Fixed dx,dy,dist;

	if (!(thing->flags & MF_SHOOTABLE) ) {		/* Can this item be hit? */
		return TRUE;			/* Next thing... */
	}

	dx = abs(thing->x - bombspot->x);		/* Absolute distance from BOOM */
	dy = abs(thing->y - bombspot->y);
	dist = dx>=dy ? dx : dy;		/* Get the greater of the two */
	dist = (dist - thing->radius) >> FRACBITS;
	if (dist < 0) {		/* Within the blast? */
		dist = 0;		/* Fix the distance */
	}
	if (dist < bombdamage) {		/* Within blast range? */
		DamageMObj(thing,bombspot,bombsource,bombdamage-dist);
	}
	return TRUE;		/* Continue */
}

/**********************************

	Inflict damage to all items within blast range.
	Source is the creature that casued the explosion at spot

**********************************/

void RadiusAttack(mobj_t *spot,mobj_t *source,Word damage)
{
	Word x,y,xl,xh,yl,yh;
	Fixed dist;

	dist = damage<<FRACBITS;		/* Convert to fixed */
	yh = (spot->y + dist - BlockMapOrgY)>>MAPBLOCKSHIFT;
	yl = (spot->y - dist - BlockMapOrgY)>>MAPBLOCKSHIFT;
	xh = (spot->x + dist - BlockMapOrgX)>>MAPBLOCKSHIFT;
	xl = (spot->x - dist - BlockMapOrgX)>>MAPBLOCKSHIFT;
	++xh;
	++yh;
	bombspot = spot;		/* Copy to globals so PIT_Radius can see it */
	bombsource = source;
	bombdamage = damage;

	y = yl;
	do {
		x=xl;
		do {		/* Damage all things in collision range */
			BlockThingsIterator(x,y,PIT_RadiusAttack);
		} while (++x<xh);
	} while (++y<yh);
}

/**********************************

	Choose a target based of direction of facing.
	A line will be traced from the middle of shooter in the direction of
	attackangle until either a shootable mobj is within the visible
	aimtopslope / aimbottomslope range, or a solid wall blocks further
	tracing.  If no thing is targeted along the entire range, the first line
	that blocks the midpoint of the trace will be hit.

**********************************/

Fixed AimLineAttack(mobj_t *t1,angle_t angle,Fixed distance)
{
	shooter = t1;
	attackrange = distance;
	attackangle = angle;
	aimtopslope = 100*FRACUNIT/160;	/* Can't shoot outside view angles */
	aimbottomslope = -100*FRACUNIT/160;

	++validcount;

	P_Shoot2();			/* Call other code */
	linetarget = shootmobj;
	if (linetarget) {		/* Was there a valid hit? */
		return shootslope;	/* Return the slope of target */
	}
	return 0;		/* No target */
}

/**********************************

	Actually perform an attack based off of the direction facing.
	Use by pistol, chain gun and shotguns.
	If slope == MAXINT, use screen bounds for attacking

**********************************/

void LineAttack(mobj_t *t1,angle_t angle,Fixed distance,Fixed slope,Word damage)
{
	line_t *shootline2;
	int	shootx2, shooty2, shootz2;

	shooter = t1;
	attackrange = distance;
	attackangle = angle;

	if (slope == MAXINT) {
		aimtopslope = 100*FRACUNIT/160;	/* can't shoot outside view angles */
		aimbottomslope = -100*FRACUNIT/160;
	} else {
		aimtopslope = slope+1;
		aimbottomslope = slope-1;
	}
	++validcount;
	P_Shoot2();			/* Perform the calculations */
	linetarget = shootmobj;		/* Get the result */
	shootline2 = shootline;
	shootx2 = shootx;
	shooty2 = shooty;
	shootz2 = shootz;

/* Shoot thing */
	if (linetarget) {			/* Did you hit? */
		if (linetarget->flags & MF_NOBLOOD) {
			P_SpawnPuff(shootx2,shooty2,shootz2);	/* Make a spark on the target */
		} else {
			P_SpawnBlood(shootx2,shooty2,shootz2,damage);	/* Squirt some blood! */
		}
		DamageMObj(linetarget,t1,t1,damage);		/* Do the damage */
		return;
	}

/* Shoot wall */

	if (shootline2) {		/* Hit a wall? */
		if (shootline2->special) {		/* Special */
			P_ShootSpecialLine(t1, shootline2);	/* Open a door or switch? */
		}
		if (shootline2->frontsector->CeilingPic==-1) {
			if (shootz2 > shootline2->frontsector->ceilingheight) {
				return;		/* don't shoot the sky! */
			}
			if	(shootline2->backsector &&
				shootline2->backsector->CeilingPic==-1) {
				return;		/* it's a sky hack wall */
			}
		}
		P_SpawnPuff(shootx2,shooty2,shootz2);		/* Make a puff of smoke */
	}
}
