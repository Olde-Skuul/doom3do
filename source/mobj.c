#include "Doom.h"
#include <String.h>

typedef struct {		/* Respawn think logic */
	Fixed x,y;			/* X and Y to spawn at */
	Word angle;			/* Angle facing */
	Word time;			/* Time delay */
	mobjinfo_t *InfoPtr;	/* Item type */
} spawnthing_t;

/* Bit field for item spawning based on level */

static Word LevelBitMasks[5] = {MTF_EASY,MTF_EASY,MTF_NORMAL,MTF_HARD,MTF_HARD};	

/**********************************

	Respawn items in the respawn queue

**********************************/

#if 0
static void T_Respawn(spawnthing_t *mthing)
{
	Fixed x,y,z;
	subsector_t *ss;
	mobj_t *mo;

	if (mthing->time>ElapsedTime) {	/* Time up? */
		mthing->time-=ElapsedTime;		/* Count down */
		return;
	}

	x = mthing->x;		/* Get map coords of item */
	y = mthing->y;

/* Spawn a teleport fog at the new spot */

	ss = PointInSubsector(x,y);		/* Get the sector for floor info */
	mo = SpawnMObj(x,y,ss->sector->floorheight,&mobjinfo[MT_IFOG]);	/* Sparkle */
	S_StartSound(&mo->x,sfx_itmbk);		/* Poof! Sound */

/* Find which type to spawn */

	if (mthing->InfoPtr->flags & MF_SPAWNCEILING) {	/* Stuck on the ceiling? */
		z = ONCEILINGZ;
	} else {
		z = ONFLOORZ;
	}
	mo = SpawnMObj(x,y,z,mthing->InfoPtr);		/* Create the object */
	mo->angle = mthing->angle;	/* Get the angle */
	RemoveThinker(mthing);	/* Free memory */
}
#endif

/**********************************

	Remove a monster object from memory

**********************************/

void P_RemoveMobj(mobj_t *mobj)
{

/* Add to the respawnque for altdeath mode */

#if 0
	spawnthing_t *MapItem;
	if ((mobj->flags & MF_SPECIAL) &&		/* Special item */
		!(mobj->flags & MF_DROPPED) &&		/* Wasn't dropped there */
		(mobj->InfoPtr != &mobjinfo[MT_INVULNERABILITY]) &&			/* Not invulnerability */
		(mobj->InfoPtr != &mobjinfo[MT_INVISIBILITY]))	{			/* Not invisibility */
		MapItem = (spawnthing_t *)AddThinker(T_Respawn,sizeof(spawnthing_t));
		MapItem->x = mobj->x;		/* Restore the X,Y */
		MapItem->y = mobj->y;
		MapItem->InfoPtr = mobj->InfoPtr;	/* Object to spawn */
		MapItem->angle = mobj->angle;	/* Facing */
		MapItem->time = (30*TICKSPERSEC);	/* Time before respawn */
	}
#endif

/* Unlink from sector and block lists */

	UnsetThingPosition(mobj);

/* Unlink from mobj list */

	mobj->next->prev = mobj->prev;
	mobj->prev->next = mobj->next;
	DeallocAPointer(mobj);			/* Release the memory */
}

/**********************************

	Returns true if the mobj is still present

**********************************/

Word SetMObjState(mobj_t *mobj,state_t *StatePtr)
{
	if (!StatePtr) {		/* Shut down state? */
		P_RemoveMobj(mobj);		/* Remove the object */
		return FALSE;			/* Object is shut down */
	}
	mobj->state = StatePtr;			/* Save the state index */
	mobj->tics = StatePtr->Time;		/* Reset the tic count */

	if (StatePtr->action) {	/* Call action functions when the state is set */
		StatePtr->action(mobj);		/* Call the procedure */
	}
	return TRUE;
}

/**********************************

	To make some death spawning a little more random, subtract a little 
	random time value to mix up the deaths.

**********************************/

void Sub1RandomTick(mobj_t *mobj)
{
	Word Delay;
	Delay = GetRandom(3);	/* Getthe random adjustment */
	if (mobj->tics>=Delay) {	/* Too large? */
		mobj->tics-=Delay;	/* Set the new time */
	} else {
		mobj->tics=0;		/* Force a minimum */
	}
}

/**********************************

	Detonate a missile

**********************************/

void ExplodeMissile(mobj_t *mo)
{
	Word Sound;
	mo->momx = mo->momy = mo->momz = 0;		/* Stop forward momentum */
	SetMObjState(mo,mo->InfoPtr->deathstate);	/* Enter explosion state */
	Sub1RandomTick(mo);
	mo->flags &= ~MF_MISSILE;		/* It's not a missile anymore */
	Sound = mo->InfoPtr->deathsound;	/* Get the sound */
	if (Sound) {
		S_StartSound(&mo->x,Sound);		/* Play the sound if any */
	}
}

/**********************************

	Spawn a misc object

**********************************/

mobj_t *SpawnMObj(Fixed x,Fixed y,Fixed z,mobjinfo_t *InfoPtr)
{
	mobj_t *mobj;
	state_t *st;
	sector_t *sector;

	mobj = (mobj_t *)AllocAPointer(sizeof(mobj_t));
	memset(mobj,0,sizeof(mobj_t));		/* Init the memory */

	mobj->InfoPtr = InfoPtr;	/* Save the type pointer */
	mobj->x = x;			/* Set the x (In Fixed pixels) */
	mobj->y = y;
	mobj->radius = InfoPtr->Radius;	/* Radius of object */
	mobj->height = InfoPtr->Height;	/* Height of object */
	mobj->flags = InfoPtr->flags;		/* Misc flags */
	mobj->MObjHealth = InfoPtr->spawnhealth;	/* Health at start */
	mobj->reactiontime = InfoPtr->reactiontime;	/* Initial reaction time */

/* do not set the state with SetMObjState, because action routines can't */
/* be called yet */

	st = InfoPtr->spawnstate;		/* Get the initial state */
	if (!st) {					/* If null, then it's dormant */
		st = &states[S_NULL];	/* Since I am creating this, I MUST have a valid state */
	}
	mobj->state = st;				/* Save the state pointer */
	mobj->tics = st->Time;			/* Init the tics */

/* Set subsector and/or block links */

	SetThingPosition(mobj);		/* Attach to floor */

	sector = mobj->subsector->sector;
	mobj->floorz = sector->floorheight;	/* Get the objects top clearance */
	mobj->ceilingz = sector->ceilingheight;		/* And bottom clearance */
	if (z == ONFLOORZ) {
		mobj->z = mobj->floorz;		/* Set the floor z */
	} else if (z == ONCEILINGZ) {
		mobj->z = mobj->ceilingz - mobj->InfoPtr->Height;	 /* Adjust from ceiling */
	} else {
		mobj->z = z;		/* Use the raw z */
	}

/* Link into the END of the mobj list */

	mobjhead.prev->next = mobj;		/* Set the new forward link */
	mobj->next = &mobjhead;			/* Set my next link */
	mobj->prev = mobjhead.prev;		/* Set my previous link */
	mobjhead.prev = mobj;			/* Link myself in */
	return mobj;		/* Return the new object pointer */
}

/**********************************

	Called when a player is spawned on the level
	Most of the player structure stays unchanged between levels

**********************************/

void P_SpawnPlayer(mapthing_t *mthing)
{
	player_t *p;
	Fixed x,y;
	mobj_t *mobj;
	Word i;

	i = mthing->type-1;		/* Which player entry? */
	if (i) {				/* In the game? */
		return;				/* not playing */
	}
	p = &players;		/* Get the player # */

	if (p->playerstate == PST_REBORN) {		/* Did you die? */
		G_PlayerReborn();			/* Reset the player's variables */
	}

	x = mthing->x;		/* Get the Fixed coords */
	y = mthing->y;
	mobj = SpawnMObj(x,y,(Fixed)ONFLOORZ,&mobjinfo[MT_PLAYER]);

	mobj->angle = mthing->angle;	/* Get the facing angle */

	mobj->player = p;			/* Save the player's pointer */
	mobj->MObjHealth = p->health;	/* Init the health */
	p->mo = mobj;				/* Save the mobj into the player record */
	p->playerstate = PST_LIVE;	/* I LIVE!!! */
	p->refire = FALSE;			/* Not autofiring (Yet) */
	p->message = 0;				/* No message passed */
	p->damagecount = 0;			/* No damage taken */
	p->bonuscount = 0;			/* No bonus awarded */
	p->extralight = 0;			/* No light goggles */
	p->fixedcolormap = 0;		/* Normal color map */
	p->viewheight = VIEWHEIGHT;	/* Normal height */
	SetupPSprites(p);		/* Setup gun psprite */
}

/**********************************

	Create an item for the map

**********************************/

void SpawnMapThing(mapthing_t *mthing)
{
	Word i,Type;
	mobj_t *mobj;
	mobjinfo_t *InfoPtr;

/* Count deathmatch start positions */

	Type = mthing->type;
	if (Type == 11) {		/* Starting positions */
		if (deathmatch_p < &deathmatchstarts[10]) {	/* Full? */
			deathmatch_p[0] = mthing[0];
		}
		++deathmatch_p;		/* I have a starting position */
		return;				/* Exit now */
	}

/* Check for players specially */

	if (Type < 5) {		/* Object 0-4 */
		/* save spots for respawning in network games */
		if (Type < (1+1)) {
			playerstarts = mthing[0];	/* Save the start spot */
			P_SpawnPlayer(mthing);		/* Create the player */	
		}
		return;			/* Exit */
	}

/* Check for apropriate skill level */

	if (mthing->ThingFlags & MTF_DEATHMATCH) {	/* Must be in deathmatch? */
		return;			/* Don't spawn, requires deathmatch */
	}

	if (!(mthing->ThingFlags & LevelBitMasks[gameskill])) {	/* Can I spawn this? */
		return;			/* Exit now */
	}

/* Find which type to spawn */

	i = 0;
	InfoPtr = mobjinfo;		/* Get pointer to table */
    do {
		if (InfoPtr->doomednum == Type) {		/* Match? */
			Fixed z;

			/* Spawn it */

			z = ONFLOORZ;		/* Most of the time... */
			if (InfoPtr->flags & MF_SPAWNCEILING) {	/* Attach to ceiling or floor */
				z = ONCEILINGZ;
			} 
			mobj = SpawnMObj(mthing->x,mthing->y,z,InfoPtr);	/* Create the object */
			if (mobj->tics) {		/* Randomize the initial tic count */
				if (mobj->tics!=-1) {
					mobj->tics = GetRandom(mobj->tics)+1;
				}
			}
			if (mobj->flags & MF_COUNTKILL) {		/* Must be killed? */
				++TotalKillsInLevel;
			}
			if (mobj->flags & MF_COUNTITEM) {		/* Must be collected? */
				++ItemsFoundInLevel;
			}

			mobj->angle = mthing->angle;	/* Round the angle */
			if (mthing->ThingFlags & MTF_AMBUSH) {		/* Will it wait in ambush? */
				mobj->flags |= MF_AMBUSH;
			}
			return;
		}
		++InfoPtr;				/* Next entry */
	} while (++i<NUMMOBJTYPES);
}

/**********************************

	Spawn a puff of smoke on the wall

**********************************/

void P_SpawnPuff(Fixed x,Fixed y,Fixed z)
{
	mobj_t *th;

	z += (255-GetRandom(511))<<10;		/* Randomize the z */
	th = SpawnMObj(x,y,z,&mobjinfo[MT_PUFF]);		/* Create a puff */
	th->momz = FRACUNIT;	/* Allow it to move up per frame */
	Sub1RandomTick(th);

/* Don't make punches spark on the wall */

	if (attackrange == MELEERANGE) {
		SetMObjState(th,&states[S_PUFF3]);		/* Reset to the third state */
	}
}

/**********************************

	Blow up a body REAL GOOD!

**********************************/

void P_SpawnBlood(Fixed x,Fixed y,Fixed z,Word damage)
{
	mobj_t *th;

	z += (255-GetRandom(511))<<10;	/* Move a little for the Z */
	th = SpawnMObj(x,y,z,&mobjinfo[MT_BLOOD]);	/* Create the blood (Hamburger) */
	th->momz = FRACUNIT*2;			/* Allow some ascending motion */
	Sub1RandomTick(th);
	if (damage < 13 && damage >= 9) {
		SetMObjState(th,&states[S_BLOOD2]);	/* Smaller mess */
	} else if (damage < 9) {
		SetMObjState(th,&states[S_BLOOD3]);	/* Mild mess */
	}
}

/**********************************

	Moves the missile forward a bit and possibly explodes it right there

**********************************/

static void P_CheckMissileSpawn(mobj_t *th)
{
	th->x += (th->momx>>1);	/* Adjust x and y based on momentum */
	th->y += (th->momy>>1);	/* move a little forward so an angle can */
							/* be computed if it immediately explodes */
	th->z += (th->momz>>1);
	if (!P_TryMove(th,th->x,th->y)) {	/* Impact? */
		ExplodeMissile(th);			/* Boom!!! */
	}
}

/**********************************

	Launch a missile from a monster to
	a player or other monster.

**********************************/

void P_SpawnMissile(mobj_t *source,mobj_t *dest,mobjinfo_t *InfoPtr)
{
	mobj_t *th;
	angle_t an;
	Word dist;
	Word speed;

	/* Create the missile object (32 pixels off the ground) */

	th = SpawnMObj(source->x,source->y,source->z+(32*FRACUNIT),InfoPtr);
	dist = InfoPtr->seesound;
	if (dist) {
		S_StartSound(&source->x,dist);		/* Play the launch sound */
	}
	th->target = source;		/* Who launched it? */
	an = PointToAngle(source->x,source->y,dest->x,dest->y);	/* Angle of travel */
	if (dest->flags & MF_SHADOW) {		/* Hard to see, miss on purpose! */
		an += (255-GetRandom(511))<<20;
	}
	th->angle = an;
	an >>= ANGLETOFINESHIFT;		/* Convert to internal table record */
	speed = InfoPtr->Speed;		/* Get the speed */
	th->momx = (Fixed)speed * finecosine[an];
	th->momy = (Fixed)speed * finesine[an];

	dist = GetApproxDistance(dest->x-source->x,dest->y-source->y);
	dist = dist / (Fixed)(speed<<FRACBITS);		/* Convert to frac */
	if (!dist) {
		dist = 1;			/* Prevent divide by zero */
	}
	th->momz = (dest->z - source->z) / (Fixed)dist;		/* Set the z momentum */
	P_CheckMissileSpawn(th);			/* Move the missile 1 tic */
}

/**********************************

	Launch a player's missile (BFG, Rocket)
	Tries to aim at a nearby monster.

**********************************/

void SpawnPlayerMissile(mobj_t *source,mobjinfo_t *InfoPtr)
{
	mobj_t *th;
	angle_t an;
	Fixed x,y,z,slope;
	Word speed;			/* Speed of missile */

/* See which target is to be aimed at */

	an = source->angle;		/* Get the player's angle */

	slope = AimLineAttack(source,an,16*64*FRACUNIT);
	if (!linetarget) {		/* No target? */
		an += 1<<26;		/* Aim a little to the right */
		slope = AimLineAttack(source,an,16*64*FRACUNIT);
		if (!linetarget) {	/* Still no target? */
			an -= 2<<26;		/* Try a little to the left */
			slope = AimLineAttack(source,an,16*64*FRACUNIT);
			if (!linetarget) {	/* I give up, just fire directly ahead */
				an = source->angle;		/* Reset the angle */
				slope = 0;		/* No z slope */
			}
		}
	}

	x = source->x;				/* From the player */
	y = source->y;
	z = source->z + (32*FRACUNIT);		/* Off the ground */

	th = SpawnMObj(x,y,z,InfoPtr);		/* Spawn the missile */
    speed = InfoPtr->seesound;		/* Get the sound */
	if (speed) {
		S_StartSound(&source->x,speed);	/* Play the sound */
	}
	th->target = source;		/* Set myself as the target */
	th->angle = an;				/* Set the angle */

	speed = InfoPtr->Speed;		/* Get the true speed */
	an>>=ANGLETOFINESHIFT;		/* Convert to index */

	th->momx = (Fixed)speed * finecosine[an];
	th->momy = (Fixed)speed * finesine[an];
	th->momz = (Fixed)speed * slope;

	P_CheckMissileSpawn(th);		/* Move the missile a little */
}
