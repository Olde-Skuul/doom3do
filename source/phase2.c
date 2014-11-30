#include "Doom.h"
#include <IntMath.h>

/**********************************

	Check all the visible walls and fill in all the "Blanks" such
	as texture pointers and sky hack variables.
	When finished all the viswall records are filled in.
	
**********************************/

static sector_t emptysector = { 0,0,-2,-2,-2 };	/* -2 floorpic, ceilingpic, light */

/**********************************

	Returns the texture mapping scale for the current line at the given angle
	rw_distance must be calculated first

**********************************/

static Fixed ScaleFromGlobalAngle(Fixed rw_distance,angle_t anglea,angle_t angleb)
{
	Fixed num,den;
	Fixed *SineTbl;

/* both sines are always positive */

	SineTbl = &finesine[ANG90>>ANGLETOFINESHIFT];
	den = SineTbl[anglea>>ANGLETOFINESHIFT];
	num = SineTbl[angleb>>ANGLETOFINESHIFT];

	num = IMFixMul(StretchWidth,num);
	den = IMFixMul(rw_distance,den);
	if (den > num>>16) {
		num = IMFixDiv(num,den);		/* Place scale in numerator */
		if (num < 64*FRACUNIT) {
			if (num >= 256) {
				return num;
			}
			return 256;		/* Minimum scale value */
		}
	}
	return 64*FRACUNIT;		/* Maximum scale value */
}

/**********************************

	Calculate the wall scaling constants

**********************************/

static void LatePrep(viswall_t *wc,seg_t *LineSeg,angle_t LeftAngle)
{
	angle_t normalangle;		/* Angle to wall */
	Fixed PointDistance;		/* Distance to end wall point */
	Fixed rw_distance;
	angle_t offsetangle;
	Fixed scalefrac;
	Fixed scale2;

//
// calculate normalangle and rw_distance for scale calculation
// and texture mapping
//

	normalangle = LineSeg->angle + ANG90;
	offsetangle = (normalangle - LeftAngle);
	if ((int)offsetangle < 0) {
		offsetangle = -offsetangle;
	}
	if (offsetangle > ANG90) {
		offsetangle = ANG90;
	}
	PointDistance = PointToDist(LineSeg->v1.x,LineSeg->v1.y);
	wc->distance = rw_distance = IMFixMul(PointDistance,
		finesine[(ANG90 - offsetangle)>>ANGLETOFINESHIFT]);

//
// calc scales
//

	offsetangle = xtoviewangle[wc->LeftX];
	scalefrac = scale2 = wc->LeftScale = ScaleFromGlobalAngle(rw_distance,
		offsetangle,(offsetangle+viewangle)-normalangle);
	if (wc->RightX > wc->LeftX) {
		offsetangle = xtoviewangle[wc->RightX];
		scale2 = ScaleFromGlobalAngle(rw_distance,offsetangle,
			(offsetangle+viewangle)-normalangle);
		wc->ScaleStep = (int)(scale2 - scalefrac) / (int)(wc->RightX-wc->LeftX);
	}
	wc->RightScale = scale2;
	
	if (scale2<scalefrac) {
		wc->SmallScale = scale2;
		wc->LargeScale = scalefrac;
	} else {
		wc->LargeScale = scale2;
		wc->SmallScale = scalefrac;
	}
	
	if (wc->WallActions & (AC_TOPTEXTURE|AC_BOTTOMTEXTURE) ) {
		offsetangle = normalangle - LeftAngle;
		if (offsetangle > ANG180) {
			offsetangle = -offsetangle;		/* Force unsigned */
		}
		if (offsetangle > ANG90) {
			offsetangle = ANG90;		/* Clip to maximum */			
		}
		scale2 = IMFixMul(PointDistance,finesine[offsetangle >>ANGLETOFINESHIFT]);
		if (normalangle - LeftAngle < ANG180) {
			scale2 = -scale2;		/* Reverse the texture anchor */
		}
		wc->offset += scale2;
		wc->CenterAngle = ANG90 + viewangle - normalangle;
	}
}

/**********************************

	Calculate the wall scaling constants

**********************************/

void WallPrep(Word LeftX,Word RightX,seg_t *LineSeg,angle_t LeftAngle)
{
	viswall_t *CurWallPtr;	/* Pointer to work record */
	Word LineFlags;			/* Render flags for current line */
	side_t *SidePtr;		/* Pointer to line side record */
	sector_t *FrontSecPtr;	/* Pointer to front facing sector */
	sector_t *BackSecPtr;	/* Pointer to rear sector (Or empty_sector if single sided) */
	Word actionbits;		/* Flags */
	Word f_ceilingpic;		/* Front sector ceiling image # */
	Word f_lightlevel;		/* Front sector light level */
	Fixed f_floorheight;	/* Front sector floor height - viewz */
	Fixed f_ceilingheight;	/* Front sector ceiling height - viewz */
	Word b_ceilingpic;		/* Back sector ceiling image # */
	Word b_lightlevel;		/* Back sector light level */
	Fixed b_floorheight, b_ceilingheight;

	CurWallPtr = lastwallcmd;		/* Get the first wall pointer */
	lastwallcmd = CurWallPtr+1;		/* Inc my pointer */
	CurWallPtr->LeftX = LeftX;		/* Set the edges of the visible wall */
	CurWallPtr->RightX = RightX;	/* Right is inclusive! */
	CurWallPtr->SegPtr = LineSeg;	/* For clipping */
	
	{
	line_t *LinePtr;
	LinePtr = LineSeg->linedef;		/* Get the line record */
	LineFlags = LinePtr->flags;		/* Copy flags into a global */
	LinePtr->flags = LineFlags|ML_MAPPED;	/* Mark as seen... */
	}
	
	SidePtr = LineSeg->sidedef;		/* Get the line side */
	FrontSecPtr = LineSeg->frontsector;	/* Get the front sector */
	f_ceilingpic = FrontSecPtr->CeilingPic;		/* Store into locals */
	f_lightlevel = FrontSecPtr->lightlevel;
	f_floorheight = FrontSecPtr->floorheight - viewz;	/* Adjust for camera z */
	f_ceilingheight = FrontSecPtr->ceilingheight - viewz;
	
	/* Set the floor and ceiling shape handles */
	
	CurWallPtr->FloorPic = FlatTranslation[FrontSecPtr->FloorPic];	/* Store the floor shape */
	if (f_ceilingpic == -1) {
		CurWallPtr->CeilingPic = 0;		/* Set a null handle */
	} else {
		CurWallPtr->CeilingPic = FlatTranslation[f_ceilingpic];	/* Normal image */
	}

	BackSecPtr = LineSeg->backsector;	/* Get the back sector */
	if (!BackSecPtr) {					/* Invalid? */
		BackSecPtr = &emptysector;
	}
	b_ceilingpic = BackSecPtr->CeilingPic;	/* Get backsector data into locals */
	b_lightlevel = BackSecPtr->lightlevel;
	b_floorheight = BackSecPtr->floorheight - viewz;	/* Adjust for camera z */
	b_ceilingheight = BackSecPtr->ceilingheight - viewz;

	actionbits = 0;		/* Reset vars for future storage */
	
/* Add floors and ceilings if the wall needs one */

	if (f_floorheight < 0 &&		/* Is the camera above the floor? */
		(FrontSecPtr->FloorPic != BackSecPtr->FloorPic ||	/* Floor texture changed? */
		f_floorheight != b_floorheight ||	/* Differant height? */
		f_lightlevel != b_lightlevel || 	/* Differant light? */
		b_ceilingheight == b_floorheight) ) {	/* No thickness line? */
		CurWallPtr->floorheight = CurWallPtr->floornewheight = f_floorheight>>FIXEDTOHEIGHT;
		actionbits = (AC_ADDFLOOR|AC_NEWFLOOR);	/* Create floor */
	}

	if ((f_ceilingpic != -1 || b_ceilingpic != -1) &&		/* Normal ceiling? */
		(f_ceilingheight > 0 || f_ceilingpic == -1) &&	/* Camera below ceiling? Sky ceiling? */
		(f_ceilingpic != b_ceilingpic || 	/* New ceiling image? */
		f_ceilingheight != b_ceilingheight ||	/* Differant ceiling height? */
		f_lightlevel != b_lightlevel || 		/* Differant ceiling light? */
		b_ceilingheight == b_floorheight ) ) {	/* Thin dividing line? */
		CurWallPtr->ceilingheight = CurWallPtr->ceilingnewheight = f_ceilingheight >>FIXEDTOHEIGHT;			
		if (f_ceilingpic == -1) {
			actionbits |= AC_ADDSKY|AC_NEWCEILING;	/* Add sky to the ceiling */
		} else {
 			actionbits |= AC_ADDCEILING|AC_NEWCEILING;	/* Add ceiling texture */
		}
	}

	CurWallPtr->t_topheight = f_ceilingheight>>FIXEDTOHEIGHT;	/* Y coord of the top texture */

/* Single sided line? They only have a center texture. */

	if (BackSecPtr == &emptysector) {		/* Bogus back sector? */
		int t_texturemid;
		CurWallPtr->t_texture = TextureTranslation[SidePtr->midtexture];
		if (LineFlags & ML_DONTPEGBOTTOM) {	/* bottom of texture at bottom */
			t_texturemid = f_floorheight + (CurWallPtr->t_texture->height << FRACBITS);
		} else {
			t_texturemid = f_ceilingheight;		/* top of texture at top */
		}
		t_texturemid += SidePtr->rowoffset;	/* Add texture anchor offset */
		CurWallPtr->t_texturemid = t_texturemid;	/* Save the top texture anchor var */
		CurWallPtr->t_bottomheight = f_floorheight>>FIXEDTOHEIGHT;
		actionbits |= (AC_TOPTEXTURE|AC_SOLIDSIL);	/* Draw the middle texture only */
	} else {

/* Two sided lines are more tricky since I may be able to see through it */

		if (b_floorheight > f_floorheight) {	/* Is the bottom wall texture visible? */
			int b_texturemid;
				
			/* Draw the bottom texture */
			
			CurWallPtr->b_texture = TextureTranslation[SidePtr->bottomtexture];
			if (LineFlags & ML_DONTPEGBOTTOM) {
				b_texturemid = f_ceilingheight; /* bottom of texture at bottom */
			} else {								  
				b_texturemid = b_floorheight;	/* Top of texture at top */
			}
			b_texturemid += SidePtr->rowoffset;	/* Add the adjustment */

			CurWallPtr->b_texturemid = b_texturemid;
			CurWallPtr->b_topheight = CurWallPtr->floornewheight = b_floorheight>>FIXEDTOHEIGHT;
			CurWallPtr->b_bottomheight = f_floorheight>>FIXEDTOHEIGHT;
			actionbits |= AC_NEWFLOOR|AC_BOTTOMTEXTURE;	/* Generate a floor and bottom wall texture */
		}


		if (b_ceilingheight < f_ceilingheight && (f_ceilingpic != -1 || b_ceilingpic != -1)) {	/* Ceiling wall without sky */
			int t_texturemid;
			
		/* Draw the top texture */
			
			CurWallPtr->t_texture = TextureTranslation[SidePtr->toptexture];
			if (LineFlags & ML_DONTPEGTOP) {
				t_texturemid = f_ceilingheight;	// top of texture at top
			} else {
				t_texturemid = b_ceilingheight + (CurWallPtr->t_texture->height<<FRACBITS);
			}
			t_texturemid += SidePtr->rowoffset;	/* Anchor the top texture */
			CurWallPtr->t_texturemid = t_texturemid;	/* Save the top texture anchor var */
			CurWallPtr->t_bottomheight = CurWallPtr->ceilingnewheight = b_ceilingheight>>FIXEDTOHEIGHT;
			actionbits |= AC_NEWCEILING|AC_TOPTEXTURE;		/* Generate the top texture */
		}
		
		/* Check if this wall is solid (This is for sprite clipping) */
		if (b_floorheight >= f_ceilingheight || b_ceilingheight <= f_floorheight) {
			actionbits |= AC_SOLIDSIL;		/* This is solid (For sprite masking) */
		} else {
			int width;
			
			width = (RightX-LeftX+1);		/* Get width of opening */
			if ( (b_floorheight > 0 && b_floorheight > f_floorheight) ||
				(f_floorheight < 0 && f_floorheight > b_floorheight) ) {
				actionbits |= AC_BOTTOMSIL;		/* There is a mask on the bottom */
				CurWallPtr->BottomSil = lastopening - LeftX;
				lastopening += width;
			}
			
			if (f_ceilingpic != -1 || b_ceilingpic != -1) {	/* Only if no sky */
				if ( (b_ceilingheight <= 0 && b_ceilingheight < f_ceilingheight) ||
					(f_ceilingheight > 0 && b_ceilingheight > f_ceilingheight) ) {	/* Top sil? */
					actionbits |= AC_TOPSIL;		/* There is a mask on the bottom */
					CurWallPtr->TopSil = lastopening - LeftX;
					lastopening += width;
				}
			}
		}
	}
	
	CurWallPtr->WallActions = actionbits;		/* Save the action bits */
	if (f_lightlevel < 240) {		/* Get the light level */
		f_lightlevel += extralight;	/* Add the light factor */
		if (f_lightlevel > 240) {
			f_lightlevel = 240;
		}
	}
	CurWallPtr->seglightlevel = f_lightlevel;	/* Save the light level */
	CurWallPtr->offset = SidePtr->textureoffset+LineSeg->offset;	/* Texture anchor X */
	LatePrep(CurWallPtr,LineSeg,LeftAngle);

}
