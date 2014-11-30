#include "Doom.h"
#include <IntMath.h>

#define MAXBOB (16<<FRACBITS)	/* 16 pixels of bobbing up and down */
#define SLOWTURNTICS 10			/* Time before fast turning */

static Boolean onground;		/* True if the player is on the ground */

static LongWord forwardmove[2] = {0x38000>>2,0x60000>>2};
static LongWord sidemove[2] = {0x38000>>2,0x58000>>2};

static Fixed angleturn[] =
	{600<<FRACBITS, 600<<FRACBITS,1000<<FRACBITS,1000<<FRACBITS,1200<<FRACBITS,
	1400<<FRACBITS,1600<<FRACBITS,1800<<FRACBITS,1800<<FRACBITS,2000<<FRACBITS};
static Fixed fastangleturn[] =	/* Will be mul'd by ElapsedTime */
	{400<<FRACBITS, 400<<FRACBITS, 450<<FRACBITS, 500<<FRACBITS, 500<<FRACBITS,
	 600<<FRACBITS, 600<<FRACBITS, 650<<FRACBITS, 650<<FRACBITS, 700<<FRACBITS};

/**********************************

	Move the player along the momentum

**********************************/

static void P_PlayerMove(mobj_t *mo)
{
	Fixed momx, momy;

	momx = ElapsedTime*(mo->momx>>2);		/* Get the momemtum */
	momy = ElapsedTime*(mo->momy>>2);

	P_SlideMove(mo);			/* Slide the player ahead */

	if (slidex != mo->x || slidey != mo->y) {	/* No motion at all? */
		if (P_TryMove(mo,slidex,slidey)) {		/* Can I move? */
			goto dospecial;		/* Movement OK, just special and exit */
		}
	}

	if (momx > MAXMOVE)	{		/* Clip the momentum to maximum */
		momx = MAXMOVE;
	}
	if (momx < -MAXMOVE) {
		momx = -MAXMOVE;
	}
	if (momy > MAXMOVE) {
		momy = MAXMOVE;
	}
	if (momy < -MAXMOVE) {
		momy = -MAXMOVE;
	}

/* Since I can't either slide or directly go to my destination point, */
/* stairstep to the closest point to the wall */

	if (P_TryMove(mo,mo->x,mo->y+momy)) {		/* Try Y motion only */
		mo->momx = 0;		/* Kill the X motion */
		mo->momy = momy;	/* Keep the Y motion */
	} else if (P_TryMove(mo,mo->x+momx,mo->y)) {	/* Try X motion only */
		mo->momx = momx;	/* Keep the X motion */
		mo->momy = 0;		/* Kill the Y motion */
	} else {
		mo->momx = mo->momy = 0;	/* No more sliding */
	}

dospecial:
	if (specialline) {		/* Did a line get crossed? */
		P_CrossSpecialLine(specialline,mo);	/* Call the special event */
	}
}

/**********************************

	Move the player in the X and Y directions

**********************************/

#define	STOPSPEED	0x1000		/* Speed to stop at */
#define	FRICTION	0xD240		/* 841/1024 0.821289 Friction rate */

static void P_PlayerXYMovement(mobj_t *mo)
{
	P_PlayerMove(mo);		/* Move the player */

/* Slow down */

	if (mo->z <= mo->floorz) {		/* Touching the floor? */
		if (mo->flags & MF_CORPSE) {	/* Corpse's will fall off */
			if (mo->floorz != mo->subsector->sector->floorheight) {
				return;			/* Don't stop halfway off a step */
			}
		}

		if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED &&	/* Too slow? */
			mo->momy > -STOPSPEED && mo->momy < STOPSPEED) {
			mo->momx = 0;		/* Kill momentum */
			mo->momy = 0;
		} else {
			mo->momx = IMFixMul(mo->momx,FRICTION);		/* Slow down */
			mo->momy = IMFixMul(mo->momy,FRICTION);
		}
	}
}


/**********************************

	Handle motion in the Z axis (Falling or gravity)

**********************************/

static void P_PlayerZMovement(mobj_t *mo)
{

/* Check for smooth step up */

	if (mo->z < mo->floorz) {		/* My z lower than the floor? */
		mo->player->viewheight -= mo->floorz-mo->z;	/* Adjust the view for floor */
		mo->player->deltaviewheight = (VIEWHEIGHT - mo->player->viewheight)>>2;
	}
	mo->z += mo->momz;		/* Apply gravity to the player's z */

/* Clip movement */

	if (mo->z <= mo->floorz) {	/* Hit the floor */
		if (mo->momz < 0) {		/* Going down? */
			if (mo->momz < -GRAVITY*4) {	/* squat down (Hit hard!) */
				mo->player->deltaviewheight = mo->momz>>3;
				S_StartSound(&mo->x,sfx_oof);	/* Ouch! */
			}
			mo->momz = 0;		/* Stop the fall */
		}
		mo->z = mo->floorz;		/* Set the proper z */
	} else {
		if (!mo->momz) {		/* No fall? */
			mo->momz = -GRAVITY*2;	/* Apply a little gravity */
		} else {
			mo->momz -= GRAVITY;	/* Add some more gravity */
		}
	}
	if ((mo->z + mo->height) > mo->ceilingz) {	/* Hit the ceiling? */
		if (mo->momz > 0) {		/* Going up? */
			mo->momz = 0;		/* Stop the motion */
		}
		mo->z = mo->ceilingz - mo->height;	/* Peg at the ceiling */
	}
}

/**********************************

	Cycle through the game state logic so that the
	animation frames drawn are the rate of motion.

**********************************/

static void P_PlayerMobjThink (mobj_t *mobj)
{
	state_t	*st;		/* Pointer to current game state structure */

/* Momentum movement */

	if (mobj->momx || mobj->momy) {		/* Any x,y motion? */
		P_PlayerXYMovement(mobj);		/* Move in a 2D sense */
	}

	if ( (mobj->z != mobj->floorz) || mobj->momz) {	/* Any z momentum? */
		P_PlayerZMovement(mobj);
	}

/* cycle through states, calling action functions at transitions */

	if (mobj->tics != -1) {
		if (mobj->tics>ElapsedTime) {		/* Time to cycle? */
			mobj->tics-=ElapsedTime;
			return;				/* Not time to cycle yet */
		}
		mobj->tics = 0;		/* Reset the tic count */
		st = mobj->state->nextstate;	/* Get the next state index */
		mobj->state = st;
		mobj->tics = st->Time;		/* Reset the timer ticks */
	}
}

/**********************************

	Convert joypad inputs into player motion

**********************************/

static void P_BuildMove(player_t *player)
{
	Word buttons;		/* Current joypad */
	Word oldbuttons;	/* Previous joypad */
	Word TurnIndex;		/* Index to the turn table */
	Word SpeedIndex;	/* Must be 0 or 1 */
	Fixed Motion;		/* Motion result */
	mobj_t *mo;

	buttons = JoyPadButtons;
	oldbuttons = PrevJoyPadButtons;
	SpeedIndex = (buttons&PadSpeed) ? 1 : 0;
	
/* Use two stage accelerative turning on the joypad */

	TurnIndex = player->turnheld + ElapsedTime;
	
	if ( !(buttons & PadLeft) || !(oldbuttons & PadLeft) ) {		/* Not held? */
		if ( !(buttons & PadRight) || !(oldbuttons & PadRight) ) {
			TurnIndex = 0;		/* Reset timer */
		}
	}
	if (TurnIndex >= SLOWTURNTICS) {	/* Detect overflow */
		TurnIndex = SLOWTURNTICS-1;
	}
	player->turnheld = TurnIndex;		/* Save it */

	Motion = 0;				/* Assume no side motion */
	if (!(buttons & PadUse)) {		/* Use allows weapon change */
		if (buttons & (PadRightShift|PadLeftShift)) {	/* Side motion? */
			Motion = sidemove[SpeedIndex]*ElapsedTime;	/* Sidestep to the right */
			if (buttons & PadLeftShift) {
				Motion = -Motion;	/* Sidestep to the left */
			}
		}
	}
	player->sidemove = Motion;		/* Save the result */

	Motion = 0;			/* No angle turning */
	if (SpeedIndex && !(buttons&(PadUp|PadDown)) ) {
		if (buttons & (PadRight|PadLeft)) {
			Motion = fastangleturn[TurnIndex]*ElapsedTime;
			if (buttons & PadRight) {
				Motion = -Motion;
			}
		}
	} else {
		if (buttons & (PadRight|PadLeft)) {
			Motion = angleturn[TurnIndex];		/* Don't time adjust, for fine tuning */
			if (ElapsedTime<4) {
				Motion>>=1;
				if (ElapsedTime<2) {
					Motion>>=1;
				}
			}
			if (buttons & PadRight) {
				Motion = -Motion;
			}
		}
	}
	player->angleturn = Motion;		/* Save the new angle */

	Motion = 0;
	if (buttons & (PadUp|PadDown)) {
		Motion = forwardmove[SpeedIndex]*ElapsedTime;
		if (buttons & PadDown) {
			Motion = -Motion;
		}
	}
	player->forwardmove = Motion;	/* Save the motion */

/* If slowed down to a stop, change to a standing frame */

	mo = player->mo;

	if (!mo->momx && !mo->momy && !player->forwardmove && !player->sidemove) {
	/* if in a walking frame, stop moving */
		state_t *StatePtr;
		StatePtr = mo->state;
		if (StatePtr == &states[S_PLAY_RUN1] ||
			StatePtr == &states[S_PLAY_RUN2] ||
			StatePtr == &states[S_PLAY_RUN3] ||
			StatePtr == &states[S_PLAY_RUN4]) {
			SetMObjState(mo,&states[S_PLAY]);		/* Standing frame */
		}
	}
}

/**********************************

	Moves the given origin along a given angle

**********************************/

static void PlayerThrust(mobj_t *MObjPtr,angle_t angle,Fixed move)
{
	if (move) {
		angle >>= ANGLETOFINESHIFT;		/* Convert to index to table */
		move >>= (FRACBITS-8);				/* Convert to integer (With 8 bit frac) */
		MObjPtr->momx += (move*finecosine[angle])>>8;	/* Add momentum */
		MObjPtr->momy += (move*finesine[angle])>>8;
	}
}

/**********************************

	Calculate the walking / running height adjustment
	(This will bob the camera up and down)
	Note : I MUST calculate the bob value or the gun will
	not be locked onto the player's view properly!!

**********************************/

static void PlayerCalcHeight(player_t *player)
{
	Fixed bob;		/* Bobbing offset */
	Fixed top;

/* Regular movement bobbing (needs to be calculated for gun swing even */
/* if not on ground) */

	bob = player->mo->momx;		/* Get the momentum constant */
	top = player->mo->momy;
	bob = (IMFixMul(bob,bob)+IMFixMul(top,top))>>4;
	if (bob>MAXBOB) {
		bob = MAXBOB;		/* Use the maximum */
	}
	player->bob = bob;		/* Save the new vertical adjustment */

	if (!onground) {		/* Don't bob the view if in the air! */
		bob = 0;			/* Zap the bob factor */
	} else {

/* Calculate the viewing angle offset since the camera is now */
/* bobbing up and down */
/* The multiply constant is based on a movement of a complete cycle bob */
/* every 0.666 seconds or 2/3 * TICKSPERSEC) or 40 ticks (60 ticks a sec) */
/* This is translated to 8192/40 or 204.8 angles per tick. */
/* Neat eh? */

		bob = (bob>>(FRACBITS+1)) * finesine[		/* What a mouthful!! */
			((FINEANGLES/((2*TICKSPERSEC)/3))*TotalGameTicks)&FINEMASK];

		if (player->playerstate == PST_LIVE) {		/* If the player is alive... */
			top = player->deltaviewheight;		/* Get gravity */
			player->viewheight += top;	/* Adjust to delta */
			if (player->viewheight > VIEWHEIGHT) {
				player->viewheight = VIEWHEIGHT;	/* Too high! */
				top = 0;		/* Kill delta */
			}
			if (player->viewheight < VIEWHEIGHT/2) {	/* Too low? */
				player->viewheight = VIEWHEIGHT/2;	/* Set the lowest */
				if (top <= 0) {	/* Minimum squat is 1 */
					top = 1;
				}
			}
			if (top) {	/* Going down? */
				top += FRACUNIT/2;	/* Increase the drop speed */
				if (!top) {		/* Zero is special case */
					top = 1;	/* Make sure it's not zero! */
				}
			}
			player->deltaviewheight = top;		/* Save delta */
		}
	}
	bob += player->mo->z + player->viewheight;	/* Set the view z */
	top = player->mo->ceilingz-(4*FRACUNIT);	/* Adjust for ceiling height */
	if (bob > top) {	/* Did I hit my head on the ceiling? */
		bob = top;		/* Peg at the ceiling */
	}
	player->viewz = bob;		/* Set the new z coord */
}

/**********************************

	Take all the motion constants and apply it
	to the player. Allow clipping and bumping.

**********************************/

static void MoveThePlayer(player_t *player)
{
	angle_t newangle;
	mobj_t *MObjPtr;
	
	MObjPtr = player->mo;		/* Cache the MObj */
	newangle = MObjPtr->angle;	/* Get the angle */
	newangle += player->angleturn;	/* Adjust turning angle always */
	MObjPtr->angle = newangle;	/* Save it, but keep for future use */
	
	/* Don't let the player control movement if not onground */

	onground = (MObjPtr->z <= MObjPtr->floorz);	/* Save for PlayerCalcHeight */

	if (onground) {		/* Can I move? */
		PlayerThrust(MObjPtr,newangle,player->forwardmove);
		PlayerThrust(MObjPtr,newangle-ANG90,player->sidemove);
	}

	if ( (player->forwardmove || player->sidemove) &&		/* Am I moving? */
		MObjPtr->state == &states[S_PLAY] ) {	/* Normal play? */
		SetMObjState(MObjPtr,&states[S_PLAY_RUN1]);		/* Set the sprite */
	}
}

/**********************************

	I am dead, just view the critter or player that killed me.

**********************************/

#define ANG5 (ANG90/18)		/* Move in increments of 5 degrees */

static void P_DeathThink(player_t *player)
{
	angle_t angle;		/* Angle to look at enemy */
	angle_t delta;		/* Angle differance from current angle to enemy angle */

	MovePSprites(player);	/* Animate the weapon sprites and shoot if possible */

/* fall to the ground */

	if (player->viewheight > 8*FRACUNIT) {		/* Still above the ground */
		player->viewheight -= (ElapsedTime<<FRACBITS);	/* Fall over */
		if (player->viewheight<(8*FRACUNIT)) {	/* Too far down? */
			player->viewheight=8*FRACUNIT;		/* Set to the bottom */
		}
	}
	onground = (player->mo->z <= player->mo->floorz);	/* Get the floor state */
	PlayerCalcHeight(player);		/* Calc the height of the player */

	/* Only face killer if I didn't kill myself or jumped into lava */

	if (player->attacker && player->attacker != player->mo) {
		angle = PointToAngle(player->mo->x,player->mo->y,
			player->attacker->x,player->attacker->y);
		delta = angle - player->mo->angle;		/* Get differance */
		if (delta < ANG5 || delta >= (Word)-ANG5) {
			/* looking at killer, so fade damage flash down */
			player->mo->angle = angle;		/* Set the angle */
			goto DownDamage;			/* Fade down */
		} else if (delta < ANG180) {	/* Which way? */
			player->mo->angle += ANG5;	/* Turn towards the killer */
		} else {
			player->mo->angle -= ANG5;
		}
	} else {
DownDamage:
		if (player->damagecount) {		/* Fade down the redness on the screen */
			player->damagecount-=ElapsedTime;	/* Count down time */
			if (player->damagecount&0x8000) {	/* Negative */
				player->damagecount=0;		/* Force zero */
			}
		}
	}

	if ( (JoyPadButtons & PadUse) && player->viewheight < ((8*FRACUNIT)+1)) {
		player->playerstate = PST_REBORN;		/* Restart the player */
	}
}

/**********************************

	Adjust for wraparound since the pending weapon was added
	with 1 or -1 and then check if the requested weapon is present.

**********************************/

static Boolean WeaponAllowed(player_t *player)
{
	if (player->pendingweapon&0x8000) {		/* Handle wrap around for weapon */
		player->pendingweapon=(weapontype_t)(NUMWEAPONS-1);	/* Highest weapon allowed */
	}
	if (player->pendingweapon>=NUMWEAPONS) {	/* Too high? */
		player->pendingweapon = (weapontype_t)0;	/* Reset to the first */
	}
	if (player->weaponowned[player->pendingweapon]) {	/* Do I have this? */
		return TRUE;		/* Yep! */
	}
	return FALSE;		/* Nope, don't select this */
}

/**********************************

	Perform all the actions to move the player
	by reading the joypad information and acting upon it.
	(Called from Tick.c)

**********************************/

void P_PlayerThink(player_t *player)
{
	Word buttons;		/* Current joypad buttons */
	Word i;

	buttons = JoyPadButtons;		/* Get the joypad info */

	P_PlayerMobjThink(player->mo);		/* Perform the inertia movement */
	P_BuildMove(player);			/* Convert joypad info to motion */

/* I use MF_JUSTATTACKED when the chainsaw is being used */

	if (player->mo->flags & MF_JUSTATTACKED) {	/* Chainsaw attack? */
		player->angleturn = 0;			/* Don't allow player turning. */
		player->forwardmove = 0xc800;	/* Moving into the enemy */
		player->sidemove = 0;			/* Don't move side to side */
		player->mo->flags &= ~MF_JUSTATTACKED;	/* Clear the flag */
	}

	if (player->playerstate == PST_DEAD) {		/* Am I dead? */
		P_DeathThink(player);		/* Just face your killer */
		return;				/* Exit now */
	}

/* Reactiontime is used to prevent movement for a bit after a teleport */

	i = player->mo->reactiontime;		/* Get the reaction time */
	if (!i) {				/* Am I active? */
		MoveThePlayer(player);		/* Move the player */
	} else {
		if (ElapsedTime<i) {	/* Subtraction factor */
			i-=ElapsedTime;		/* Remove time base */
		} else {
			i = 0;				/* Force zero */
		}
		player->mo->reactiontime=i;		/* Save the new reaction time */
	}
	PlayerCalcHeight(player);		/* Adjust the player's z coord */

	{
	sector_t *sector;			/* Local pointer */
	sector = player->mo->subsector->sector;		/* Get sector I'm standing on */
	if (sector->special) {	/* Am I standing on a special? */
		PlayerInSpecialSector(player,sector);		/* Process special event */
	}
	}

/* Process use actions */

	if (buttons & PadUse) {
		if (player->pendingweapon == wp_nochange) {
        	i = (buttons^PrevJoyPadButtons)&buttons;	/* Get button downs */
    	    if (i&(PadRightShift|PadLeftShift)) {	/* Pressed the shifts? */
        		i = (PadRightShift&i) ? 1 : -1;	/* Cycle up or down? */
	        	player->pendingweapon=player->readyweapon;	/* Init the weapon */
		        do {
					        /* Cycle to next weapon */
		        	player->pendingweapon=(weapontype_t)(player->pendingweapon+i);
				} while (!WeaponAllowed(player));	/* Ok to keep? */
			}
		}

		if (!player->usedown) {		/* Was use held down? */
			P_UseLines(player);		/* Nope, process the use button */
			player->usedown = TRUE;	/* Wait until released */
		}
	} else {
		player->usedown = FALSE;	/* Use is released */
	}

/* Process weapon attacks */

	if (buttons & PadAttack) {			/* Am I attacking? */
		player->attackdown+=ElapsedTime;		/* Add in the timer */
		if (player->attackdown >= (TICKSPERSEC*2)) {
			stbar.specialFace = f_mowdown;
		}
	} else {
		player->attackdown = FALSE;		/* Reset the timer */
	}

	MovePSprites(player);		/* Process the weapon sprites and shoot */

/* Timed counters */

	if (player->powers[pw_strength] && player->powers[pw_strength]<255) {
		/* Strength counts up to diminish fade */
		player->powers[pw_strength]+=ElapsedTime;	/* Add some time */
		if (player->powers[pw_strength]>=256) {	/* Time up? */
			player->powers[pw_strength] = 255;	/* Maximum */
		}
	}

/* Count down timers for powers and screen colors */

	if (player->powers[pw_invulnerability]) {		/* God mode */
		player->powers[pw_invulnerability]-=ElapsedTime;
		if (player->powers[pw_invulnerability]&0x8000) {
			player->powers[pw_invulnerability]=0;
		}
	}
	
	if (player->powers[pw_invisibility]) {		/* Invisible? */
		player->powers[pw_invisibility]-=ElapsedTime;
		if (player->powers[pw_invisibility]&0x8000) {
			player->powers[pw_invisibility] = 0;
		}
		if (!player->powers[pw_invisibility]) {
			player->mo->flags &= ~MF_SHADOW;
		}
	}

	if (player->powers[pw_ironfeet]) {		/* Radiation suit */
		player->powers[pw_ironfeet]-=ElapsedTime;
		if (player->powers[pw_ironfeet]&0x8000) {
			player->powers[pw_ironfeet]=0;
		}
	}

	if (player->damagecount) {			/* Red factor */
		player->damagecount-=ElapsedTime;
		if (player->damagecount&0x8000) {
			player->damagecount=0;
		}
	}

	if (player->bonuscount) {			/* Gold factor */
		player->bonuscount-=ElapsedTime;
		if (player->bonuscount&0x8000) {
			player->bonuscount=0;
		}
	}
}
