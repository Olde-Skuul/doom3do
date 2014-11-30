#include "Doom.h"
#include <String.h>

/**********************************

	Local enums and data

**********************************/

#define	FLASHDELAY (TICKSPERSEC/4)	/* # of tics delay (1/60 sec) */
#define FLASHTIMES 6	/* # of times to flash new frag amount */

#define AMMOX 60		/* (42 Center) X,Y coord of the ammo */
#define AMMOY 174

#define HEALTHX 122		/* (93 Center) X,Y coord of health */
#define HEALTHY 174

#define KEYX 124		/* X coord of the key cards */
#define REDKEYY	163		/* Y coords of the key cards */
#define BLUKEYY	175
#define YELKEYY	187

#define FACEX 144		/* X,Y of the face */
#define FACEY 165

#define ARMORX 233		/* (204 Center) X,Y of the armor */
#define ARMORY 174

#define MAPX 290	/* X,Y Area level */
#define MAPY 174

#define	NUMMICROS 6			/* Amount of micro-sized #'s (weapon armed) */
#define	GODFACE 40			/* God mode face */
#define DEADFACE 41			/* Dead face */
#define FIRSTSPLAT 42		/* First gibbed face */
#define GIBTIME (TICKSPERSEC/6)			/* Time for gibbed animation */

enum {				/* Shape group rSBARSHP */
	sb_card_b,		/* Blue card */
	sb_card_y,		/* Yellow card */
	sb_card_r,		/* Red card */
	sb_skul_b,		/* Blue skull */
	sb_skul_y,		/* Yellow skull */
	sb_skul_r,		/* Red skull */
	sb_micro		/* The 6 small weapons */
};

enum {		/* Indexs for face array */
	sbf_lookfwd,	/* Look ahead */
	sbf_lookrgt,	/* Look right */
	sbf_looklft,	/* Look left */
	sbf_facelft,	/* Facing left */
	sbf_facergt,	/* Facing right */
	sbf_ouch,		/* In pain */
	sbf_gotgat,		/* Got some goodies */
	sbf_mowdown		/* Mowing down opponents */
};

typedef struct {
	Word delay;		/* Time delay */
	Word times;		/* Number of times to flash */
	Boolean	doDraw;	/* True if I draw now */
} sbflash_t;

stbar_t stbar;		/* Current state of the status bar */

static void **StatusBarShape;	/* Handle to current status bar shape */
static void **SBObj;	/* Cached handle to the status bar sub shapes */
static void **Faces;	/* Cached handle to the faces */
static Word micronums_x[NUMMICROS] = {237,249,261,237,249,261};	/* X's */
static Word micronums_y[NUMMICROS] = {175,175,175,185,185,185};	/* Y's */
static Word card_x[NUMCARDS] = {KEYX,KEYX,KEYX,KEYX+3,KEYX+3,KEYX+3};
static Word card_y[NUMCARDS] = {BLUKEYY,YELKEYY,REDKEYY,BLUKEYY,YELKEYY,REDKEYY};
static Word facetics;		/* Time before animating the face */
static Word newface;		/* Which normal face to show */
static Word spclfaceSprite[NUMSPCLFACES] =	/* Special face to shape # */
		{sbf_lookfwd,sbf_facelft,sbf_facergt,sbf_ouch,sbf_gotgat,sbf_mowdown};
static sbflash_t flashCards[NUMCARDS];	/* Info for flashing cards & Skulls */
static Boolean gibdraw;		/* Got gibbed? */
static Word gibframe;		/* Which gib frame */
static Word gibdelay;		/* Delay for gibbing */

/**********************************

	Process the timer for a shape flash

**********************************/

static void CycleFlash(sbflash_t *FlashPtr)
{
	if (FlashPtr->delay) {		/* Active? */
		if (FlashPtr->delay>ElapsedTime) {	/* Still time? */
			FlashPtr->delay-=ElapsedTime;	/* Remove the time */
		} else {
			if (!--FlashPtr->times) {		/* Can I still go? */
				FlashPtr->delay = 0;
				FlashPtr->doDraw = FALSE;		/* Force off */
			} else {
				FlashPtr->delay = FLASHDELAY;	/* Reset the time */
				FlashPtr->doDraw ^= TRUE;		/* Toggle the draw flag */
				if (FlashPtr->doDraw) {		/* If on, play sound */
					S_StartSound(0,sfx_itemup);
				}
			}
		}
	}
}

/**********************************

	Locate and load all needed graphics
	for the status bar

**********************************/

void ST_Start(void)
{
	SBObj = LoadAResourceHandle(rSBARSHP);	/* Status bar shapes */
	Faces = LoadAResourceHandle(rFACES);	/* Load all the face frames */
	StatusBarShape = LoadAResourceHandle(rSTBAR);	/* Load the status bar */
	memset(&stbar,0,sizeof(stbar));		/* Reset the status bar */
	facetics = 0;		/* Reset the face tic count */
	gibdraw = FALSE;	/* Don't draw gibbed head sequence */
	memset(&flashCards,0,sizeof(flashCards));
}

/**********************************

	Release resources allocated by the status bar

**********************************/

void ST_Stop(void)
{
	ReleaseAResource(rSBARSHP);		/* Status bar shapes */
	ReleaseAResource(rFACES);		/* All the face frames */
	ReleaseAResource(rSTBAR);		/* Lower bar */
}

/**********************************

	Called during the think logic phase to prepare
	to draw the status bar.

**********************************/

void ST_Ticker(void)
{
	Word ind;
	sbflash_t *FlashPtr;

	/* Animate face */

	facetics -= ElapsedTime;		/* Count down */
	if (facetics & 0x8000) {		/* Negative? */
		facetics = GetRandom(15)*4;		/* New random value */
		newface = GetRandom(2);		/* Which face 0-2 */
	}

	/* Draw special face? */

	if (stbar.specialFace) {
		newface = spclfaceSprite[stbar.specialFace];	/* Get the face shape */
		facetics = TICKSPERSEC;		/* 1 second */
		stbar.specialFace = f_none;	/* Ack the shape */
	}

	/* Did we get gibbed? */

	if (stbar.gotgibbed && !gibdraw) {	/* In progress? */
		gibdraw = TRUE;		/* In progress... */
		gibframe = 0;
		gibdelay = GIBTIME;
		stbar.gotgibbed = FALSE;
	}

	/* Tried to open a CARD or SKULL door? */

	ind = 0;
	FlashPtr = flashCards;
	do {	/* Check for initalization */
		if (stbar.tryopen[ind]) {		/* Did the user ask to flash the card? */
			stbar.tryopen[ind] = FALSE;	/* Ack the flag */
			FlashPtr->delay = FLASHDELAY;
			FlashPtr->times = FLASHTIMES+1;
			FlashPtr->doDraw = FALSE;
		}
		CycleFlash(FlashPtr);		/* Handle the ticker */
		++FlashPtr;
	} while (++ind<NUMCARDS);		/* All scanned? */
}

/**********************************

	Draw the status bar

**********************************/

#define VBLTIMER 0

#if VBLTIMER
#define TIMERSIZE 16
static Word Frames;
static Word CountVBL[TIMERSIZE];
#endif

void ST_Drawer (void)
{
	Word i;
	Word ind;
	player_t *p;
	sbflash_t *FlashPtr;

	DrawShape(0,160,*StatusBarShape);			/* Draw the status bar */

#if VBLTIMER
	++Frames;
	if (Frames>=TIMERSIZE) {
		Frames=0;
	}
	CountVBL[Frames]=ElapsedTime;
	ind = 0;
	i = 0;
	do {
		ind+=CountVBL[i];
	} while (++i<TIMERSIZE);
	ind = (60*TIMERSIZE)/ind;

	PrintNumber(100,80,ind,0);
#endif
	
	p = &players;

	/* Ammo */

	if (p->readyweapon != wp_nochange) {		/* No weapon? */
		ind = WeaponAmmos[p->readyweapon];
		if (ind != am_noammo) {		/* No ammo needed? */
			i = p->ammo[ind];
			PrintNumber(AMMOX,AMMOY,i,PNRight);		/* Draw ammo */
		}
	}
	PrintNumber(HEALTHX,HEALTHY,p->health,PNRight|PNPercent);	/* Draw the health */
	PrintNumber(ARMORX,ARMORY,p->armorpoints,PNRight|PNPercent);	/* Draw armor */
	PrintNumber(MAPX,MAPY,gamemap,PNCenter);		/* Draw AREA */

	/* Cards & skulls */

	ind = 0;
	FlashPtr = flashCards;
	do {
		if (p->cards[ind] || FlashPtr->doDraw) {	/* Flashing? */
			DrawMShape(
				card_x[ind],card_y[ind],GetShapeIndexPtr(*SBObj,sb_card_b+ind));
		}
		++FlashPtr;
	} while (++ind<NUMCARDS);

	/* Weapons */

	ind = 0;
	do {
		if (p->weaponowned[ind+1]) {
			DrawMShape(micronums_x[ind],micronums_y[ind],GetShapeIndexPtr(*SBObj,sb_micro+ind));
		}
	} while (++ind<NUMMICROS);
 	
	/* Face change */

	if ((p->AutomapFlags & AF_GODMODE) || p->powers[pw_invulnerability]) {		/* In god mode? */
		i = GODFACE;		/* God mode */
		gibframe = 0;
	} else if (gibdraw) {		/* Got gibbed */
		gibdelay-=ElapsedTime;		/* Time down gibbing */
		i = FIRSTSPLAT+gibframe;
		if (gibdelay&0x8000) {
			++gibframe;
			gibdelay = GIBTIME;
			if (gibframe >= 7) {		/* All frames shown? */
				gibdraw = FALSE;		/* Shut it off */
			}
		}
	} else if (!p->health) {
		i = FIRSTSPLAT+gibframe;	/* Dead man */
	} else {
		gibframe = 0;
		i = p->health;		/* Get the health */
		if (i>=80) {	/* Div by 20 without a real divide (Saves time) */
			i = 0;
		} else if (i>=60) {
			i = 8;
		} else if (i>=40) {
			i = 16;
		} else if (i>=20) {
			i = 24;
		} else {
			i = 32;	/* Bad shape... */
		}
		i = i+newface;	/* Get shape requested */
	}
	DrawMShape(FACEX,FACEY,GetShapeIndexPtr(*Faces,i));	/* Dead man */
}
