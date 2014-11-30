#include "Doom.h"
#include <IntMath.h>

#define OPENMARK ((MAXSCREENHEIGHT-1)<<8)

Byte *PlaneSource;			/* Pointer to image of floor/ceiling texture */
Fixed planey;		/* latched viewx / viewy for floor drawing */
Fixed basexscale,baseyscale;
Word PlaneDistance;
static Word PlaneHeight;
static Word spanstart[MAXSCREENHEIGHT];

/**********************************

basexscale
baseyscale
planey

	This is the basic primitive to draw horizontal lines quickly
	
**********************************/

static void MapPlane(Word x2,Word y)
{
	angle_t	angle;
	Word distance;
	Fixed length;
	Fixed xfrac,yfrac,xstep,ystep;
	Word x1;

// planeheight is 10.6
// yslope is 6.10, distscale is 1.15
// distance is 12.4
// length is 11.5

	x1 = spanstart[y];
	distance = (yslope[y]*PlaneHeight)>>12;	/* Get the offset for the plane height */
	length = (distscale[x1]*distance)>>14;
	angle = (xtoviewangle[x1]+viewangle)>>ANGLETOFINESHIFT;

/* xfrac, yfrac, xstep, ystep */

	xfrac = (((finecosine[angle]>>1)*length)>>4)+viewx;
	yfrac = planey - (((finesine[angle]>>1)*length)>>4);

	xstep = ((Fixed)distance*basexscale)>>4;
	ystep = ((Fixed)distance*baseyscale)>>4;

	length = lightcoef/(Fixed)distance - lightsub;
	if (length < lightmin) {
		length = lightmin;
	}
	if (length > lightmax) {
		length = lightmax;
	}
	tx_texturelight = length;
	DrawFloorColumn(y,x1,x2-x1,xfrac,yfrac,xstep,ystep);
}

/**********************************
	
	Draw a plane by scanning the open records. 
	The open records are an array of top and bottom Y's for
	a graphical plane. I traverse the array to find out the horizontal
	spans I need to draw. This is a bottleneck routine.

**********************************/

void DrawVisPlane(visplane_t *p)
{
	register Word x;
	Word stop;
	Word oldtop;
	register Word *open;

	PlaneSource = (Byte *)*p->PicHandle;	/* Get the base shape index */
	x = p->height;
	if ((int)x<0) {
		x = -x;
	}
	PlaneHeight = x;
	
	stop = p->PlaneLight;
	lightmin = lightmins[stop];
	lightmax = stop;
	lightsub = lightsubs[stop];
	lightcoef = planelightcoef[stop];
	
	stop = p->maxx+1;	/* Maximum x coord */
	x = p->minx;		/* Starting x */
	open = p->open;		/* Init the pointer to the open Y's */
	oldtop = OPENMARK;	/* Get the top and bottom Y's */
	open[stop] = oldtop;	/* Set posts to stop drawing */

	do {
		Word newtop;
		newtop = open[x];		/* Fetch the NEW top and bottom */
		if (oldtop!=newtop) {
			Word PrevTopY,NewTopY;		/* Previous and dest Y coords for top line */
			Word PrevBottomY,NewBottomY;	/* Previous and dest Y coords for bottom line */
			PrevTopY = oldtop>>8;		/* Starting Y coords */
			PrevBottomY = oldtop&0xFF;
			NewTopY = newtop>>8;
			NewBottomY = newtop&0xff;
		
			/* For lines on the top, check if the entry is going down */
			
			if (PrevTopY < NewTopY && PrevTopY<=PrevBottomY) {	/* Valid? */
				register Word Count;
					
				Count = PrevBottomY+1;	/* Convert to < */
				if (NewTopY<Count) {	/* Use the lower */
					Count = NewTopY;	/* This is smaller */
				}
				do {
					MapPlane(x,PrevTopY);		/* Draw to this x */
				} while (++PrevTopY<Count);	/* Keep counting */
			}
			if (NewTopY < PrevTopY && NewTopY<=NewBottomY) {
				register Word Count;
				Count = NewBottomY+1;
				if (PrevTopY<Count) {
					Count = PrevTopY;
				}
				do {
					spanstart[NewTopY] = x;	/* Mark the starting x's */
				} while (++NewTopY<Count);
			}
		
			if (PrevBottomY > NewBottomY && PrevBottomY>=PrevTopY) {
				register int Count;
				Count = PrevTopY-1;
				if (Count<(int)NewBottomY) {
					Count = NewBottomY;
				}
				do {
					MapPlane(x,PrevBottomY);	/* Draw to this x */
				} while ((int)--PrevBottomY>Count);
			}
			if (NewBottomY > PrevBottomY && NewBottomY>=NewTopY) {
				register int Count;
				Count = NewTopY-1;
				if (Count<(int)PrevBottomY) {
					Count = PrevBottomY;
				}
				do {
					spanstart[NewBottomY] = x;		/* Mark the starting x's */
				} while ((int)--NewBottomY>Count);
			}
			oldtop=newtop;
		}
	} while (++x<=stop);
}

