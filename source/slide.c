#include "Doom.h"
#include <IntMath.h>

#define	CLIPRADIUS	23
#define SIDE_ON	0
#define	SIDE_FRONT	1
#define	SIDE_BACK	-1

Fixed slidex, slidey;		// the final position
line_t *specialline;

static Fixed		slidedx, slidedy;		// current move for completablefrac

static Fixed		endbox[4];				// final proposed position

static Fixed blockfrac;			// the fraction of move that gets completed
static Fixed blocknvx, blocknvy;	// the vector of the line that blocks move

// p1, p2 are line endpoints
// p3, p4 are move endpoints

static int p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y;
static Fixed	nvx, nvy;				// normalized line vector

static mobj_t *slidething;

static int	SL_PointOnSide2 (int x1, int y1, int x2, int y2, int x3, int y3)
{
	int	nx, ny;
	int dist;

	x1 = (x1-x2);
	y1 = (y1-y2);

	nx = (y3-y2);
	ny = (x2-x3);

	dist = IMFixMul(x1,nx);
	dist += IMFixMul(y1,ny);

	if (dist < 0)
		return SIDE_BACK;
	return SIDE_FRONT;
}


/*
===================
=
= P_SlideMove
=
===================
*/

void P_SlideMove (mobj_t *mo)
{
	Fixed	dx, dy;
	Fixed	rx, ry;
	int		i;
	Fixed	frac, slide;

	slidething = mo;
	dx = slidething->momx;
	dy = slidething->momy;
	slidex = slidething->x;
	slidey = slidething->y;

// perform a maximum of three bumps

	for (i=0 ; i<3 ; i++) {
		frac = P_CompletableFrac (dx,dy);
		if (frac != 0x10000)
			frac -= 0x1000;
		if (frac < 0)
			frac = 0;
		rx = IMFixMul(frac,dx);
		ry = IMFixMul(frac,dy);

		slidex += rx;
		slidey += ry;

	//
	// made it the entire way
	//
		if (frac == 0x10000) {
			slidething->momx = dx;
			slidething->momy = dy;
			SL_CheckSpecialLines (slidething->x, slidething->y
				, slidex, slidey);
			return;
		}

	//
	// project the remaining move along the line that blocked movement
	//
		dx -= rx;
		dy -= ry;
		slide = IMFixMul(dx,blocknvx);
		slide += IMFixMul(dy,blocknvy);

		dx = IMFixMul(slide,blocknvx);
		dy = IMFixMul(slide,blocknvy);
	}

//
// some hideous situation has happened that won't let the player slide
//
	slidex = slidething->x;
	slidey = slidething->y;
	slidething->momx = slidething->momy = 0;
}


/*
===================
=
= P_CompletableFrac
=
= Returns the fraction of the move that is completable
===================
*/

Fixed P_CompletableFrac(Fixed dx,Fixed dy)
{
	int			xl,xh,yl,yh,bx,by;


	blockfrac = 0x10000;		// the entire dist until shown otherwise
	slidedx = dx;
	slidedy = dy;

	endbox[BOXTOP] = slidey + CLIPRADIUS*FRACUNIT;
	endbox[BOXBOTTOM] = slidey - CLIPRADIUS*FRACUNIT;
	endbox[BOXRIGHT] = slidex + CLIPRADIUS*FRACUNIT;
	endbox[BOXLEFT] = slidex - CLIPRADIUS*FRACUNIT;

	if (dx > 0)
		endbox[BOXRIGHT] += dx;
	else
		endbox[BOXLEFT] += dx;
	if (dy > 0)
		endbox[BOXTOP] += dy;
	else
		endbox[BOXBOTTOM] += dy;

	++validcount;

//
// check lines
//
	xl = (endbox[BOXLEFT] - BlockMapOrgX)>>MAPBLOCKSHIFT;
	xh = (endbox[BOXRIGHT] - BlockMapOrgX)>>MAPBLOCKSHIFT;
	yl = (endbox[BOXBOTTOM] - BlockMapOrgY)>>MAPBLOCKSHIFT;
	yh = (endbox[BOXTOP] - BlockMapOrgY)>>MAPBLOCKSHIFT;

	if (xl<0)
		xl = 0;
	if (yl<0)
		yl = 0;
	if (xh>=BlockMapWidth)
		xh = BlockMapWidth-1;
	if (yh>=BlockMapHeight)
		yh = BlockMapHeight-1;

	for (bx=xl ; bx<=xh ; bx++)
		for (by=yl ; by<=yh ; by++) {
			BlockLinesIterator(bx,by,SL_CheckLine);
		}

//
// examine results
//
	if (blockfrac < 0x1000)
	{
		blockfrac = 0;
		specialline = 0;	// can't cross anything on a bad move
		return 0;				// solid wall or thing
	}

	return blockfrac;
}


int SL_PointOnSide (int x, int y)
{
	int		dx, dy, dist;


	dx = x - p1x;
	dy = y - p1y;

	dist = IMFixMul(dx,nvx);
	dist += IMFixMul(dy,nvy);

	if (dist > FRACUNIT)
		return SIDE_FRONT;
	if (dist < -FRACUNIT)
		return SIDE_BACK;

	return SIDE_ON;
}


Fixed SL_CrossFrac (void)
{
	int		dx, dy, dist1, dist2, frac;

// project move start and end points onto line normal
	dx = p3x - p1x;
	dy = p3y - p1y;

	dist1 = IMFixMul(dx,nvx);
	dist1 += IMFixMul(dy,nvy);

	dx = p4x - p1x;
	dy = p4y - p1y;

	dist2 = IMFixMul(dx,nvx);
	dist2 += IMFixMul(dy,nvy);

	if ( (dist1 < 0) == (dist2 < 0)	)
		return FRACUNIT;		// doesn't cross

	frac = IMFixDiv(dist1, dist1 - dist2 );

	return frac;
}


Boolean CheckLineEnds (void)
{
	int		snx, sny;		// sight normals
	int		dist1, dist2;
	int		dx, dy;

	snx = p4y-p3y;
	sny = -(p4x-p3x);

	dx = p1x - p3x;
	dy = p1y - p3y;

	dist1 = IMFixMul(dx,snx);
	dist1 += IMFixMul(dy,sny);

	dx = p2x - p3x;
	dy = p2y - p3y;

	dist2 = IMFixMul(dx,snx);
	dist2 += IMFixMul(dy,sny);

	if ( (dist1<0) == (dist2<0) )
		return TRUE;

	return FALSE;
}


/*
====================
=
= ClipToLine
=
= Call with p1 and p2 set to the endpoints
= and nvx, nvy set to normalized vector
= Assumes the start point is definately on the front side of the line
= returns the fraction of the current move that crosses the line segment
====================
*/

void ClipToLine ( void )
{
	Fixed frac;
	int			side2, side3;

//
// adjust start so it will be the first point contacted on the player
// circle
//

// p3, p4 are move endpoints

	p3x = slidex - CLIPRADIUS*nvx;
	p3y = slidey - CLIPRADIUS*nvy;

	p4x = p3x + slidedx;
	p4y = p3y + slidedy;

//
// if the adjusted point is on the other side of the line, the endpoint
// must be checked
//
	side2 = SL_PointOnSide (p3x, p3y);

	if (side2 == SIDE_BACK)
	{

		return;	// !!! ClipToPoint and slide along normal to line
	}

	side3 = SL_PointOnSide (p4x, p4y);
	if (side3 == SIDE_ON)
		return;		// the move goes flush with the wall
	if (side3 == SIDE_FRONT)
		return;		// moves doesn't cross line

	if (side2 == SIDE_ON)
	{
		frac = 0;		// moves towards the line
		goto blockmove;
	}

//
// the line endpoints must be on opposite sides of the move trace
//

//
// find the fractional intercept
//
	frac = SL_CrossFrac ();

	if (frac < blockfrac)
	{
blockmove:
		blockfrac = frac;
		blocknvx = -nvy;
		blocknvy = nvx;
	}
}

/*
==================
=
= SL_CheckLine
=
==================
*/

Word SL_CheckLine(line_t *ld)
{
	Fixed opentop, openbottom;
	sector_t	*front, *back;
	int			side1, temp;

// check bbox first
	if (endbox[BOXRIGHT] < ld->bbox[BOXLEFT]
	||	endbox[BOXLEFT] > ld->bbox[BOXRIGHT]
	||	endbox[BOXTOP] < ld->bbox[BOXBOTTOM]
	||	endbox[BOXBOTTOM] > ld->bbox[BOXTOP] )
		return TRUE;

// see if it can possibly block movement

	if (!ld->backsector || ld->flags & ML_BLOCKING)
		goto findfrac;		// explicitly blocking

	front = ld->frontsector;
	back = ld->backsector;

	if (front->floorheight > back->floorheight)
		openbottom = front->floorheight;
	else
		openbottom = back->floorheight;

	if (openbottom - slidething->z > 24*FRACUNIT)
		goto findfrac;		// too big of a step up

	if (front->ceilingheight < back->ceilingheight)
		opentop = front->ceilingheight;
	else
		opentop = back->ceilingheight;

	if (opentop - openbottom >= 56*FRACUNIT)
		return TRUE;		// the line doesn't block movement

// the line definately blocks movement

findfrac:

// p1, p2 are line endpoints
	p1x = ld->v1.x;
	p1y = ld->v1.y;
	p2x = ld->v2.x;
	p2y = ld->v2.y;

	nvx = finesine[ld->fineangle];
	nvy = -finecosine[ld->fineangle];

	side1 = SL_PointOnSide (slidex, slidey);
	if (side1 == SIDE_ON)
		return TRUE;
	if (side1 == SIDE_BACK)
	{
		if (!ld->backsector)
			return TRUE;			// don't clip to backs of one sided lines
		temp = p1x;
		p1x = p2x;
		p2x = temp;
		temp = p1y;
		p1y = p2y;
		p2y = temp;
		nvx = -nvx;
		nvy = -nvy;
	}
	ClipToLine ();

	return TRUE;
}


static line_t **list;
static line_t *ld;

void SL_CheckSpecialLines (int x1, int y1, int x2, int y2)
{
	int			bx, by, xl, xh, yl, yh, bxl, bxh, byl, byh;
	int			x3,y3,x4,y4;
	int			side1, side2;

	if (x1<x2) {
		xl = x1;
		xh = x2;
	} else {
		xl = x2;
		xh = x1;
	}
	if (y1<y2) {
		yl = y1;
		yh = y2;
	} else {
		yl = y2;
		yh = y1;
	}

	bxl = (xl - BlockMapOrgX)>>MAPBLOCKSHIFT;
	bxh = (xh - BlockMapOrgX)>>MAPBLOCKSHIFT;
	byl = (yl - BlockMapOrgY)>>MAPBLOCKSHIFT;
	byh = (yh - BlockMapOrgY)>>MAPBLOCKSHIFT;

	if (bxl<0)
		bxl = 0;
	if (byl<0)
		byl = 0;
	if (bxh>=BlockMapWidth)
		bxh = BlockMapWidth-1;
	if (byh>=BlockMapHeight)
		byh = BlockMapHeight-1;

	specialline = 0;
	++validcount;

	for (bx=bxl ; bx<=bxh ; bx++)
		for (by=byl ; by<=byh ; by++) {

			for ( list = BlockMapLines[(by*BlockMapWidth)+bx] ;list[0];++list) {
				ld = list[0];
				if (!ld->special)
					continue;
				if (ld->validcount == validcount)
					continue;		// line has already been checked
				ld->validcount = validcount;

				if (xh < ld->bbox[BOXLEFT]
				||	xl > ld->bbox[BOXRIGHT]
				||	yh < ld->bbox[BOXBOTTOM]
				||	yl > ld->bbox[BOXTOP] )
					continue;

				x3 = ld->v1.x;
				y3 = ld->v1.y;
				x4 = ld->v2.x;
				y4 = ld->v2.y;

				side1 = SL_PointOnSide2 (x1,y1, x3,y3, x4,y4);
				side2 = SL_PointOnSide2 (x2,y2, x3,y3, x4,y4);

				if (side1 == side2)
					continue;		// move doesn't cross line

				side1 = SL_PointOnSide2 (x3,y3, x1,y1, x2,y2);
				side2 = SL_PointOnSide2 (x4,y4, x1,y1, x2,y2);

				if (side1 == side2)
					continue;		// line doesn't cross move

				specialline = ld;
				return;
			}
		}

}
