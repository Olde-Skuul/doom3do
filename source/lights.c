#include "Doom.h"

typedef struct {		/* Struct for light flashers */
	sector_t *sector;	/* Sector to affect */
	Word count;			/* Timer */
	Word maxlight;		/* Light level for bright flash */
	Word minlight;		/* Light level for ambient light */
	Word maxtime;		/* Time to hold bright light level */
	Word mintime;		/* Time to hold ambient light level */
} lightflash_t;

typedef struct {		/* Struct for strobe lights */
	sector_t *sector;	/* Sector to affect */
	Word count;			/* Timer */
	Word maxlight;		/* Light level for bright flash */
	Word minlight;		/* Light level for ambient light */
	Word brighttime;	/* Time to hold bright light */
	Word darktime;		/* Time to hold ambient light */
} strobe_t;

typedef struct {		/* Struct for glowing room */
	sector_t *sector;	/* Sector to affect */
	Word minlight;		/* Minimum light */
	Word maxlight;		/* Maximum light */
	int	direction;		/* Direction of light change */
} glow_t;

/**********************************

	Think logic for lighting effect of a dim occasional flash.
	(Used to pretend you have a defective flourecent bulb)

**********************************/

static void T_LightFlash(lightflash_t *flash)
{
	if (flash->count>ElapsedTime) {
		flash->count-=ElapsedTime;
		return;
	}

	if (flash->sector->lightlevel == flash->maxlight) {	/* Bright already? */
		flash->sector->lightlevel = flash->minlight;	/* Go dim */
		flash->count = GetRandom(flash->mintime)+1;	/* Time effect */
	} else {
		flash->sector->lightlevel = flash->maxlight;	/* Set bright */
		flash->count = GetRandom(flash->maxtime)+1;	/* Time */
	}
}

/**********************************

	Create a light flasher.

**********************************/

void P_SpawnLightFlash (sector_t *sector)
{
	lightflash_t *flash;

	sector->special = 0;	/* Nothing special about it during gameplay */

	flash = (lightflash_t *)AddThinker(T_LightFlash,sizeof(lightflash_t));
	flash->sector = sector;			/* Sector to affect */
	flash->maxlight = sector->lightlevel;	/* Use existing light as max */
	flash->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
	flash->maxtime = (TICKSPERSEC*4);		/* Time to hold light */
	flash->mintime = (TICKSPERSEC/2);
	flash->count = GetRandom(flash->maxtime)+1;	/* Init timer */
}

/**********************************

	Think logic for strobe lights

**********************************/

static void T_StrobeFlash(strobe_t *flash)
{
	if (flash->count>ElapsedTime) {			/* Time up? */
		flash->count-=ElapsedTime;		/* Count down */
		return;				/* Exit */
	}
	if (flash->sector->lightlevel == flash->minlight) {	/* Already dim? */
		flash->sector->lightlevel = flash->maxlight;	/* Make it bright */
		flash->count = flash->brighttime;		/* Reset timer */
	} else {
		flash->sector->lightlevel = flash->minlight;	/* Make it dim */
		flash->count = flash->darktime;			/* Reset timer */
	}
}

/**********************************

	Create a strobe light thinker

**********************************/

void P_SpawnStrobeFlash(sector_t *sector,Word fastOrSlow,Boolean inSync)
{
	strobe_t *flash;

	flash = (strobe_t *)AddThinker(T_StrobeFlash,sizeof(strobe_t));
	flash->sector = sector;		/* Set the thinker */
	flash->darktime = fastOrSlow;		/* Save the time delay */
	flash->brighttime = STROBEBRIGHT;	/* Time for bright light */
	flash->maxlight = sector->lightlevel;		/* Maximum light level */
	flash->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);

	if (flash->minlight == flash->maxlight) {	/* No differance in light? */
		flash->minlight = 0;		/* Pitch black then */
	}
	sector->special = 0;	/* Nothing special about it during gameplay */

	if (!inSync) {
		flash->count = GetRandom(7)+1;	/* Start at a random time */
	} else {
		flash->count = 1;		/* Start at a fixed time */
	}
}

/**********************************

	Start strobing lights (usually from a trigger)

**********************************/

void EV_StartLightStrobing(line_t *line)
{
	Word secnum;
	sector_t *sec;

	secnum = -1;
	while ((secnum = P_FindSectorFromLineTag(line,secnum)) != -1) {
		sec = &sectors[secnum];
		if (!sec->specialdata) {		/* Something here? */
			P_SpawnStrobeFlash(sec,SLOWDARK,FALSE);	/* Start a flash */
		}
	}
}

/**********************************

	Turn line's tag lights off

**********************************/

void EV_TurnTagLightsOff(line_t	*line)
{
	Word j;
	Word tag;
	sector_t *sector;

	sector = sectors;
	j = numsectors;
	tag = line->tag;
	do {
		if (sector->tag == tag) {
			line_t **templine;		/* Pointer to line_t array */
			Word min;				/* Lowest light level found */
			Word i;

			min = sector->lightlevel;	/* Get the current light level */
			i = sector->linecount;
			templine = sector->lines;
			do {
				sector_t *tsec;
				tsec = getNextSector(templine[0],sector);
				if (tsec) {
					if (tsec->lightlevel < min) {
						min = tsec->lightlevel;
					}
				}
				++templine;
			} while (--i);			/* All done? */
			sector->lightlevel = min;	/* Get the lowest light level */
		}
		++sector;
	} while (--j);
}

/**********************************

	Turn line's tag lights on

**********************************/

void EV_LightTurnOn(line_t *line,Word bright)
{
	Word i;
	sector_t *sector;
	Word tag;

    tag = line->tag;
	sector = sectors;
	i = numsectors;
	do {
		if (sector->tag == tag) {

			/* bright = 0 means to search for highest */
			/* light level surrounding sector */

			if (!bright) {
				line_t **templine;
				Word j;
				j = sector->linecount;
				templine = sector->lines;
				do {
					sector_t *temp;
					temp = getNextSector(templine[0],sector);
					if (temp) {
						if (temp->lightlevel > bright) {
							bright = temp->lightlevel;
						}
					}
					++templine;
				} while (--j);
			}
			sector->lightlevel = bright;
		}
		++sector;
	} while (--i);
}

/**********************************

	Spawn glowing light

**********************************/

static void T_Glow(glow_t *g)
{
	switch(g->direction) {
	case -1:		/* DOWN */
		g->sector->lightlevel -= GLOWSPEED;
		if ((g->sector->lightlevel & 0x8000) || g->sector->lightlevel <= g->minlight) {
			g->sector->lightlevel = g->minlight;
			g->direction = 1;
		}
		break;
	case 1:			/* UP */
		g->sector->lightlevel += GLOWSPEED;
		if (g->sector->lightlevel >= g->maxlight) {
			g->sector->lightlevel = g->maxlight;
			g->direction = -1;
		}
	}
}

/**********************************

	Spawn glowing light

**********************************/

void P_SpawnGlowingLight(sector_t *sector)
{
	glow_t *g;

	g = (glow_t *)AddThinker(T_Glow,sizeof(glow_t));
	g->sector = sector;
	g->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
	g->maxlight = sector->lightlevel;
	g->direction = -1;		/* Darken */
	sector->special = 0;	/* Nothing special here */
}
