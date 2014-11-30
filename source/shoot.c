#include "Doom.h"
#include <IntMath.h>

//===================
//
// IN
//
// A line will be shootdivd from the middle of shooter in the direction of
// attackangle until either a shootable mobj is within the visible
// aimtopslope / aimbottomslope range, or a solid wall blocks further
// tracing.  If no thing is targeted along the entire range, the first line
// that blocks the midpoint of the shootdiv will be hit.
//===================

line_t *shootline;
mobj_t *shootmobj;
Fixed shootslope;					// between aimtop and aimbottom
Fixed shootx, shooty, shootz;		// location for puff/blood

//===================
//
// TEMPS
//
//===================

static Fixed aimmidslope;				// for detecting first wall hit
static vector_t shootdiv;
static Fixed shootx2, shooty2;
static Fixed firstlinefrac;

static int shootdivpositive;

static Fixed old_frac;
static void	*old_value;
static Boolean old_isline;
static int ssx1,ssy1,ssx2,ssy2;

/**********************************

	Returns true if strace crosses the given node successfuly

**********************************/

static Boolean PA_CrossBSPNode(node_t *bsp)
{
	Word side;

	if ((Word)bsp & 1) {
		return PA_CrossSubsector((subsector_t *)((LongWord)bsp&(~1UL)));
	}

//
// decide which side the start point is on
//
	side = PointOnVectorSide(shootdiv.x, shootdiv.y,&bsp->Line);

// cross the starting side

	if (!PA_CrossBSPNode((node_t*)bsp->Children[side]) ) {
		return FALSE;
	}

// the partition plane is crossed here

	if (side == PointOnVectorSide(shootx2,shooty2,&bsp->Line)) {
		return TRUE;			// the line doesn't touch the other side
	}

// cross the ending side
	return PA_CrossBSPNode((node_t *)bsp->Children[side^1]);
}

/*
=====================
=
= P_Shoot2
=
=====================
*/

void P_Shoot2(void)
{
	mobj_t		*t1;
	unsigned	angle;

	t1 = shooter;

	shootline = 0;
	shootmobj = 0;

	angle = attackangle >> ANGLETOFINESHIFT;

	shootdiv.x = t1->x;
	shootdiv.y = t1->y;
	shootx2 = t1->x + (attackrange>>FRACBITS)*finecosine[angle];
	shooty2 = t1->y + (attackrange>>FRACBITS)*finesine[angle];
	shootdiv.dx = shootx2 - shootdiv.x;
	shootdiv.dy = shooty2 - shootdiv.y;
	shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;

	shootdivpositive = (shootdiv.dx ^ shootdiv.dy)>0;

	ssx1 = shootdiv.x >> 16;
	ssy1 = shootdiv.y >> 16;
	ssx2 = shootx2 >> 16;
	ssy2 = shooty2 >> 16;

	aimmidslope = (aimtopslope + aimbottomslope)>>1;

//
// cross everything
//
	old_frac = 0;

	PA_CrossBSPNode(FirstBSPNode);

// check the last intercept if needed
	if (!shootmobj)
		PA_DoIntercept(0, FALSE, FRACUNIT);

//
// post process
//
	if (shootmobj)
		return;

	if (!shootline)
		return;

//
// calculate the intercept point for the first line hit
//
	// position a bit closer
	firstlinefrac -= IMFixDiv(4*FRACUNIT,attackrange);

	shootx = shootdiv.x + IMFixMul(shootdiv.dx, firstlinefrac);
	shooty = shootdiv.y + IMFixMul(shootdiv.dy, firstlinefrac);
	shootz = shootz + IMFixMul(aimmidslope,IMFixMul(firstlinefrac,attackrange));

}


/*
==================
=
= PA_DoIntercept
=
==================
*/

Boolean PA_DoIntercept(void *value,Boolean isline,int frac)
{
	int		temp;

	if (old_frac < frac)
	{
		temp = (int)old_value;
		old_value = value;
		value = (void *)temp;

		temp = old_isline;
		old_isline = isline;
		isline = temp;

		temp = old_frac;
		old_frac = frac;
		frac = temp;
	}

	if (frac == 0 || frac >= FRACUNIT)
		return TRUE;

	if (isline)
		return PA_ShootLine ((line_t *)value, frac);

	return PA_ShootThing ((mobj_t *)value, frac);
}


/*
==============================================================================
=
= PA_ShootLine
=
==============================================================================
*/

Boolean	PA_ShootLine (line_t *li, Fixed interceptfrac)
{
	Fixed		slope;
	Fixed		dist;
	sector_t	*front, *back;
	Fixed opentop,openbottom;

	if ( !(li->flags & ML_TWOSIDED) )
	{
		if (!shootline)
		{
			shootline = li;
			firstlinefrac = interceptfrac;
		}
		old_frac = 0;	// don't shoot anything past this
		return FALSE;
	}

//
// crosses a two sided line
//
	front = li->frontsector;
	back = li->backsector;

	if (front->ceilingheight < back->ceilingheight)
		opentop = front->ceilingheight;
	else
		opentop = back->ceilingheight;
	if (front->floorheight > back->floorheight)
		openbottom = front->floorheight;
	else
		openbottom = back->floorheight;

	dist = IMFixMul(attackrange,interceptfrac);

	if (li->frontsector->floorheight != li->backsector->floorheight)
	{
		slope = IMFixDiv(openbottom - shootz , dist);
		if (slope >= aimmidslope && !shootline)
		{
			shootline = li;
			firstlinefrac = interceptfrac;
		}
		if (slope > aimbottomslope)
			aimbottomslope = slope;
	}

	if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
	{
		slope = IMFixDiv(opentop - shootz , dist);
		if (slope <= aimmidslope && !shootline)
		{
			shootline = li;
			firstlinefrac = interceptfrac;
		}
		if (slope < aimtopslope)
			aimtopslope = slope;
	}

	if (aimtopslope <= aimbottomslope)
		return FALSE;		// stop


	return TRUE;		// shot continues
}

/*
==============================================================================
=
= PA_ShootThing
=
==============================================================================
*/
Boolean PA_ShootThing (mobj_t *th, Fixed interceptfrac)
{
	Fixed		frac;
	Fixed		dist;
	Fixed		thingaimtopslope, thingaimbottomslope;

	if (th == shooter)
		return TRUE;		// can't shoot self
	if (!(th->flags&MF_SHOOTABLE))
		return TRUE;		// corpse or something

// check angles to see if the thing can be aimed at
	dist = IMFixMul (attackrange, interceptfrac);
	thingaimtopslope = IMFixDiv(th->z+th->height - shootz , dist);
	if (thingaimtopslope < aimbottomslope)
		return TRUE;		// shot over the thing
	thingaimbottomslope = IMFixDiv(th->z - shootz, dist);
	if (thingaimbottomslope > aimtopslope)
		return TRUE;		// shot under the thing

//
// this thing can be hit!
//
	if (thingaimtopslope > aimtopslope)
		thingaimtopslope = aimtopslope;
	if (thingaimbottomslope < aimbottomslope)
		thingaimbottomslope = aimbottomslope;

	// shoot midway in the visible part of the thing
	shootslope = (thingaimtopslope+thingaimbottomslope)/2;

	shootmobj = th;

	// position a bit closer
	frac = interceptfrac - IMFixDiv(10*FRACUNIT,attackrange);
	shootx = shootdiv.x + IMFixMul(shootdiv.dx, frac);
	shooty = shootdiv.y + IMFixMul(shootdiv.dy, frac);
	shootz = shootz + IMFixMul(shootslope,IMFixMul(frac,attackrange));

	return FALSE;			// don't go any farther

}


/*
=================
=
= PA_SightCrossLine
=
= First checks the endpoints of the line to make sure that they cross the
= sight trace treated as an infinite line.
=
= If so, it calculates the fractional distance along the sight trace that
= the intersection occurs at.  If 0 < intercept < 1.0, the line will block
= the sight.
=================
*/

Fixed PA_SightCrossLine (line_t *line)
{
	int			s1, s2;
	int			p1x,p1y,p2x,p2y,p3x,p3y,p4x,p4y,dx,dy,ndx,ndy;

// p1, p2 are line endpoints
	p1x = line->v1.x >> 16;
	p1y = line->v1.y >> 16;
	p2x = line->v2.x >> 16;
	p2y = line->v2.y >> 16;

// p3, p4 are sight endpoints
	p3x = ssx1;
	p3y = ssy1;
	p4x = ssx2;
	p4y = ssy2;

	dx = p2x - p3x;
	dy = p2y - p3y;

	ndx = p4x - p3x;		// this can be precomputed if worthwhile
	ndy = p4y - p3y;

	s1 =  (ndy * dx) <  (dy * ndx);

	dx = p1x - p3x;
	dy = p1y - p3y;

	s2 =  (ndy * dx) <  (dy * ndx);

	if (s1 == s2)
		return -1;			// line isn't crossed

	ndx = p1y - p2y;		// vector normal to world line
	ndy = p2x - p1x;

	s1 = ndx*dx + ndy*dy;	// distance projected onto normal

	dx = p4x - p1x;
	dy = p4y - p1y;

	s2 = ndx*dx + ndy*dy;	// distance projected onto normal

	s2 = IMFixDiv(s1,(s1+s2));

	return s2;
}



/*
=================
=
= PA_CrossSubsectorPass
=
= Returns true if strace crosses the given subsector successfuly
=================
*/

static struct {
 vertex_t tv1,tv2;
} thingline;

Boolean PA_CrossSubsector (subsector_t *sub)
{
	seg_t		*seg;
	line_t		*line;
	int			count;
	Fixed		frac;
	mobj_t		*thing;

//
// check things
//
	for (thing = sub->sector->thinglist ; thing ; thing = thing->snext )
	{
		if (thing->subsector != sub)
			continue;

		// check a corner to corner crossection for hit

		if (shootdivpositive) {
			thingline.tv1.x = thing->x - thing->radius;
			thingline.tv1.y = thing->y + thing->radius;

			thingline.tv2.x = thing->x + thing->radius;
			thingline.tv2.y = thing->y - thing->radius;
		} else {
			thingline.tv1.x = thing->x - thing->radius;
			thingline.tv1.y = thing->y - thing->radius;

			thingline.tv2.x = thing->x + thing->radius;
			thingline.tv2.y = thing->y + thing->radius;
		}

		frac = PA_SightCrossLine ((line_t *)&thingline);

		if (frac < 0 || frac > FRACUNIT) {
			continue;
		}
		if (!PA_DoIntercept ( (void *)thing, FALSE, frac)) {
			return FALSE;
		}
	}

//
// check lines
//
	count = sub->numsublines;
	seg = sub->firstline;

	for ( ; count ; seg++, count--)
	{
		line = seg->linedef;

		if (line->validcount == validcount)
			continue;		// already checked other side
		line->validcount = validcount;

		frac = PA_SightCrossLine (line);

		if (frac < 0 || frac > FRACUNIT)
			continue;

		if (!PA_DoIntercept ( (void *)line, TRUE, frac))
//		if (!PA_ShootLine (line, frac))
			return FALSE;
	}


	return TRUE;			// passed the subsector ok
}



