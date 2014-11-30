#include "Doom.h"
#include <IntMath.h>

#define SCREENGUNY -40		/* Y offset to center the player's weapon properly */

Word spropening[MAXSCREENWIDTH];		/* clipped range */

/**********************************

	Perform a merge sort of two arrays of words.
	Source1 and Source2 will be placed into Dest.
	Count1 and Count2 are assumed to be NOT zero.
	I will sort with the lowest value first, to the highest.
	
	Optimized for the ARM 6 microprocessor, your mileage may vary.

**********************************/

static void Merge(Word *Source1,Word Count1,Word *Source2,Word Count2,Word *Dest)
{
	Word Cache;
	Word Cache2;
	
	if (*Source1 < *Source2) {		/* Do the initial compare for the sort */
mergefrom1:
		Cache = *Source2;		/* Cache into a register */
		Cache2 = *Source1;
		do {
			*Dest++ = Cache2;
			if (!--Count1) {	
				goto Finish2;	/* Finish the list #2 */
			}
			Cache2 = *++Source1;
		} while (Cache2 < Cache);
	}
	Cache = *Source1;
	Cache2 = *Source2;
	do {
		*Dest++ = Cache2;
		if (!--Count2) {		/* Do this way to avoid killing the pipeline! */
			goto Finish1;
		}
		Cache2 = *++Source2;
	} while (Cache2 < Cache);	
	goto mergefrom1;

Finish2:			/* List #1 is empty, so I will copy the rest of list #2 */
	*Dest++ = Cache;
	if (--Count2) {
		do {
			*Dest++ = *++Source2;
		} while (--Count2);
	}
	return;			/* Bye! */
	
Finish1:			/* List #2 is empty, so I will copy the rest of list #1 */
	*Dest++ = Cache;
	if (--Count1) {
		do {
			*Dest++ = *++Source1;	/* Copy an entry */
		} while (--Count1);			/* Count down */
	}
}

/**********************************

	Perform a merge sort of two arrays of words.
	Start with a group of single entry lists and work your way until 
	the whole mess is sorted.
	
	Calls Merge to perform the dirty work.
	
	Return a pointer to the sorted buffer

**********************************/

Word *SortWords(Word *Before,Word *After,Word Total)
{
	Word ChunkSize;		/* Number of elements to sort */
	Word LoopCount;		/* Number of times loop has executed */
    
	if (Total<2) {			/* Already sorted? */
		return Before;		/* Exit now */
	}
	ChunkSize = 1;		/* Size of each list (Start with 1) */
	LoopCount = 1;		/* Number of times executed */
		
	for (;;) {
		Word *List1Ptr;		/* First list pointer */
		Word *List2Ptr;		/* Second list pointer */
		Word *DestPtr;		/* Dest buffer pointer */
		{
		Word ChunkCount;	/* Number of large chunks to merge */
		ChunkCount = Total>>LoopCount;	/* Number of chunks */
				
		List1Ptr = Before;	/* First list */
		List2Ptr = Before + (ChunkCount <<  (LoopCount-1));	/* Second list */
		DestPtr = After;
		
		/* Merge sort the large blocks */
		
		if (ChunkCount) {		/* Any large blocks? */
			do {
				Merge(List1Ptr,ChunkSize,List2Ptr,ChunkSize,DestPtr);
				List1Ptr+=ChunkSize;	/* Inc the pointers */
				List2Ptr+=ChunkSize;
				DestPtr+=ChunkSize<<1;
			} while (--ChunkCount);	/* Any more? */
		}
		}
		
		/* Copy or merge the remaining chunk fragment */
		
		{
		Word Remainder;
		Remainder = Total&((ChunkSize<<1)-1);
		if (Remainder) {
			if (Remainder > ChunkSize) {	
				Merge(List2Ptr,ChunkSize,List2Ptr+ChunkSize,Remainder - ChunkSize,DestPtr);
			} else  {	/* Copy the rest then... */
				do {
					*DestPtr++ = *List2Ptr++;
				} while (--Remainder);
			}
		}
		}
		
		/* Am I done yet? */

		ChunkSize <<= 1;		/* Next block size */
		if (ChunkSize >= Total) {	/* All done? */
			return After;		/* Exit */
		}
		++LoopCount;		/* Next loop count */
		{
		Word *Temp;
		Temp = Before;		/* Swap the pointers to the buffers */
		Before = After;
		After = Temp;
		}
	}
}

/**********************************

	Using a point in space, determine if it is BEHIND a wall.
	Use a cross product to determine facing.
	
**********************************/

static Word SegBehindPoint(viswall_t *ds,Fixed dx,Fixed dy)
{
	Fixed x1,y1;
	Fixed sdx,sdy;
	seg_t *SegPtr;
	
	SegPtr = ds->SegPtr;
	
	x1 = SegPtr->v1.x;
	y1 = SegPtr->v1.y;
	
	sdx = SegPtr->v2.x-x1;
	sdy = SegPtr->v2.y-y1;
	
	dx -= x1;
	dy -= y1;
	
	sdx>>=FRACBITS;
	sdy>>=FRACBITS;
	dx>>=FRACBITS;
	dy>>=FRACBITS;
	
	dx*=sdy;
	sdx*=dy;
	if (sdx<dx) {
		return TRUE;
	} 
	return FALSE;
}

/**********************************

	See if a sprite needs clipping and if so, then draw it clipped

**********************************/

void DrawVisSprite(vissprite_t *vis)
{
	viswall_t *ds;
	int	x, r1, r2;
	int	silhouette;
	int	x1, x2;
	Byte *topsil,*bottomsil;
	Word opening;
	int	top, bottom;
	Word scalefrac;
	Word Clipped;

	x1 = vis->x1;		/* Get the sprite's screen posts */
	x2 = vis->x2;
	if (x1<0) {			/* These could be offscreen */
		x1 = 0;
	}
	if (x2>=(int)ScreenWidth) {
		x2 = ScreenWidth-1;
	}
	scalefrac = vis->yscale;	/* Get the Z scale */
	
	Clipped = FALSE;		/* Assume I don't clip */

/* scan drawsegs from end to start for obscuring segs */
/* the first drawseg that has a greater scale is the clip seg */


	ds = lastwallcmd;
	do {
		--ds;			/* Point to the next wall command */
		
		/* determine if the drawseg obscures the sprite */

		if (ds->LeftX > x2 || ds->RightX < x1 ||
			ds->LargeScale <= scalefrac ||
			!(ds->WallActions&(AC_TOPSIL|AC_BOTTOMSIL|AC_SOLIDSIL)) ) {
			continue;			// doesn't cover sprite
		}

		if (ds->SmallScale<=scalefrac) {	/* In range of the wall? */
			if (SegBehindPoint(ds,vis->thing->x,vis->thing->y)) {
				continue;			/* Wall seg is behind sprite */
			}
		}
		if (!Clipped) {		/* Never initialized? */
			Clipped = TRUE;
			x = x1;
			opening = ScreenHeight;
			do {
				spropening[x] = opening;		/* Init the clip table */
			} while (++x<=x2);
		}
		r1 = ds->LeftX < x1 ? x1 : ds->LeftX;		/* Get the clip bounds */
		r2 = ds->RightX > x2 ? x2 : ds->RightX;

/* clip this piece of the sprite */

		silhouette = ds->WallActions & (AC_TOPSIL|AC_BOTTOMSIL|AC_SOLIDSIL);
		x=r1;
		if (silhouette == AC_SOLIDSIL) {
			opening = ScreenHeight<<8;
			do {
				spropening[x] = opening;		/* Clip these to blanks */
			} while (++x<=r2);
			continue;
		}
		
		topsil = ds->TopSil;
		bottomsil = ds->BottomSil;

		if (silhouette == AC_BOTTOMSIL) {	// bottom sil only
			do {
				opening = spropening[x];
				if ( (opening&0xff) == ScreenHeight) {
					spropening[x] = (opening&0xff00) + bottomsil[x];
				}
			} while (++x<=r2);
		} else if (silhouette == AC_TOPSIL) {	// top sil only
			do {
				opening = spropening[x];
				if ( !(opening&0xff00)) {
					spropening[x] = (topsil[x]<<8) + (opening&0xff);
				}
			} while (++x<=r2);
		} else if (silhouette == (AC_TOPSIL|AC_BOTTOMSIL) ) {	// both
			do {
				top = spropening[x];
				bottom = top&0xff;
				top >>= 8;
				if (bottom == ScreenHeight) {
					bottom = bottomsil[x];
				}
				if (!top) {
					top = topsil[x];
				}
				spropening[x] = (top<<8)+bottom;
			} while (++x<=r2);
		}
	} while (ds!=viswalls);
	
	/* Now that I have created the clip regions, let's see if I need to do this */
	
	if (!Clipped) {			/* Do I have to clip at all? */
		DrawSpriteNoClip(vis);		/* Draw it using no clipping at all */
		return;		/* Exit */
	}
	
	/* Check the Y bounds to see if the clip rect even touches the sprite */
	
	r1 = vis->y1;
	r2 = vis->y2;
	if (r1<0) {
		r1 = 0;		/* Clip to screen coords */
	}
	if (r2>=(int)ScreenHeight) {
		r2 = ScreenHeight;
	}
	x = x1;
	do {
		top = spropening[x];
		if (top!=ScreenHeight) {		/* Clipped? */
			bottom = top&0xff;
			top >>=8;
			if (r1<top || r2>=bottom) {	/* Needs manual clipping! */
				if (x!=x1) {		/* Was any part visible? */
					DrawSpriteClip(x1,x2,vis);	/* Draw it and exit */
					return;
				}
				do {
					top = spropening[x];
					if (top!=(ScreenHeight<<8)) {
						bottom = top&0xff;
						top >>=8;
						if (r1<bottom && r2>=top) {	/* Is it even visible? */
							DrawSpriteClip(x1,x2,vis);		/* Draw it */
							return;
						}
					}
				} while (++x<=x2);
				return;		/* It's not visible at all!! */
			}
		}
	} while (++x<=x2);
	DrawSpriteNoClip(vis);		/* It still didn't need clipping!! */
}

/**********************************

	Draw a single weapon or muzzle flash on the screen

**********************************/

static void DrawAWeapon(pspdef_t *psp,Word Shadow)
{
	Short *Input;		/* Pointer to the xy offset'd shape */
//	Word Color;
	Word RezNum;
	int x,y;
	state_t *StatePtr;

	StatePtr = psp->StatePtr;		/* Get the state struct pointer */
	RezNum = StatePtr->SpriteFrame>>FF_SPRITESHIFT;	/* Get the file */
	Input = (Short *)LoadAResource(RezNum);	/* Get the main pointer */
	Input = (Short *)GetShapeIndexPtr(Input,StatePtr->SpriteFrame & FF_FRAMEMASK);
	
	((LongWord *)Input)[7] = GunXScale;		/* Set the scale factor */
	((LongWord *)Input)[10] = GunYScale;
	if (Shadow) {
		((LongWord *)Input)[13] = 0x9C81;	/* Set the shadow bits */
	} else {
		((LongWord *)Input)[13] = 0x1F00;	/* Normal PMode */
#if 0
		if (StatePtr->SpriteFrame & FF_FULLBRIGHT) {
			Color = 255;			/* Full bright */
		} else {					/* Ambient light */
			Color = players.mo->subsector->sector->lightlevel;
		}
#endif
	}
	x = Input[0];
	y = Input[1];
	x = ((psp->WeaponX+x)*(int)GunXScale)>>20;
	y = ((psp->WeaponY+SCREENGUNY+y)*(int)GunYScale)>>16;
	x+=ScreenXOffset;
	y+=ScreenYOffset+2;			/* Add 2 pixels to cover up the hole in the bottom */
	DrawMShape(x,y,&Input[2]);	/* Draw the weapon's shape */
	ReleaseAResource(RezNum);
}

/**********************************

	Draw the player's weapon in the foreground

**********************************/

void DrawWeapons(void)
{
	Word i;
	Word Shadow;		/* Flag for shadowing */
	pspdef_t *psp;
	
	psp = players.psprites;	/* Get the first sprite in the array */ 
	Shadow = FALSE;			/* Assume no shadow draw mode */
	if (players.mo->flags & MF_SHADOW) {	/* Could be active? */
		i = players.powers[pw_invisibility];	/* Get flash time */
		if (i>=(5*TICKSPERSEC) || i&0x10) {	/* On a long time or if flashing... */
			Shadow = TRUE;		/* Draw as a shadow right now */
		}
	}
	i = 0;		/* Init counter */
	do {
		if (psp->StatePtr) {		/* Valid state record? */
			DrawAWeapon(psp,Shadow);	/* Draw the weapon */
		}
		++psp;		/* Next... */
	} while (++i<NUMPSPRITES);	/* All done? */
	
	i = ScreenSize+rBACKGROUNDMASK;		/* Get the resource needed */
	DrawMShape(0,0,LoadAResource(i));	/* Draw the border */
	ReleaseAResource(i);				/* Release the resource */
}

