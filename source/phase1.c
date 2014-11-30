#include "Doom.h"
#include <IntMath.h>

#define	MAXSEGS	32		/* Maximum number of segs to scan */

/**********************************

	By traversing the BSP tree, I will create a viswall_t array describing
	all walls are are visible to the computer screen, they may be projected off
	the left and right sides but this is to allow for scaling of the textures
	properly for clipping.
	
	All backface walls are removed by casting two angles to the end points and seeing
	if the differance in the angles creates a negative number (Reversed).
	The viswall_t record will contain the leftmost angle in unmodified 3 Space, the
	clipped screen left x and right x and the line segment that needs to be rendered.
	
	I also create all the sprite records (Unsorted) so that they can be merged
	with the rendering system to handle clipping.
	
**********************************/

typedef	struct {
	int	LeftX;		/* Left side of post */
	int RightX;		/* Right side of post */
} cliprange_t;

Word SpriteTotal;		/* Total number of sprites to render */
Word *SortedSprites;	/* Pointer to array of words of sprites to render */
static Word SortBuffer[MAXVISSPRITES*2];

static seg_t *curline;			/* Current line segment being processed */
static angle_t lineangle1;		/* Angle to leftmost side of wall segment */

static Word checkcoord[9][4] = {
{BOXRIGHT,BOXTOP,BOXLEFT,BOXBOTTOM},		/* Above,Left */
{BOXRIGHT,BOXTOP,BOXLEFT,BOXTOP},			/* Above,Center */
{BOXRIGHT,BOXBOTTOM,BOXLEFT,BOXTOP},		/* Above,Right */
{BOXLEFT,BOXTOP,BOXLEFT,BOXBOTTOM},			/* Center,Left */
{-1,0,0,0},			/* Center,Center */
{BOXRIGHT,BOXBOTTOM,BOXRIGHT,BOXTOP},		/* Center,Right */
{BOXLEFT,BOXTOP,BOXRIGHT,BOXBOTTOM},		/* Below,Left */
{BOXLEFT,BOXBOTTOM,BOXRIGHT,BOXBOTTOM},		/* Below,Center */
{BOXLEFT,BOXBOTTOM,BOXRIGHT,BOXTOP} };		/* Below,Right */

static cliprange_t solidsegs[MAXSEGS];		/* List of valid ranges to scan through */
static cliprange_t *newend;		/* Pointer to the first free entry */

/**********************************

	I will now find and try to display all objects and sprites in the 3D view. 
	I throw out any sprites that are off the screen to the left or right. 
	I don't check top to bottom.
	
**********************************/

static void SortAllSprites(void)
{
	vissprite_t *VisPtr;
	Word i;
	Word *LocalPtr;
	
	VisPtr = vissprites;
	SpriteTotal = vissprite_p - VisPtr;		/* How many sprites to draw? */
	if (SpriteTotal) {		/* Any sprites to speak of? */
		LocalPtr = SortBuffer;	/* Init buffer pointer */
		i = 0;
		do {
			*LocalPtr++ = (VisPtr->yscale<<7)+i;	/* Create array of indexs */
			++VisPtr;
		} while (++i<SpriteTotal);	/* All done? */
		SortedSprites = SortWords(SortBuffer,&SortBuffer[MAXVISSPRITES],SpriteTotal);		/* Sort the sprites */
	}
}

/**********************************

	I have a possible sprite record, transform to 2D coords 
	and then see if it is clipped.
	
**********************************/

static void PrepMObj(mobj_t *thing)
{
	Fixed Trx,Try,Trz;
	Word lump;
	LongWord Offset;
	vissprite_t	*vis;
	state_t *StatePtr;
	void **PatchHandle;
	patch_t *patch;
	int x1, x2;

/* This is a HACK, so I don't draw the player for the 3DO version */

	if (thing->player) {		/* Don't draw the player */
		return;
	}

/* Transform the origin point */

	vis = vissprite_p;
	if (vis == &vissprites[MAXVISSPRITES]) {		/* Too many? */
		return;		/* sprite overload, don't draw it */
	}

	Trx = thing->x - viewx;		/* Get the point in 3 Space */
	Try = thing->y - viewy;

	Trz = IMFixMul(Trx,viewcos);	/* Rotate around the camera */
	Trz += IMFixMul(Try,viewsin);	/* Add together */

	if (Trz < MINZ) {	/* Too large? */
		return;			/* Exit now */
	}
	
	Trx = IMFixMul(Trx,viewsin);		/* Calc the 3Space x coord */
	Trx -= IMFixMul(Try,viewcos);
	
	if (Trx > (Trz<<2) || Trx < -(Trz<<2) ) {
		return;		/* Greater than 45 degrees off the side */
	}

/* Decide which patch to use for sprite relative to player */

	StatePtr = thing->state;
	lump = StatePtr->SpriteFrame>>FF_SPRITESHIFT;		/* Get the resource # */
	PatchHandle = LoadAResourceHandle(lump);	/* Get the sprite group */
	patch = (patch_t *)*PatchHandle;		/* Deref the handle */
	Offset = ((LongWord *)patch)[StatePtr->SpriteFrame & FF_FRAMEMASK];
	if (Offset&PT_NOROTATE) {		/* Do I rotate? */
		angle_t ang;
		angle_t rot;
		patch = (patch_t *)&((Byte *)patch)[Offset & 0x3FFFFFFF];		/* Get pointer to rotation list */
		ang = PointToAngle(viewx,viewy,thing->x,thing->y);		/* Get angle to critter */
		ang -= thing->angle;		/* Adjust for object's facing */
		rot = (ang+(angle_t)((ANG45/2)*9U))>>29;	/* Get rotation offset */
		Offset = ((LongWord *)patch)[rot];		/* Use the rotated offset */
	}
	patch = (patch_t *) &((Byte *)patch)[Offset & 0x3FFFFFFF];	/* Get pointer to patch */

/* Store information in a vissprite */
/* I also will clip to screen coords */

	Trz = IMFixDiv(CenterX<<FRACBITS,Trz);		/* Get the scale factor */
	vis->xscale = Trz;			/* Save it */
	
	Trx -= patch->leftoffset<<FRACBITS;		/* Adjust the x to the sprite's x */
	x1 = (IMFixMul(Trx,Trz)>>FRACBITS)+CenterX;		/* Scale to screen coords */
	if (x1 > (int)ScreenWidth) {
		ReleaseAResource(lump);
		return;		/* Off the right side, don't draw */
	}

/* The shape is sideways, so I get the HEIGHT instead of the width! */
	
	x2 = IMFixMul(GetShapeHeight(&patch->Data),Trz)+x1;
	if (x2 <= 0) {
		ReleaseAResource(lump);
		return;		/* Off the left side */
	}

/* get light level */

	Try = IMFixMul(Trz,Stretch);	/* Adjust for aspect ratio */
	vis->yscale = Try;
	vis->PatchLump = lump;		/* Resource referance */
	vis->PatchOffset = (Byte *)patch - (Byte *)*PatchHandle;	/* Shape offset */
	vis->x1 = x1;	/* Save the edge coords */
	vis->x2 = x2;
	vis->thing = thing;
	if (thing->flags & MF_SHADOW) {		/* Draw a shadow... */
		x1 = 0x8000U;
	} else if (StatePtr->SpriteFrame & FF_FULLBRIGHT) {
		x1 = 255;				/* full bright */
	} else {
		x1 = thing->subsector->sector->lightlevel; /* + extralight; */
		if ((Word)x1 >= 256) {
			x1 = 255;		/* Use maximum */
		}
	}
	if (Offset&PT_FLIP) {	/* Reverse the shape */
		x1 |=0x4000;
	}
	vis->colormap = x1;	/* Save the light value */
	
	Trz = thing->z-viewz;
	vis->y2 = CenterY - (IMFixMul(Trz-(5<<FRACBITS),Try)>>FRACBITS);
	Trz = Trz+(patch->topoffset<<FRACBITS);		/* Height offset */
	vis->y1 = CenterY - (IMFixMul(Trz,Try)>>FRACBITS);		/* Get screen Y */
	if (vis->y2>=0 || vis->y1<(int)ScreenHeight) {	/* Clipped vertically? */
		vissprite_p = vis+1;		/* Use the record */
	}
	ReleaseAResource(lump);
}

/**********************************

	Given a sector pointer, and if I hadn't already rendered the sprites,
	make valid sprites for the sprite list.
	
**********************************/

static void SpritePrep(sector_t *se)
{
	mobj_t *thing;
	if (se->validcount != validcount) {		/* Has this been processed? */
		se->validcount = validcount;	/* Mark it */			
		thing = se->thinglist;		/* Init the thing list */
		if (thing) {				/* Traverse the linked list */
			do {
				PrepMObj(thing);		/* Draw the object if ok... */
				thing = thing->snext;	/* Next? */
			} while (thing);
		}
	}
}

/**********************************

	Store the data describing a wall section that needs to be drawn
	
**********************************/

static void StoreWallRange(Word LeftX,Word RightX)
{
	WallPrep(LeftX,RightX,curline,lineangle1);	/* Create the wall data */
}

/**********************************

	Clips a wall segment and adds it to the solid wall
	segment list for masking.
	
**********************************/

static void ClipSolidWallSegment(int LeftX,int RightX)
{
	cliprange_t	*next;
	cliprange_t *start;
	cliprange_t *next2;
	int Temp;

/* Find the first range that touches the range (adjacent pixels are touching) */

	start = solidsegs;	/* Init start table */
	Temp = LeftX-1;
	if (start->RightX < Temp) {	/* Loop down */
		do {
			++start;		/* Next entry */
		} while (start->RightX < Temp);
	}

	if (LeftX < start->LeftX) {		/* Clipped on the left? */
		if (RightX < start->LeftX-1) {	/* post is entirely visible, so insert a new clippost */
			StoreWallRange(LeftX,RightX);		/* Draw the wall */
			next = newend;
			newend = next+1;		/* Save the new last entry */
			if (next != start) {		/* Copy the current entry over */
				do {
					--next;		/* Move back one */
					next[1] = next[0];	/* Copy the struct */
				} while (next!=start);
			}
			start->LeftX = LeftX;	/* Insert the new record */
			start->RightX = RightX;
			return;			/* Exit now */
		}

	  /* Oh oh, there is a wall in front, clip me */
	  
		StoreWallRange(LeftX,start->LeftX-1);	/* I am clipped on the right */
		start->LeftX = LeftX;		/* Adjust the clip size to a new left edge */
	}

	if (RightX <= start->RightX) {	/* Is the rest obscured? */
		return;			/* Yep, exit now */
	}

	/* Start has the first post group that needs to be removed */
	
	next = start;
	next2 = next+1;
	if (RightX >= next2->LeftX-1) {
		do {
		/* there is a fragment between two posts */
			StoreWallRange(next->RightX+1,next2->LeftX-1);
			next=next2;
			if (RightX <= next2->RightX) {	/* bottom is contained in next */
				start->RightX = next2->RightX;	/* adjust the clip size */
				goto crunch;		/* Don't store the final fragment */
			}
			++next2;
		} while (RightX >= next2->LeftX-1);
	}
	StoreWallRange(next->RightX+1,RightX);		/* Save the final fragment */
	start->RightX = RightX;		/* Adjust the clip size (Clipped on the left) */

/* remove start+1 to next from the clip list, */
/* because start now covers their area */

crunch:
	if (next != start) {	/* Do I need to remove any? */
		if (next != newend) {	/* remove a post */
			do {
				++next;
				++start;
				start[0] = next[0];		/* Copy the struct */
			} while (next!=newend);
		}
		newend = start+1;		/* All disposed! */
	}
}

/**********************************

	Clips a wall segment but does not add it to the solid wall
	segment list for masking.
	
**********************************/

static void ClipPassWallSegment(int LeftX,int RightX)
{
	cliprange_t	*ClipPtr;
	cliprange_t *NextClipPtr;
	int Temp;

/* find the first range that touches the range (adjacent pixels are touching) */

	ClipPtr = solidsegs;
	Temp = LeftX-1;			/* Leftmost edge I can ignore */
	if (ClipPtr->RightX < Temp) { 	/* Skip over non-touching posts */
		do {
			++ClipPtr;		/* Next index */	
		} while (ClipPtr->RightX < Temp);
	}

	if (LeftX < ClipPtr->LeftX) {	/* Is the left side visible? */
		if (RightX < ClipPtr->LeftX-1) {	/* Post is entirely visible (above start) */
			StoreWallRange(LeftX,RightX);	/* Store the range! */
			return;					/* Exit now! */
		}
		StoreWallRange(LeftX,ClipPtr->LeftX-1);	/* Oh oh, I clipped on the right! */
	}
	
	/* At this point, I know that some leftmost pixels are hidden. */

	if (RightX <= ClipPtr->RightX) {		/* All are hidden? */
		return;			/* Don't draw. */
	}
	NextClipPtr = ClipPtr+1;		/* Next index */
	if (RightX >= NextClipPtr->LeftX-1) {	/* Now draw all fragments behind solid posts */
		do {
			StoreWallRange(ClipPtr->RightX+1,NextClipPtr->LeftX-1);
			if (RightX <= NextClipPtr->RightX) {	/* Is the rest hidden? */
				return;
			}
			ClipPtr = NextClipPtr;	/* Next index */
			++NextClipPtr;			/* Inc running pointer */
		} while (RightX >= NextClipPtr->LeftX-1);
	}
	StoreWallRange(ClipPtr->RightX+1,RightX);		/* Draw the final fragment */
}

/**********************************

	Clips the given segment and adds any visible pieces to the line list
	I also add to the solid wall list so that I can rule out BSP sections quickly.
	
**********************************/

static void AddLine(seg_t *line,sector_t *FrontSector)
{
	angle_t angle1,angle2,span,tspan;
	sector_t *backsector;

	angle1 = PointToAngle(viewx,viewy,line->v1.x,line->v1.y);	/* Calc the angle for the left edge */
	angle2 = PointToAngle(viewx,viewy,line->v2.x,line->v2.y);	/* Now the right edge */

	span = angle1 - angle2;		/* Get the line span */
	if (span >= ANG180) {		/* Backwards? */
		return;		/* Don't handle backwards lines */
	}
	lineangle1 = angle1;		/* Store the leftmost angle for StoreWallRange */
	angle1 -= viewangle;		/* Adjust the angle for viewangle */
	angle2 -= viewangle;

	tspan = angle1+clipangle;	/* Adjust the center x of 0 */
	if (tspan > doubleclipangle) {	/* Possibly off the left side? */
		tspan -= doubleclipangle;	/* See if it's visible */
		if (tspan >= span) {	/* Off the left? */
			return;	/* Remove it */
		}
		angle1 = clipangle;	/* Clip the left edge */
	}
	tspan = clipangle - angle2;		/* Get the right edge adjustment */
	if (tspan > doubleclipangle) {	/* Possibly off the right side? */
		tspan -= doubleclipangle;
		if (tspan >= span) {		/* Off the right? */
			return;			/* Off the right side */
		}
		angle2 = -(int)clipangle;		/* Clip the right side */
	}

/* The seg is in the view range, but not necessarily visible */
/* It may be a line for specials or imbedded floor line */

	angle1 = (angle1+ANG90)>>(ANGLETOFINESHIFT+1);		/* Convert angles to table indexs */
	angle2 = (angle2+ANG90)>>(ANGLETOFINESHIFT+1);
	angle1 = viewangletox[angle1];		/* Get the screen x left */
	angle2 = viewangletox[angle2];		/* Screen x right */
	if (angle1 >= angle2) {
		return;				/* This is too small to bother with or invalid */
	}
	--angle2;					/* Make the right side inclusive */
	backsector = line->backsector;	/* Get the back sector */
	curline = line;			/* Save the line record */

	if (!backsector ||	/* Single sided line? */
		backsector->ceilingheight <= FrontSector->floorheight ||	/* Closed door? */
		backsector->floorheight >= FrontSector->ceilingheight) {
		ClipSolidWallSegment(angle1,angle2);		/* Make a SOLID wall */
		return;
	}

	if (backsector->ceilingheight != FrontSector->ceilingheight ||	/* Normal window */
		backsector->floorheight != FrontSector->floorheight ||
		backsector->CeilingPic != FrontSector->CeilingPic ||		/* Different texture */
		backsector->FloorPic != FrontSector->FloorPic ||			/* Floor texture */
		backsector->lightlevel != FrontSector->lightlevel ||		/* Differant light? */
		line->sidedef->midtexture) {			/* Center wall texture? */
		ClipPassWallSegment(angle1,angle2);		/* Render but allow walls behind it */
	}
}

/**********************************

	Given a subsector pointer, pass all walls to the
	rendering engine. Also pass all the sprites.
	
**********************************/

static void Subsector(subsector_t *sub)
{
	Word count;
	seg_t *line;
	sector_t *CurrentSector;
	
	CurrentSector = sub->sector;	/* Get the front sector */	
	SpritePrep(CurrentSector);			/* Prepare sprites for rendering */
	count = sub->numsublines;	/* Number of line to process */
	line = sub->firstline;		/* Get pointer to the first line */
	do {
		AddLine(line,CurrentSector);		/* Render each line */
		++line;				/* Inc the line pointer */
	} while (--count);		/* All done? */
}

/**********************************

	Check if any part of the BSP bounding box is touching the view arc. 
	Also project the width of the box to screen coords to see if it is too
	small to even bother with.
	If I should process this box then return TRUE.
	
**********************************/

static Word CheckBBox(Fixed *bspcoord)
{
	angle_t	angle1,angle2;	/* Left and right angles for view */

/* Find the corners of the box that define the edges from current viewpoint */
	
	{		/* Use BoxPtr */
	Word *BoxPtr;			/* Pointer to bspcoord offset table */
	BoxPtr = &checkcoord[0][0];		/* Init to the base of the table (Above) */
	if (viewy < bspcoord[BOXTOP]) {	/* Off the top? */
		BoxPtr+=12;					/* Index to center */
		if (viewy <= bspcoord[BOXBOTTOM]) {	/* Off the bottom? */
			BoxPtr += 12;			/* Index to below */
		}
	}

	if (viewx > bspcoord[BOXLEFT]) {	/* Check if off the left edge */
		BoxPtr+=4;					/* Center x */
		if (viewx >= bspcoord[BOXRIGHT]) {	/* Is it off the right? */
			BoxPtr+=4;
		}
	}
	if (BoxPtr[0]==-1) {		/* Center node? */
		return TRUE;	/* I am in the center of the box, process it!! */
	}
	
	
/* I now have in 3 Space the endpoints of the BSP box, now project it to the screen */
/* and see if it is either off the screen or too small to even care about */

	angle1 = PointToAngle(viewx,viewy,bspcoord[BoxPtr[0]],bspcoord[BoxPtr[1]]) - viewangle;	/* What is the projected angle? */
	angle2 = PointToAngle(viewx,viewy,bspcoord[BoxPtr[2]],bspcoord[BoxPtr[3]]) - viewangle;	/* Now the rightmost angle */
	}		/* End use of BoxPtr */

	{		/* Use span and tspan */
	angle_t span;		/* Degrees of span for the view */
	angle_t tspan;		/* Temp */

	span = angle1 - angle2;	/* What is the span of the angle? */
	if (span >= ANG180) {	/* Whoa... I must be sitting on the line or it's in my face! */
		return TRUE;	/* Process this one... */
	}
	
	/* angle1 must be treated as signed, so to see if it is either >-clipangle and < clipangle */
	/* I add clipangle to the angle to adjust the 0 center and compare to clipangle * 2 */
	
	tspan = angle1+clipangle;
	if (tspan > doubleclipangle) {	/* Possibly off the left edge */
		tspan -= doubleclipangle;
		if (tspan >= span) {		/* Off the left side? */
			return FALSE;	/* Don't bother, it's off the left side */
		}
		angle1 = clipangle;	/* Clip the left edge */
	}
	
	tspan = clipangle - angle2;		/* Move from a zero base of "clipangle" */
	if (tspan > doubleclipangle) {	/* Possible off the right edge */
		tspan -= doubleclipangle;
		if (tspan >= span) {	/* The entire span is off the right edge? */
			return FALSE;			/* Too far right! */
		}
		angle2 = -(int)clipangle;	/* Clip the right edge angle */
	}

/* See if any part of the contained area could be visible */

	angle1 = (angle1+ANG90)>>(ANGLETOFINESHIFT+1);	/* Rotate 90 degrees and make table index */
	angle2 = (angle2+ANG90)>>(ANGLETOFINESHIFT+1);
	} /* End use of span and tspan */
	
	angle1 = viewangletox[angle1];		/* Get the screen coords */
	angle2 = viewangletox[angle2];
	if (angle1 == angle2) {				/* Is the run too small? */
		return FALSE;				/* Don't bother rendering it then */
	}
	--angle2;
	{	/* Use start */
	
	cliprange_t	*SolidPtr;	/* Pointer to range */
	SolidPtr = solidsegs;	/* Index to the solid walls */
	if (SolidPtr->RightX < (int)angle2) {		/* Scan through the sorted list */
		do {
			++SolidPtr;		/* Next entry */
		} while (SolidPtr->RightX< (int)angle2);
	}
	if ((int)angle1 >= SolidPtr->LeftX && (int)angle2 <= SolidPtr->RightX) {
		return FALSE;	/* This block is behind a solid wall! */
	}
	return TRUE;		/* Process me! */
	}	/* End use of start */
}

/**********************************

	Traverse the BSP tree starting from a tree node (Or sector)
	and recursively subdivide if needed.
	Use a cross product from the line cast from the viewxy to the bspxy
	and the bsp line itself.

**********************************/

static void RenderBSPNode(node_t *bsp)
{
	Word Side;
	if ((Word)bsp & 1) {		/* Is this a BSP sector node? */
		Subsector((subsector_t *)(((Byte *)bsp)-1));	/* Process the sector */
		return;			/* Exit */
	}

/* Decide which side the view point is on */

	Side = PointOnVectorSide(viewx,viewy,&bsp->Line);	/* Is this the front side? */
	RenderBSPNode((node_t *)bsp->Children[Side]);	/* Process the side closer to me */
	Side ^= 1;			/* Swap the side */
	if (CheckBBox(bsp->bbox[Side])) {		/* Is the viewing rect on both sides? */
		RenderBSPNode((node_t *)bsp->Children[Side]);	/* Render the back side */
	}
}

/**********************************

	Find all walls that can be rendered in the current view plane.
	I make it handle the whole screen by placing fake posts
	on the farthest left and right sides in solidsegs 0 and 1.
	
**********************************/

void BSP(void)
{
	++validcount;					/* For sprite recursion */
	solidsegs[0].LeftX = -0x4000;	/* Fake leftmost post */
	solidsegs[0].RightX = -1;
	solidsegs[1].LeftX = ScreenWidth;	/* Fake rightmost post */
	solidsegs[1].RightX = 0x4000;
	newend = solidsegs+2;			/* Init the free memory pointer */
	RenderBSPNode(FirstBSPNode);	/* Begin traversing the BSP tree for all walls in render range */
	SortAllSprites();			/* Sort the sprites from front to back */
}


