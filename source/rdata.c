#include "Doom.h"
#include <String.h>
#include <IntMath.h>

#define STRETCH(WIDTH,HEIGHT) (Fixed)((160.0/(float)WIDTH)*((float)HEIGHT/180.0)*2.2*65536)		

typedef struct {		/* Actual structure of TEXTURE1 */
	Word Count;			/* Count of entries */
	Word First;			/* Starting resource # */
	Word FlatCount;		/* Count of flats */
	Word FirstFlat;		/* Starting resource # for flats */
	texture_t Array[1];	/* Array of entries[Count] */
} Filemaptexture_t;

static Word ScreenWidths[6] = {280,256,224,192,160,128};
static Word ScreenHeights[6] = {160,144,128,112,96,80};
static Fixed Stretchs[6] = {
	STRETCH(280,160),
	STRETCH(256,144),
	STRETCH(224,128),
	STRETCH(192,112),
	STRETCH(160, 96),
	STRETCH(128, 80)
};
Word NumTextures;		/* Number of textures in the game */
Word FirstTexture;		/* First texture resource */
Word NumFlats;			/* Number of flats in the game */
Word FirstFlat;			/* Resource number to first flat texture */
texture_t *TextureInfo;	/* Array describing textures */
void ***FlatInfo;		/* Array describing flats */
texture_t **TextureTranslation;	/* Indexs to textures for global animation */
void ***FlatTranslation;		/* Indexs to textures for global animation */
texture_t *SkyTexture;		/* Pointer to the sky texture */

/**********************************

	Load in the "TextureInfo" array so that the game knows
	all about the wall and sky textures (Width,Height).
	Also initialize the texture translation table for wall animations.
	Called on powerup only.

**********************************/

void R_InitData(void)
{
	Word i;
{
	Filemaptexture_t *maptex;		/* Pointer to data loaded from disk */
	texture_t **TransPtr;
	texture_t *TexturePtr;

/* Load the map texture definitions from Textures1 */

	maptex = (Filemaptexture_t *)LoadAResource(rTEXTURE1);	/* Load it in */
	NumTextures = maptex->Count;		/* How many are there? */
    FirstTexture = maptex->First;		/* First resource number for loading */
    NumFlats = maptex->FlatCount;
    FirstFlat = maptex->FirstFlat;
	TextureInfo = &maptex->Array[0];		/* Index to the first entry */

/* Translation table for global animation */

	TransPtr = (texture_t **)AllocAPointer(NumTextures*sizeof(texture_t *));
	TextureTranslation = TransPtr;		/* Save the data pointer */
	TexturePtr = TextureInfo;
	i = NumTextures;
	do {
		TransPtr[0] = TexturePtr;	/* Init the table with sequential numbers */
		++TransPtr;
		++TexturePtr;
	} while (--i);		/* All done? */
}
    FlatInfo = (void ***)AllocAPointer(NumFlats*sizeof(Byte *));
    memset(FlatInfo,0,NumFlats*sizeof(void *));		/* Blank out the array for shape handles */

	FlatTranslation = (void ***)AllocAPointer(NumFlats*sizeof(Byte *));
	memset(FlatTranslation,0,sizeof(Byte *)*NumFlats);	/* Translate table for flat animation */
	
/**********************************

	Create a recipocal mul table so that I can
	divide 0-8191 from 1.0.
	This way I can fake a divide with a multiply

**********************************/

	IDivTable[0] = -1;
	i = 1;
	do {
		IDivTable[i] = IMFixDiv(512<<FRACBITS,i<<FRACBITS);		/* 512.0 / i */
	} while (++i<(sizeof(IDivTable)/sizeof(Word)));	
	InitMathTables();
}

/**********************************

	Create all the math tables for the current screen size

**********************************/

void InitMathTables(void)
{
	Fixed j;
	Word i;
	
	ScreenWidth = ScreenWidths[ScreenSize];
	ScreenHeight = ScreenHeights[ScreenSize];
	CenterX = (ScreenWidth/2);
	CenterY = (ScreenHeight/2);
	ScreenXOffset = ((320-ScreenWidth)/2);
	ScreenYOffset = ((160-ScreenHeight)/2);
	GunXScale = (ScreenWidth*0x100000)/320;		/* Get the 3DO scale factor for the gun shape */
	GunYScale = (ScreenHeight*0x10000)/160;		/* And the y scale */

	Stretch = Stretchs[ScreenSize];
	StretchWidth = Stretch*((int)ScreenWidth/2);

	/* Create the viewangletox table */
	
	j = IMFixDiv(CenterX<<FRACBITS,finetangent[FINEANGLES/4+FIELDOFVIEW/2]);
	i = 0;
	do {
		Fixed t;
		if (finetangent[i]>FRACUNIT*2) {
			t = -1;
		} else if (finetangent[i]< -FRACUNIT*2) {
			t = ScreenWidth+1;
		} else {
			t = IMFixMul(finetangent[i],j);
			t = ((CenterX<<FRACBITS)-t+FRACUNIT-1)>>FRACBITS;
			if (t<-1) {
				t = -1;
			} else if (t>(int)ScreenWidth+1) {
				t = ScreenWidth+1;
			}
		}	
		viewangletox[i/2] = t;
		i+=2;
	} while (i<FINEANGLES/2);

	/* Using the viewangletox, create xtoviewangle table */
		
	i = 0;
	do {
		Word x;
		x = 0;
		while (viewangletox[x]>(int)i) {
			++x;
		}
		xtoviewangle[i] = (x<<(ANGLETOFINESHIFT+1))-ANG90;
	} while (++i<ScreenWidth+1);
	
	/* Set the minimums and maximums for viewangletox */
	i = 0;
	do {
		if (viewangletox[i]==-1) {
			viewangletox[i] = 0;
		} else if (viewangletox[i] == ScreenWidth+1) {
			viewangletox[i] = ScreenWidth;
		}
	} while (++i<FINEANGLES/4);
	
	/* Make the yslope table for floor and ceiling textures */
	
	i = 0;
	do {
		j = (((int)i-(int)ScreenHeight/2)*FRACUNIT)+FRACUNIT/2;
		j = IMFixDiv(StretchWidth,abs(j));
		j >>= 6;
		if (j>0xFFFF) {
			j = 0xFFFF;
		}
		yslope[i] = j;
	} while (++i<ScreenHeight);
	
	/* Create the distance scale table for floor and ceiling textures */
	
	i = 0;
	do {
		j = abs(finecosine[xtoviewangle[i]>>ANGLETOFINESHIFT]);
		distscale[i] = IMFixDiv(FRACUNIT,j)>>1;
	} while (++i<ScreenWidth);

	/* Create the lighting tables */
	
	i = 0;
	do {
		Fixed Range;
		j = i/3;
		lightmins[i] = j;	/* Save the light minimum factors */
		Range = i-j;
		lightsubs[i] = ((Fixed)ScreenWidth*Range)/(800-(Fixed)ScreenWidth);
		lightcoefs[i] = (Range<<16)/(800-(Fixed)ScreenWidth);
		planelightcoef[i] = Range*(0x140000/(800-(Fixed)ScreenWidth));
	} while (++i<256);
}
