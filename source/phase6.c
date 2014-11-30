#include "Doom.h"
#include <IntMath.h>

#define OPENMARK ((MAXSCREENHEIGHT-1)<<8)

/**********************************

	This code will draw all the VERTICAL walls for
	a screen.

	Clip values are the solid pixel bounding the range
	floorclip starts out ScreenHeight
	ceilingclip starts out -1
	clipbounds[] = (ceilingclip+1)<<8 + floorclip

**********************************/

static Word clipboundtop[MAXSCREENWIDTH];		/* Bounds top y for vertical clipping */
static Word clipboundbottom[MAXSCREENWIDTH];	/* Bounds bottom y for vertical clipping */

typedef struct {
	Byte *data;			/* Pointer to raw texture data */
	Word width;			/* Width of texture in pixels */
	Word height;		/* Height of texture in pixels */
	int topheight;		/* Top texture height in global pixels */
	int bottomheight;	/* Bottom texture height in global pixels */
	Word texturemid;	/* Anchor point for texture */
} drawtex_t;

static drawtex_t toptex;		/* Describe the upper texture */
static drawtex_t bottomtex;		/* Describe the lower texture */

Word tx_x;			/* Screen x coord being drawn */
int tx_scale;		/* True scale value 0-0x7FFF */
static Word tx_texturecolumn;	/* Column offset into source image */


/**********************************

	Calculate texturecolumn and iscale for the rendertexture routine

**********************************/

static void DrawTexture(drawtex_t *tex)
{
	int top;
	Word Run;
	Word colnum;	/* Column in the texture */
	LongWord frac;

	Run = (tex->topheight-tex->bottomheight)>>HEIGHTBITS;	/* Source image height */
	if ((int)Run<=0) {		/* Invalid? */
		return;
	}
	top = CenterY-((tx_scale*tex->topheight)>>(HEIGHTBITS+SCALEBITS));	/* Screen Y */

	colnum = tx_texturecolumn;	/* Get the starting column offset */
	frac = tex->texturemid - (tex->topheight<<FIXEDTOHEIGHT);	/* Get the anchor point */
	frac >>= FRACBITS;
	while (frac&0x8000) {
		--colnum;
		frac += tex->height;		/* Make sure it's on the shape */
	}
	frac&=0x7f;		/* Zap unneeded bits */
	colnum &= (tex->width-1);		/* Wrap around the texture */
	colnum = (colnum*tex->height)+frac;	/* Index to the shape */
	DrawWallColumn(top,colnum,tex->data,Run);	/* Project it */
}

/**********************************

	Draw a single wall texture.
	Also save states for pending ceiling, floor and future clipping

**********************************/

static void DrawSeg(viswall_t *segl)
{
	register Word x;		/* Current x coord */
	register int scale;
	int _scalefrac;
	Word ActionBits;

	ActionBits = segl->WallActions;
	if (ActionBits & (AC_TOPTEXTURE|AC_BOTTOMTEXTURE)) {
		
		x = segl->seglightlevel;
		lightmin = lightmins[x];
		lightmax = x;
		lightsub = lightsubs[x];
		lightcoef = lightcoefs[x];
			
		if (ActionBits&AC_TOPTEXTURE) {		/* Is there a top wall? */
			texture_t *tex;
			toptex.topheight = segl->t_topheight;	/* Init the top texture */
			toptex.bottomheight = segl->t_bottomheight;
			toptex.texturemid = segl->t_texturemid;
			tex = segl->t_texture;
			toptex.width = tex->width;
			toptex.height = tex->height;
			toptex.data = (Byte *)*tex->data;
		}
		if (ActionBits&AC_BOTTOMTEXTURE) {	/* Is there a bottom wall? */
			texture_t *tex;
			bottomtex.topheight = segl->b_topheight;
			bottomtex.bottomheight = segl->b_bottomheight;
			bottomtex.texturemid = segl->b_texturemid;
			tex = segl->b_texture;
			bottomtex.width = tex->width;
			bottomtex.height = tex->height;
			bottomtex.data = (Byte *)*tex->data;
		}
		_scalefrac = segl->LeftScale;		/* Init the scale fraction */
		
		x = segl->LeftX;				/* Init the x coord */
		do {							/* Loop for each X coord */
			scale = _scalefrac>>FIXEDTOSCALE;	/* Current scaling factor */
			if (scale >= 0x2000) {		/* Too large? */
				scale = 0x1fff;			/* Fix the scale to maximum */
		}
			tx_x = x;		/* Pass the X coord */

/* Calculate texture offset into shape */

			tx_texturecolumn = (segl->offset-IMFixMul(
				finetangent[(segl->CenterAngle+xtoviewangle[x])>>ANGLETOFINESHIFT],
				segl->distance))>>FRACBITS;
			tx_scale = scale;			/* 0-0x1FFF */
			{
			int texturelight;
			texturelight = ((scale*lightcoef)>>16) - lightsub;
			if (texturelight < lightmin) {
				texturelight = lightmin;
			}
			if (texturelight > lightmax) {
				texturelight = lightmax;
			}
			tx_texturelight = texturelight;
			}
			if (ActionBits&AC_TOPTEXTURE) {
				DrawTexture(&toptex);		/* Draw upper texture */
			}
			if (ActionBits&AC_BOTTOMTEXTURE) {
				DrawTexture(&bottomtex);	/* Draw lower texture */
			}
			_scalefrac += segl->ScaleStep;		/* Step to the next scale */
		} while (++x<=segl->RightX);
	}
}

/**********************************

	Given a span of pixels, see if it is already defined
	in a record somewhere. If it is, then merge it otherwise
	make a new plane definition.

**********************************/

static visplane_t *FindPlane(visplane_t *check,Fixed height,void **PicHandle,int start,int stop,Word Light)
{
	Word i;
	Word j;
	Word *set;

	++check;		/* Automatically skip to the next plane */
	if (check<lastvisplane) {
		do {
			if (height == check->height &&		/* Same plane as before? */
				PicHandle == check->PicHandle &&
				Light == check->PlaneLight &&
				check->open[start] == OPENMARK) {	/* Not defined yet? */
				if (start < check->minx) {	/* In range of the plane? */
					check->minx = start;	/* Mark the new edge */
				}
				if (stop > check->maxx) {
					check->maxx = stop;		/* Mark the new edge */
				}
				return check;			/* Use the same one as before */
			}
		} while (++check<lastvisplane);
	}
	
/* make a new plane */
	
	check = lastvisplane;
	++lastvisplane;
	check->height = height;		/* Init all the vars in the visplane */
	check->PicHandle = PicHandle;
	check->minx = start;
	check->maxx = stop;
	check->PlaneLight = Light;		/* Set the light level */

/* Quickly fill in the visplane table */

	i = OPENMARK;
	set = check->open;	/* A brute force method to fill in the visplane record FAST! */
	j = ScreenWidth/8;
	do {
		set[0] = i;
		set[1] = i;
		set[2] = i;
		set[3] = i;
		set[4] = i;
		set[5] = i;
		set[6] = i;
		set[7] = i;
		set+=8;
	} while (--j);
	return check;
}


/**********************************

	Do a fake wall rendering so I can get all the visplane records.
	This is a fake-o routine so I can later draw the wall segments from back to front.

**********************************/

static void SegLoop(viswall_t *segl)
{
	register Word x;		/* Current x coord */
	register int scale;
	register int _scalefrac;
	register Word ActionBits;
	visplane_t *FloorPlane,*CeilingPlane;
	int ceilingclipy,floorclipy;
	
	_scalefrac = segl->LeftScale;		/* Init the scale fraction */

			/* visplanes[0] is zero to force a FindPlane on the first pass */
			
	FloorPlane = CeilingPlane = visplanes;		/* Reset the visplane pointers */
	ActionBits = segl->WallActions;
	x = segl->LeftX;				/* Init the x coord */
	do {							/* Loop for each X coord */
		scale = _scalefrac>>FIXEDTOSCALE;	/* Current scaling factor */
		if (scale >= 0x2000) {		/* Too large? */
			scale = 0x1fff;			/* Fix the scale to maximum */
		}
		ceilingclipy = clipboundtop[x];	/* Get the top y clip */
		floorclipy = clipboundbottom[x];	/* Get the bottom y clip */

/* Shall I add the floor? */

		if (ActionBits & AC_ADDFLOOR) {
			int top,bottom;
			top = CenterY-((scale*segl->floorheight)>>(HEIGHTBITS+SCALEBITS));	/* Y coord of top of floor */
			if (top <= ceilingclipy) {
				top = ceilingclipy+1;		/* Clip the top of floor to the bottom of the visible area */
			}
			bottom = floorclipy-1;		/* Draw to the bottom of the screen */
			if (top <= bottom) {		/* Valid span? */
				if (FloorPlane->open[x] != OPENMARK) {	/* Not already covered? */
					FloorPlane = FindPlane(FloorPlane,segl->floorheight,
						segl->FloorPic,x,segl->RightX,segl->seglightlevel);
				}
				if (top) {
					--top;
				}
				FloorPlane->open[x] = (top<<8)+bottom;	/* Set the new vertical span */
			}
		}

/* Handle ceilings */

		if (ActionBits & AC_ADDCEILING) {
			int top,bottom;
			top = ceilingclipy+1;		/* Start from the ceiling */
			bottom = CenterY-1-((scale*segl->ceilingheight)>>(HEIGHTBITS+SCALEBITS));	/* Bottom of the height */
			if (bottom >= floorclipy) {		/* Clip the bottom? */
				bottom = floorclipy-1;
			}
			if (top <= bottom) {
				if (CeilingPlane->open[x] != OPENMARK) {		/* Already in use? */
					CeilingPlane = FindPlane(CeilingPlane,segl->ceilingheight,
						segl->CeilingPic,x,segl->RightX,segl->seglightlevel);
				}
				if (top) {
					--top;
				}
				CeilingPlane->open[x] = (top<<8)+bottom;		/* Set the vertical span */
			}
		}

/* Sprite clip sils */

		if (ActionBits & (AC_BOTTOMSIL|AC_NEWFLOOR)) {
			int low;
			low = CenterY-((scale*segl->floornewheight)>>(HEIGHTBITS+SCALEBITS));
			if (low > floorclipy) {
				low = floorclipy;
			}
			if (low < 0) {
				low = 0;
			}
			if (ActionBits & AC_BOTTOMSIL) {
				segl->BottomSil[x] = low;
			} 
			if (ActionBits & AC_NEWFLOOR) {
				clipboundbottom[x] = low;
			}
		}

		if (ActionBits & (AC_TOPSIL|AC_NEWCEILING)) {
			int high;
			high = (CenterY-1)-((scale*segl->ceilingnewheight)>>(HEIGHTBITS+SCALEBITS));
			if (high < ceilingclipy) {
				high = ceilingclipy;
			}
			if (high > (int)ScreenHeight-1) {
				high = ScreenHeight-1;
			}
			if (ActionBits & AC_TOPSIL) {
				segl->TopSil[x] = high+1;
			}
			if (ActionBits & AC_NEWCEILING) {
				clipboundtop[x] = high;
			}
		}

/* I can draw the sky right now!! */

		if (ActionBits & AC_ADDSKY) {
			int bottom;
			bottom = CenterY-((scale*segl->ceilingheight)>>(HEIGHTBITS+SCALEBITS));
			if (bottom > floorclipy) {
				bottom = floorclipy;
			}
			if ((ceilingclipy+1) < bottom) {		/* Valid? */
				tx_x = x;		/* Pass the X coord */
				DrawSkyLine();	/* Draw the sky */
			}
		}
		_scalefrac += segl->ScaleStep;		/* Step to the next scale */
	} while (++x<=segl->RightX);
}

/**********************************

	Draw all the sprites from back to front.
	
**********************************/

static void DrawSprites(void)
{
	Word i;
	Word *LocalPtr;
	vissprite_t *VisPtr;
	
	i = SpriteTotal;	/* Init the count */
	if (i) {		/* Any sprites to speak of? */
		LocalPtr = SortedSprites;	/* Get the pointer to the sorted array */
		VisPtr = vissprites;	/* Cache pointer to sprite array */
		do {
			DrawVisSprite(&VisPtr[*LocalPtr++&0x7F]);	/* Draw from back to front */
		} while (--i);
	}
} 

/**********************************

	Follow the list of walls and draw each
	and every wall fragment.
	Note : I draw the walls closest to farthest and I maintain a ZBuffet

**********************************/

void SegCommands(void)
{
	{
	Word i;		/* Temp index */
	viswall_t *WallSegPtr;		/* Pointer to the current wall */
	viswall_t *LastSegPtr;
	
	
	WallSegPtr = viswalls;		/* Get the first wall segment to process */
	LastSegPtr = lastwallcmd;	/* Last one to process */
	if (LastSegPtr == WallSegPtr) {	/* No walls to render? */
		return;				/* Exit now!! */
	}

	EnableHardwareClipping();		/* Turn on all hardware clipping to remove slop */
	
	i = 0;		/* Init the vertical clipping records */
	do {
		clipboundtop[i] = -1;		/* Allow to the ceiling */
		clipboundbottom[i] = ScreenHeight;	/* Stop at the floor */
	} while (++i<ScreenWidth);

	/* Process all the wall segments */

	do {
		SegLoop(WallSegPtr);			/* Create the viswall records and draw the sky only */
	} while (++WallSegPtr<LastSegPtr);	/* Next wall in chain */
	
	/* Now I actually draw the walls back to front to allow for clipping because of slop */
	
	LastSegPtr = viswalls;		/* Stop at the last one */
	do {
		--WallSegPtr;			/* Last go backwards!! */
		DrawSeg(WallSegPtr);		/* Draw the wall (Only if needed) */
	} while (WallSegPtr!=LastSegPtr);	/* All done? */
}

	/* Now we draw all the planes. They are already clipped and create no slop! */
{	
	visplane_t *PlanePtr;
	visplane_t *LastPlanePtr;
	Word WallScale;
		
	PlanePtr = visplanes+1;		/* Get the range of pointers */
	LastPlanePtr = lastvisplane;
	
	if (PlanePtr!=LastPlanePtr) {	/* No planes generated? */
		planey = -viewy;		/* Get the Y coord for camera */
		WallScale = (viewangle-ANG90)>>ANGLETOFINESHIFT;	/* left to right mapping */
		basexscale = (finecosine[WallScale] / ((int)ScreenWidth/2));
		baseyscale = -(finesine[WallScale] / ((int)ScreenWidth/2));
		do {
			DrawVisPlane(PlanePtr);		/* Convert the plane */
		} while (++PlanePtr<LastPlanePtr);		/* Loop for all */
	}
}
	DisableHardwareClipping();		/* Sprites require full screen management */
	DrawSprites();					/* Draw all the sprites (ZSorted and clipped) */
}
