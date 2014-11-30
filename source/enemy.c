#include "Doom.h"
#include <IntMath.h>

/**********************************

	Return true if the target mobj_t is in melee
	range

**********************************/

static Word CheckMeleeRange(mobj_t *actor)
{
	mobj_t *pl;		/* Target mobj_t pointer */
	Fixed dist;		/* Distance calculated */

	pl = actor->target;		/* Get the mobj_t of the target */
	if (!pl ||			/* No target? */
		!(actor->flags&MF_SEETARGET)) {	/* Can't see target? */
		return FALSE;			/* Not in range... */
	}
	dist = GetApproxDistance(pl->x-actor->x,pl->y-actor->y);
	if (dist >= MELEERANGE) {		/* Out of range? */
		return FALSE;			/* Can't attack yet */
	}
	return TRUE;			/* Attack NOW! */
}

/**********************************

	Return true if the actor is in missile range
	to the target

**********************************/

static Word CheckMissileRange(mobj_t *actor)
{
	Fixed dist;
	mobj_t *pl;

	if ( !(actor->flags & MF_SEETARGET) ) {	/* Are you seen? */
		return FALSE;			/* Nope, don't fire! */
	}

	/* The target just hit the enemy, so fight back! */

	if (actor->flags & MF_JUSTHIT) {		/* Took damage? */
		actor->flags &= ~MF_JUSTHIT;		/* Clear the flag */
		return TRUE;				/* Fire back */
	}

	if (actor->reactiontime) {		/* Still waking up? */
		return FALSE;		/* Don't attack yet */
	}

	pl = actor->target;		/* Get local */
	dist = (GetApproxDistance(actor->x-pl->x,actor->y-pl->y)>>FRACBITS)-64;
	if (!actor->InfoPtr->meleestate) {	/* No hand-to-hand combat mode? */
		dist -= 128;		/* No melee attack, so fire more often */
	}

	if (actor->InfoPtr == &mobjinfo[MT_SKULL]) {	/* Is it a skull? */
		dist >>= 1;			/* Halve the distance for attack */
	}

	if (dist >= 201) {		/* 200 units? */
		dist = 200;			/* Set the maximum */
	}

	if ((int)GetRandom(255)<dist) {	/* Random chance for attack */
		return FALSE;		/* No attack */
	}
	return TRUE;			/* Attack! */
}

/**********************************

	Move in the current direction
	returns false if the move is blocked
	Called for ACTORS, not the player or missiles

**********************************/

static Fixed xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
static Fixed yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

static Boolean P_Move(mobj_t *actor)
{
	Word Direction;		/* Direction of travel */
	Word Speed;		/* Speed of travel */
	Fixed tryx,tryy;	/* New x and y */
	line_t *blkline;

	Direction = actor->movedir;
	if (Direction == DI_NODIR) {	/* No direction of travel? */
		return FALSE;			/* Can't move */
	}

	Speed = actor->InfoPtr->Speed;		/* Get the speed into cache */
	tryx = actor->x + ((Fixed)Speed*xspeed[Direction]);
	tryy = actor->y + ((Fixed)Speed*yspeed[Direction]);

	if (!P_TryMove(actor,tryx,tryy) ) {	/* Open any specials */
		if (actor->flags & MF_FLOAT && floatok) {	/* Must adjust height */
			if (actor->z < tmfloorz) {
				actor->z += FLOATSPEED;		/* Jump up */
			} else {
				actor->z -= FLOATSPEED;		/* Jump down */
			}
			actor->flags |= MF_INFLOAT;		/* I am floating (Or falling) */
			return TRUE;			/* I can move!! */
		}
		blkline = blockline;		/* What line blocked me? */
		if (!blkline || !blkline->special) {	/* Am I blocked? */
			return FALSE;		/* Can't move */
		}
		actor->movedir = DI_NODIR;	/* Force no direction */
			/* If the special isn't a door that can be opened, return false */
		if (P_UseSpecialLine(actor,blkline)) {	/* Try to open it... */
			return TRUE;			/* Allow motion */
		}
		return FALSE;		/* Can't move */
	}

	actor->flags &= ~MF_INFLOAT;		/* I am not floating anymore */
	if ( !(actor->flags & MF_FLOAT) ) {		/* Can I float at all? */
		actor->z = actor->floorz;		/* Attach to the floor */
	}
	return TRUE;		/* Yes, I can move there... */
}

/**********************************

	Attempts to move actoron in its current (ob->moveangle) direction.

	If blocked by either a wall or an actor returns false
	If move is either clear or blocked only by a door, returns TRUE and
	if a door is in the way, an OpenDoor call is made to start it opening.

**********************************/

static Boolean P_TryWalk(mobj_t *actor)
{
	if (!P_Move(actor)) {		/* Try to move in this direction */
		return FALSE;			/* Return no */
	}
	actor->movecount = GetRandom(15);	/* Get distance to travel */
	return TRUE;		/* I'm moving */
}

/**********************************

	Pick a direction to travel to chase
	the actor's target

**********************************/

static dirtype_t opposite[] =
	{DI_WEST,DI_SOUTHWEST,DI_SOUTH,DI_SOUTHEAST,DI_EAST,DI_NORTHEAST,
	DI_NORTH,DI_NORTHWEST,DI_NODIR};

static dirtype_t diags[] = {DI_NORTHWEST,DI_NORTHEAST,DI_SOUTHWEST,DI_SOUTHEAST};

static void P_NewChaseDir(mobj_t *actor)
{
	Fixed deltax,deltay;
	dirtype_t d1,d2;
	int tdir;
	dirtype_t olddir,turnaround;
	mobj_t *target;

	olddir = (dirtype_t)actor->movedir;		/* Get current direction */
	turnaround=opposite[olddir];			/* Get opposite direction */

	target = actor->target;
	deltax = target->x - actor->x;		/* Get the x offset */
	deltay = target->y - actor->y;		/* Get the y offset */

	if (deltax>=(10*FRACUNIT)) {		/* Towards the east? */
		d1=DI_EAST;
	} else if (deltax<(-10*FRACUNIT)) {	/* Towards the west? */
		d1=DI_WEST;
	} else {
		d1=DI_NODIR;			/* Go straight */
	}
	if (deltay<(-10*FRACUNIT)) {		/* Towards the south? */
		d2=DI_SOUTH;
	} else if (deltay>=(10*FRACUNIT)) {	/* Towards the north? */
		d2=DI_NORTH;
	} else {
		d2=DI_NODIR;			/* Go straight */
	}

/* Try direct route diagonally */

	if (d1 != DI_NODIR && d2 != DI_NODIR) {
		Word Index;
		Index = 0;
		if (deltay<0) {
			Index = 2;
		}
		if (deltax>=0) {
			Index|=1;
		}
		actor->movedir = diags[Index];
		if (actor->movedir != turnaround && P_TryWalk(actor)) {
			return;		/* It's ok! */
		}
	}

/* try other directions */

	if (GetRandom(255)>=201 || (abs(deltax)<abs(deltay))) {
		dirtype_t temp;
		temp=d1;	/* Reverse the direction priorities */
		d1=d2;
		d2=temp;
	}

	if (d1==turnaround) {	/* Invalidate reverse course */
		d1=DI_NODIR;
	}
	if (d2==turnaround) {
		d2=DI_NODIR;
	}

	/* Move in the highest priority direction */

	if (d1!=DI_NODIR) {
		actor->movedir = d1;
		if (P_TryWalk(actor)) {
			return;     /* Either moved forward or attacked */
		}
	}

	if (d2!=DI_NODIR) {
		actor->movedir =d2;	/* Low priority direction */
		if (P_TryWalk(actor)) {
			return;
		}
	}

/* There is no direct path to the player, so pick another direction */

	if (olddir!=DI_NODIR) {		/* Try the old direction */
		actor->movedir =olddir;
		if (P_TryWalk(actor)) {
			return;
		}
	}

/* Pick a direction by turning in a random order, don't */
/* choose the reverse course */

	if (GetRandom(1)) {		/* Which way to go duh George?  */
		tdir = DI_EAST;
		do {
			if (tdir!=(int)turnaround) {	/* Not backwards? */
				actor->movedir=(dirtype_t)tdir;
				if (P_TryWalk(actor)) {	/* Can I go this way? */
					return;
				}
			}
		} while (++tdir<(DI_SOUTHEAST+1));
	} else {
		tdir = DI_SOUTHEAST;
		do {
			if (tdir!=(int)turnaround) {		/* Not backwards? */
				actor->movedir=(dirtype_t)tdir;
				if (P_TryWalk(actor)) {	/* Can I go this way? */
					return;
				}
			}
		} while (--tdir>=(int)DI_EAST);	/* Next step */
	}

/* Hmmm, the only choice left is to turn around! */

	if (turnaround != DI_NODIR) {		/* Valid direction? */
		actor->movedir = turnaround;		/* Try backwards */
		if (P_TryWalk(actor)) {
			return;		/* Ok, let's go! */
		}
	}
	actor->movedir = DI_NODIR;	/* Can't move */
}

/**********************************

	If allaround is false, only look 180 degrees in front
	returns true if a player is targeted

**********************************/

static Boolean P_LookForPlayers(mobj_t *actor,Boolean allaround)
{
	angle_t an;
	Fixed dist;
	mobj_t *mo;

	/* pick another player as target if possible */

	if (!(actor->flags & MF_SEETARGET) ) {		/* Can I see the player? */
newtarget:
		actor->target = players.mo;		/* Force player #0 tracking */
		return FALSE;		/* No one is targeted */
	}
	mo = actor->target;		/* Get the target */
	if (!mo || !mo->MObjHealth) {	/* Is it alive? */
		goto newtarget;		/* Pick a target... */
	}

	if (actor->subsector->sector->soundtarget == actor->target) {
		allaround = TRUE;		/* Ambush guys will turn around on a shot */
	}

	if (!allaround) {		/* Only 180 degrees? */
		an = PointToAngle(actor->x,actor->y,mo->x,mo->y) - actor->angle;
		if (an > ANG90 && an < ANG270) {	/* In the span? */
			dist = GetApproxDistance(mo->x-actor->x,mo->y-actor->y);
			/* if real close, react anyway */
			if (dist > MELEERANGE) {
				return FALSE;		/* Behind back */
			}
		}
	}
	actor->threshold = (TICKSPERSEC*4/4);	/* Attack for 4 seconds */
	return TRUE;		/* I have a target! */
}

/**********************************

	Stay in state until a player is sighted

**********************************/

void A_Look(mobj_t *actor)
{
	Word sound;

/* If current target is visible, start attacking */

	if (!P_LookForPlayers(actor,FALSE)) {
		return;
	}

/* Go into chase state */

	sound = actor->InfoPtr->seesound;		/* Any sound to play? */
	if (sound) {
		switch (sound) {		/* Special case for the sound? */
		case sfx_posit1:
		case sfx_posit2:
		case sfx_posit3:
			sound = sfx_posit1+GetRandom(1);
			break;
		case sfx_bgsit1:
		case sfx_bgsit2:
			sound = sfx_bgsit1+GetRandom(1);
		}
		S_StartSound(&actor->x,sound);		/* Begin a sound */
	}
	SetMObjState(actor,actor->InfoPtr->seestate);	/* Set the chase state */
}

/**********************************

	Actor has a melee attack, so it tries to close in as fast as possible

**********************************/

void A_Chase(mobj_t *actor)
{
	long delta;
	Word Sound;
	mobjinfo_t *info;

	info = actor->InfoPtr;				/* Get the info record */
	if (actor->reactiontime) {		/* Adjust the reaction time */
		--actor->reactiontime;		/* Count down 1 */
	}
	
/* modify target threshold */

	if (actor->threshold) {
		--actor->threshold;
	}

/* turn towards movement direction if not there yet */

	if (actor->movedir < 8) {
		actor->angle &= (angle_t)(7UL<<29);
		delta = actor->angle - (actor->movedir << 29);
		if (delta > 0) {
			actor->angle -= ANG45;
		} else if (delta < 0) {
			actor->angle += ANG45;
		}
	}

	if (!actor->target || !(actor->target->flags&MF_SHOOTABLE)) {
		/* Look for a new target */
		if (P_LookForPlayers(actor,TRUE)) {
			return;		/* got a new target */
		}
		SetMObjState(actor,info->spawnstate);	/* Reset the state */
		return;
	}

/* Don't attack twice in a row */

	if (actor->flags & MF_JUSTATTACKED) {		/* Attacked? */
		actor->flags &= ~MF_JUSTATTACKED;		/* Clear the flag */
		P_NewChaseDir(actor);		/* Chase the player */
		return;
	}

/* Check for melee attack */

	if (info->meleestate && CheckMeleeRange(actor)) {
		Sound = info->attacksound;		/* Attack sound? */
		if (Sound) {		/* Attack sound? */
			S_StartSound(&actor->x,Sound);
		}
		SetMObjState(actor,info->meleestate);
		return;
	}

/* check for missile attack */

	if ((gameskill == sk_nightmare || !actor->movecount) &&
		info->missilestate && CheckMissileRange(actor)) {
		SetMObjState(actor,info->missilestate);	/* Shoot missile */
		if (gameskill != sk_nightmare) {		/* Ruthless!! */
			actor->flags |= MF_JUSTATTACKED;	/* Don't attack next round */
		}
		return;
	}

/* Chase towards player */

	if (actor->movecount) {		/* Count down if needed */
		--actor->movecount;
	}
	if (!actor->movecount || !P_Move(actor)) {	/* Move the critter */
		P_NewChaseDir(actor);
	}

/* make active sound */

	Sound = info->activesound;
	if (Sound && GetRandom(255)<3) {		/* Only 1 in 80 chance of gurgle */
		S_StartSound(&actor->x,Sound);
	}
}

/**********************************

	Turn to face your target

**********************************/

void A_FaceTarget(mobj_t *actor)
{
	mobj_t *target;

	target = actor->target;		/* Get the target pointer */
	if (target) {		/* Is there a target? */
		actor->flags &= ~MF_AMBUSH;	/* Not ambushing anymore */
		actor->angle = PointToAngle(actor->x,actor->y,target->x,target->y);
		if (actor->target->flags & MF_SHADOW) {		/* Hard to see? */
			actor->angle += (255-GetRandom(511))<<21;
		}
	}
}

/**********************************

	Shoot the player with a pistol (Used by Zombiemen)

**********************************/

void A_PosAttack(mobj_t *actor)
{
	angle_t angle;
	Word damage;

	if (actor->target) {		/* Is there a target? */
		A_FaceTarget(actor);	/* Face the target */
		S_StartSound(&actor->x,sfx_pistol);		/* Shoot the pistol */
		angle = actor->angle;		/* Get the angle */
		angle += (255-GetRandom(511))<<20;	/* Angle of error */
		damage = (GetRandom(7)+1)*3;	/* 1D8 * 3 */
		LineAttack(actor,angle,MISSILERANGE,MAXINT,damage);
	}
}

/**********************************

	Shoot the player with a shotgun (Used by Shotgun man)

**********************************/

void A_SPosAttack(mobj_t *actor)
{
	Word i,damage;
	angle_t angle,bangle;

	if (actor->target) {
		S_StartSound(&actor->x,sfx_shotgn);
		A_FaceTarget(actor);
		bangle = actor->angle;		/* Base angle */
		i = 0;
		do {
			angle = bangle + ((255-GetRandom(511))<<20);
			damage = (GetRandom(7)+1)*3;
			LineAttack(actor,angle,MISSILERANGE,MAXINT,damage);
		} while (++i<3);
	}
}

/**********************************

	Spider demon firing machine gun

**********************************/

void A_SpidRefire(mobj_t *actor)
{
/* Keep firing unless target got out of sight */

	A_FaceTarget(actor);
	if (GetRandom(255) >= 10) {
		if (!actor->target || !actor->target->MObjHealth ||
			!(actor->flags&MF_SEETARGET) ) {
			SetMObjState(actor,actor->InfoPtr->seestate);
		}
	}
}

/**********************************

	Imp attack

**********************************/

void A_TroopAttack(mobj_t *actor)
{
	Word damage;

	if (actor->target) {
		A_FaceTarget(actor);		/* Face your victim */
		if (CheckMeleeRange(actor)) {
			S_StartSound(&actor->x,sfx_claw);		/* Claw sound */
			damage = (GetRandom(7)+1)*3;		/* 1D8 * 3 */
			DamageMObj(actor->target,actor,actor,damage);
			return;		/* End attack */
		}
/* Launch a imp's missile */
		P_SpawnMissile(actor,actor->target,&mobjinfo[MT_TROOPSHOT]);
	}
}

/**********************************

	Demon or Spectre attack

**********************************/

void A_SargAttack(mobj_t *actor)
{
	Word damage;

	if (actor->target) {		/* Get the target pointer */
		A_FaceTarget(actor);		/* Face the player */
		damage = (GetRandom(7)+1)*4;		/* 1D8 * 4 */
		LineAttack(actor,actor->angle,MELEERANGE,0,damage);	/* Attack */
	}
}

/**********************************

	Evil eye attack

**********************************/

void A_HeadAttack (mobj_t *actor)
{
	Word damage;

	if (actor->target) {		/* Anyone targeted? */
		A_FaceTarget(actor);	/* Face the target */
		if (CheckMeleeRange(actor)) {		/* In bite range? */
			damage = (GetRandom(7)+1)*8;		/* 1D8 * 8 */
			DamageMObj(actor->target,actor,actor,damage);
			return;		/* Exit */
		}
/* Launch a missile */
		P_SpawnMissile(actor,actor->target,&mobjinfo[MT_HEADSHOT]);	/* Shoot eye missile */
	}
}

/**********************************

	Cyberdemon firing missile

**********************************/

void A_CyberAttack(mobj_t *actor)
{
	if (actor->target) {
		A_FaceTarget(actor);		/* Face the enemy */
		P_SpawnMissile(actor,actor->target,&mobjinfo[MT_ROCKET]);	/* Launch missile */
	}
}

/**********************************

	Baron of hell attack

**********************************/

void A_BruisAttack(mobj_t *actor)
{
	Word damage;

	if (actor->target) {		/* Target aquired? */
		if (CheckMeleeRange(actor)) {		/* Claw range? */
			S_StartSound(&actor->x,sfx_claw);	/* Ouch! */
			damage = (GetRandom(7)+1)*11;	/* 1D8 * 11 */
			DamageMObj(actor->target,actor,actor,damage);
			return;		/* Exit */
		}
/* Launch a missile */
		P_SpawnMissile(actor,actor->target,&mobjinfo[MT_BRUISERSHOT]);
	}
}

/**********************************

	Fly at the player like a missile

**********************************/

void A_SkullAttack(mobj_t *actor)
{
	mobj_t *dest;
	angle_t an;
	Word dist;

	dest = actor->target;
	if (dest) {				/* Target aquired? */
		actor->flags |= MF_SKULLFLY;		/* High speed mode */
		S_StartSound(&actor->x,actor->InfoPtr->attacksound);
		A_FaceTarget(actor);		/* Face the target */
		an = actor->angle >> ANGLETOFINESHIFT;		/* Speed for distance */
		actor->momx = IMFixMul(SKULLSPEED,finecosine[an]);
		actor->momy = IMFixMul(SKULLSPEED,finesine[an]);
		dist = GetApproxDistance(dest->x-actor->x,dest->y-actor->y);
		dist = dist / SKULLSPEED;		/* Speed to hit target */
		if (!dist) {		/* Prevent divide by 0 */
			dist = 1;
		}
		actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;
	}
}

/**********************************

	Play a normal death sound

**********************************/

void A_Scream(mobj_t *actor)
{
	Word Sound;

	Sound = actor->InfoPtr->deathsound;

	switch (Sound) {
	case 0:			/* No sound at all? */
		return;

	case sfx_podth1:
	case sfx_podth2:
	case sfx_podth3:
		Sound = sfx_podth1 + GetRandom(1);
		break;

	case sfx_bgdth1:
	case sfx_bgdth2:
		Sound = sfx_bgdth1 + GetRandom(1);
	}
	S_StartSound(&actor->x,Sound);
}

/**********************************

	Play the gory squish sound for a gruesome death

**********************************/

void A_XScream(mobj_t *actor)
{
	S_StartSound(&actor->x,sfx_slop);		/* Make goo */
}

/**********************************

	Play the pain sound if any

**********************************/

void A_Pain(mobj_t *actor)
{
	Word Sound;
	Sound = actor->InfoPtr->painsound;		/* Get the sound # */
	if (Sound) {			/* Valid? */
		S_StartSound(&actor->x,Sound);		/* Play it */
	}
}

/**********************************

	Actor fell to the ground dead, mark so you can walk over it

**********************************/

void A_Fall(mobj_t *actor)
{
	actor->flags &= ~MF_SOLID;		/* Not solid anymore */
}


/**********************************

	Process damage from an explosion

**********************************/

void A_Explode(mobj_t *thingy)
{
	RadiusAttack(thingy,thingy->target,128);	/* BOOM! */
}

/**********************************

	For level #8 of Original DOOM, I will trigger event #666
	to allow you to access the exit portal when you kill the
	Baron's of Hell.

**********************************/

void A_BossDeath(mobj_t *mo)
{
	mobj_t *mo2;
	line_t junk;

	if (gamemap != 8) {		/* Level #8? */
		return;			/* Kill all you want, we'll make more! */
	}

/* Scan the remaining thinkers to see if all bosses are dead */
/* This is a brute force method, but it works! */

	mo2 = mobjhead.next;		/* Get the first entry */
	do {	/* Wrapped around? */
		if (mo2 != mo && mo2->InfoPtr == mo->InfoPtr && mo2->MObjHealth) {
			return;		/* Other boss not dead */
		}
		mo2=mo2->next;		/* Keep scanning the list */
	} while (mo2!=&mobjhead);

/* Victory! */

	junk.tag = 666;		/* Floor's must be tagged with 666 */
	EV_DoFloor(&junk,lowerFloorToLowest);	/* Open the level */
}

/**********************************

	Play the Cyberdemon's metal hoof sound

**********************************/

void A_Hoof(mobj_t *mo)
{
	S_StartSound(&mo->x,sfx_hoof);		/* Play the sound */
	A_Chase(mo);					/* Chase the player */
}

/**********************************

	Make the spider demon's metal leg sound

**********************************/

void A_Metal(mobj_t *mo)
{
	S_StartSound(&mo->x,sfx_metal);		/* Make the sound */
	A_Chase(mo);					/* Handle the standard chase code */
}

/**********************************

	A move in Base.c caused a missile to hit another thing or wall

**********************************/

void L_MissileHit(mobj_t *mo,mobj_t *missilething)
{
	Word damage;

	if (missilething) {		/* Valid? */
		damage = (GetRandom(7)+1)*mo->InfoPtr->damage;		/* Calc the damage */
		DamageMObj(missilething,mo,mo->target,damage);	/* Inflict damage */
	}
	ExplodeMissile(mo);		/* Detonate the missile */
}

/**********************************

	A move in Base.c caused a flying skull to hit another thing or a wall

**********************************/

void L_SkullBash(mobj_t *mo,mobj_t *skullthing)
{
	Word damage;		/* Damage inflicted */

	if (skullthing) {		/* Valid? */
		damage = (GetRandom(7)+1)*mo->InfoPtr->damage;
		DamageMObj(skullthing,mo,mo,damage);
	}
	mo->flags &= ~MF_SKULLFLY;		/* The skull isn't flying fast anymore */
	mo->momx = mo->momy = mo->momz = 0;		/* Zap the momentum */
	SetMObjState(mo,mo->InfoPtr->spawnstate);	/* Normal mode */
}
