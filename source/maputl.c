#include "Doom.h"
#include <IntMath.h>

/**********************************

	Given the numerator and the denominator of a fraction for a slope, 
	return the equivalent angle.
	Note : I assume that denominator is greater or equal to the numerator

**********************************/

angle_t SlopeAngle(LongWord num,LongWord den)
{
	num>>=(FRACBITS-3);			/* Leave in 3 extra bits for just a little more precision */
	den>>=FRACBITS;				/* Must be an int */

	num = num*(IDivTable[den]>>9);	/* Perform the divide using a recipocal mul */
	num>>=((FRACBITS+3)-SLOPEBITS);	/* Isolate the fraction for index to the angle table */

	if (num>SLOPERANGE) {			/* Out of range? */
		num = SLOPERANGE;			/* Fix it */
	}	
	return tantoangle[num];			/* Get the angle */
}

/**********************************

	To get a global angle from cartesian coordinates, the coordinates are
	flipped until they are in the first octant of the coordinate system, then
	the y (<=x) is scaled and divided by x to get a tangent (slope) value
	which is looked up in the tantoangle[] table.

**********************************/

angle_t PointToAngle(Fixed x1,Fixed y1,Fixed x2,Fixed y2)
{
	x2 -= x1;	/* Convert the two points into a fractional slope */
	y2 -= y1;

	if (x2 || y2) {				/* Not 0,0? */
  		if (x2>=0) {				/* Positive x? */
			if (y2>=0) {			/* Positive x, Positive y */
				if (x2>y2) {		/* Octant 0? */
					return SlopeAngle(y2,x2);     /* Octant 0 */
				}
				return (ANG90-1)-SlopeAngle(x2,y2);  /* Octant 1 */
			}
			y2 = -y2;		/* Get the absolute value of y (Was negative) */
			if (x2>y2) {	/* Octant 6 */
				return -SlopeAngle(y2,x2);		/* Octant 6 */
			}
			return SlopeAngle(x2,y2)+ANG270;	/* Octant 7 */
		}
		x2 = -x2;			/* Negate x (Make it positive) */
		if (y2>=0) {		/* Positive y? */
			if (x2>y2) {	/* Octant 3? */
				return (ANG180-1)-SlopeAngle(y2,x2);	/* Octant 3 */
			}
			return SlopeAngle(x2,y2)+ANG90;		/* Octant 2 */
		}
		y2 = -y2;		/* Negate y (Make it positive) */
		if (x2>y2) {
			return SlopeAngle(y2,x2)+ANG180;	/* Octant 4 */
		}
		return (ANG270-1)-SlopeAngle(x2,y2);	/* Octant 5 */
	}
	return 0;		/* In case of 0,0, return an angle of 0 */
}

/**********************************

	Get the distance from the view x,y from a point in 2D space
	The normal formula is dist = sqrt(x*x+y*y). But this is bad.
	Instead I first get the angle of the point and then rotate it so
	that it is directly ahead, the resulting point is the distance.

**********************************/

Fixed PointToDist(Fixed x,Fixed y)
{
	angle_t angle;

	x-=viewx;		/* Get the point offset from the camera */
	if (x<0) {
		x=-x;		/* I only want the absolute value */
	}
	y-=viewy;		/* Adjust the Y */
	if (y<0) {
		y=-y;		/* Abs value of Y */
	}
	if (y>x) {
		Fixed temp;
		temp = x;
		x = y;
		y = temp;
	}
	angle = SlopeAngle(y,x)>>ANGLETOFINESHIFT;		/* x = denominator */
	x = (x>>(FRACBITS-3))*finecosine[angle];		/* Rotate the x */
	x += (y>>(FRACBITS-3))*finesine[angle];			/* Rotate the y and add it */
	x >>= 3;		/* Convert to fixed (I added 3 extra bits of precision) */
	return x;			/* This is the true distance */
}

/**********************************

	Gives an estimation of distance (not exact)
	This is used when an exact distance between two points is not needed.

**********************************/

Fixed GetApproxDistance(Fixed dx,Fixed dy)
{
	if (dx<0) {
		dx = -dx;		/* Get the absolute value of the distance */
	}
	if (dy<0) {
		dy = -dy;
	}
	if (dx < dy) {		/* Is the x smaller? */
		dx>>=1;			/* Use half the x */
	} else {
		dy>>=1;			/* Or use half the y */
	}
	dx+=dy;				/* Add larger and half of the smaller for distance */			
	return dx;			/* Return result */
}

/**********************************

	Return TRUE if the point is on the BACK side.
	Otherwise return FALSE if on the front side.
	This code is optimized for the simple case of vertical
	or horizontal lines.
	
**********************************/

Word PointOnVectorSide(Fixed x,Fixed y,vector_t *line)
{
	Word Result;
	Fixed dx,dy;
	
	Result = TRUE;			/* Assume I am on the back side */

	/* Special case #1, vertical lines */

	dx = line->dx;		/* Cache the line vector's delta x and y */
	dy = line->dy;
	
	x = x - line->x;	/* Get the offset from the base of the line */
	if (!dx) {			/* Vertical line? (dy is base direction) */
		if (x <= 0) {		/* Which side of the line am I? */
			dy = -dy;
		}
		if (dy>=0) {
			Result = FALSE;	/* On the front side! */
		}
		return Result;
	}

	/* Special case #2, horizontal lines */
	
	y = y - line->y;	/* Get the offset for y */
	if (!dy) {			/* Horizontal line? (dx is base direction) */
		if (y <= 0) {		/* Which side of the line */
			dx = -dx;
		} 
		if (dx<=0) {
			Result = FALSE;	/* On the front side! */
		}
		return Result;		/* Return the answer */
	}

	/* Special case #3, Sign compares */ 
	
	if ( (dy^dx^x^y) & 0x80000000UL ) {		/* Negative compound sign? */
		if (!((dy^x) & 0x80000000UL)) {		/* Positive cross product? */
			Result = FALSE;	/* Front side is positive */
		}
		return Result;
	}

	/* Case #4, do it the hard way with a cross product */
	
	x>>=FRACBITS;		/* Get the integer */
	y>>=FRACBITS;
	x = (dy>>FRACBITS) * x;	/* Create the cross product */
	y = (dx>>FRACBITS) * y;

	if (y < x) {			/* Which side? */
		Result = FALSE;		/* Front side */
	}
	return Result;			/* Return the side */
}

/**********************************

	Return the pointer to a subsector record using an input
	x and y. I may need to follow the BSP tree to assist.

**********************************/

subsector_t *PointInSubsector(Fixed x,Fixed y)
{
	node_t *node;			/* Current BSP node */

	node = FirstBSPNode;	/* Get the node count from the BSP tree */
	do {					/* There is ALWAYS a BSP tree */
		node = (node_t *)node->Children[PointOnVectorSide(x,y,&node->Line)];	/* Branch there */
	} while (!((Word)node&1));		/* Valid pointer? */
	return (subsector_t *)(((Byte *)node)-1);	/* Remove the flag */
}

/**********************************

	Convert a line record into a line vector
	
**********************************/

void MakeVector(line_t *li,vector_t *dl)
{
	Fixed Temp;
	Temp = li->v1.x;		/* Get the X of the vector */
	dl->x = Temp;			/* Save the x */
	dl->dx = li->v2.x-Temp;	/* Get the X delta */
	
	Temp = li->v1.y;		/* Do the same for the Y */
	dl->y = Temp;
	dl->dy = li->v2.y-Temp;
}

/**********************************

	Returns the fractional intercept point along the first vector

**********************************/

Fixed InterceptVector(vector_t *First,vector_t *Second)
{
	Fixed num,den;
	Fixed dx2,dy2;

	dx2 = Second->dx>>FRACBITS;	/* Get the integer of the second line */
	dy2 = Second->dy>>FRACBITS;
	
	den = (dy2*(First->dx>>FRACBITS));		/* Get the step slope of the first vector */
	den -= (dx2*(First->dy>>FRACBITS));		/* Sub the step slope of the second vector */
	if (!den) {		/* They are parallel vectors! */
		return -1;		/* Infinity */
	}
	num = ((Second->x-First->x)>>FRACBITS) * dy2;		/* Get the slope to the start position */
	num += ((First->y-Second->y)>>FRACBITS) * dx2;
	num <<= FRACBITS;		/* Convert to fixed point */
	num = num / den;		/* How far to the intersection? */
	return num;		/* Return the distance of intercept */
}

/**********************************

	 Return the height of the open span or zero if a single sided line.
	
**********************************/

Word LineOpening(line_t *linedef)
{
	sector_t *front;
	sector_t *back;
	Fixed Top,Bottom;

	Top = 0;
	back = linedef->backsector;		/* Get the back side */
	if (back) {		/* Double sided line! */
		front = linedef->frontsector;		/* Get the front sector */
		Top = front->ceilingheight;			/* Assume front height */
		if (Top > back->ceilingheight) {
			Top = back->ceilingheight;		/* Use the back height */
		}
		Bottom = front->floorheight;		/* Assume front height */
		if (Bottom < back->floorheight) {	
			Bottom = back->floorheight;		/* Use back sector height */
		}
		Top-=Bottom;			/* Get the span (Zero for a closed door) */
	}
	return Top;		/* Return the span */
}

/**********************************

	Unlinks a thing from block map and sectors
	
**********************************/

void UnsetThingPosition(mobj_t *thing)
{

/* Things that have no shapes don't need to be in the sector map */
/* The sector links are used for drawing of the sprites */

	if (!(thing->flags & MF_NOSECTOR)) {	/* Visible item? */
		mobj_t *next,*prev;
		
		next = thing->snext;		/* Cache the pointers */
		prev = thing->sprev;
		if (next) {		/* Is there a forward link? */
			next->sprev = prev;	/* Attach a new backward link */
		}
		if (prev) {		/* Is there a backward link? */
			prev->snext = next;	/* Adjust the forward link */
		} else {
			thing->subsector->sector->thinglist = next;	/* New first link */
		}
	}

/* Inert things don't need to be in blockmap such as missiles or blood and gore */

	if (!(thing->flags & MF_NOBLOCKMAP)) {
		mobj_t *next,*prev;
		next = thing->bnext;		/* Cache the pointers */
		prev = thing->bprev;
		
		if (next) {			/* Forward link? */
			next->bprev = prev;
		}
		if (prev) {		/* Is there a previous link? */
			prev->bnext = next;
		} else {
			Word blockx,blocky;
			blockx = (thing->x - BlockMapOrgX)>>MAPBLOCKSHIFT;	/* Get the tile offsets */
			blocky = (thing->y - BlockMapOrgY)>>MAPBLOCKSHIFT;
			if (blockx<BlockMapWidth && blocky<BlockMapHeight) {	/* Failsafe... */
				blocky = blocky*BlockMapWidth;
				blocky += blockx;
				BlockLinkPtr[blocky] = next;		/* Index to the block map */
			}
		}
	}
}

/**********************************

	Links a thing into both a block and a subsector based on it's x y
	Sets thing->subsector properly
	
**********************************/

void SetThingPosition(mobj_t *thing)
{
	subsector_t *ss;
	
	ss = PointInSubsector(thing->x,thing->y);		/* Get the current subsector occupied */
	thing->subsector = ss;			/* Save it */
	
/* invisible things don't go into the sector links so they don't draw */

	if (!(thing->flags & MF_NOSECTOR)) {
		sector_t *sec;
		mobj_t *next;
		sec = ss->sector;		/* Get the current sector */
		next = sec->thinglist;	/* Get the first link as my next */
		thing->snext = next; 	/* Init the next link */
		if (next) {				/* Was there a link in the first place? */
			next->sprev = thing;	/* The previous is now here */
		}
		thing->sprev = 0;		/* Init the previous link */
		sec->thinglist = thing;		/* Set the new first link (Me!) */
	}

/* inert things don't need to be in blockmap, like blood and gore */

	if (!(thing->flags & MF_NOBLOCKMAP)) {	/* Do I need to? */
		Word blockx, blocky;
		
		thing->bprev = 0;			/* No previous link */
		thing->bnext = 0;
		
		blockx = (thing->x - BlockMapOrgX)>>MAPBLOCKSHIFT;	/* Get the tile index */
		blocky = (thing->y - BlockMapOrgY)>>MAPBLOCKSHIFT;
		if (blockx<BlockMapWidth && blocky<BlockMapHeight) {	/* Failsafe */
			mobj_t *next;
			blocky = blocky*BlockMapWidth;
			blocky += blockx;
			next = BlockLinkPtr[blocky];	/* Get the next link */
			BlockLinkPtr[blocky] = thing;	/* Set the new entry */
			thing->bnext = next;			/* Save the new link */
			if (next) {			/* Is there a forward link? */
				next->bprev = thing;		/* Place a backward link */
			}
		}
	}
}

/**********************************

	The validcount flags are used to avoid checking lines
	that are marked in multiple mapblocks, so increment validcount before
	the first call to BlockLinesIterator, then make one or more calls to it
	
**********************************/

Word BlockLinesIterator(Word x,Word y,Word(*func)(line_t*))
{
	line_t **list;
	line_t *ld;

	if (x<BlockMapWidth && y<BlockMapHeight) {	/* On the map? */
		y = y*BlockMapWidth;
		y += x;
		list = BlockMapLines[y];		/* Get the first line pointer */
		for (;;) {
			ld = list[0];	/* Get the entry */
			if (!ld) {		/* End of a list? */
				break;		/* Get out of the loop */
			}
			if (ld->validcount != validcount) {		/* Line not checked? */
				ld->validcount = validcount;	/* Mark it */
				if (!func(ld)) {		/* Call the line proc */
					return FALSE;		/* I have a match? */
				}
			}
			++list;		/* Next entry */
		}
	}
	return TRUE;		/* Everything was checked */
}


/**********************************

	Scan all objects standing on this map block
	
**********************************/

Word BlockThingsIterator(Word x,Word y,Word(*func)(mobj_t*))
{
	mobj_t *mobj;

	if (x<BlockMapWidth && y<BlockMapHeight) {	/* Off the map? */
		y = y*BlockMapWidth;
		y += x;
		mobj = BlockLinkPtr[y];		/* Get the first object on the block */
		while (mobj) {		/* Valid object? */
			if (!func(mobj)) {	/* Call function */
				return FALSE;	/* I found it! */
			}
			mobj = mobj->bnext;	/* Next object in list */
		}
	}
	return TRUE;		/* Not found */
}
