#include "Doom.h"

#define	BONUSADD 16		/* Time adder for bonus color */
#define	BASETHRESHOLD (7*TICKSPERSEC/4)	/* Number of tics to exclusivly follow a target */

/* A weapon is found with two clip loads, a big item has five clip loads */

static Word clipammo[NUMAMMO] = {10,4,20,1};	/* Ammo for a normal clip */

/**********************************

	Num is the number of clip loads, not the individual count (0= 1/2 clip)
	Returns false if the ammo can't be picked up at all
	Also I switch weapons if I am using a wimpy weapon and I get ammo
	for a much better weapon.

**********************************/

static Word GiveAmmo(player_t *player,ammotype_t ammo,Word numofclips)
{
	Word oldammo;
	Word maxammo;

	if (ammo == am_noammo || ammo>=NUMAMMO) {		/* Is this not ammo? */
		return FALSE;			/* Can't pick it up */
	}

	oldammo = player->ammo[ammo];		/* Get the current ammo */
	maxammo = player->maxammo[ammo];	/* Get the maximum */

	if (oldammo >= maxammo) {	/* Full already? */
		return FALSE;		/* Can't pick it up */
	}

	if (numofclips) {			/* Valid ammo count? */
		numofclips *= clipammo[ammo];		/* Get the ammo adder */
	} else {
		numofclips = clipammo[ammo]/2;		/* Give a half ration */
	}
	if (gameskill == sk_baby) {
		numofclips <<= 1;			/* give double ammo in wimp mode */
	}
	oldammo += numofclips;			/* Add in the new ammo */
	if (oldammo >= maxammo) {
		oldammo = maxammo;		/* Max it out */
	}
	player->ammo[ammo] = oldammo;		/* Save the new ammo count */

	if (oldammo!=numofclips) {	/* Only possible if oldammo == 0 */
		return TRUE;		/* Don't change up weapons, player was lower on */
	}						/* purpose */

	switch (ammo) {		/* Which type was picked up */
	case am_clip:
		if (player->readyweapon == wp_fist) {	/* I have fists up? */
			if (player->weaponowned[wp_chaingun]) {	/* Start the chaingun */
				player->pendingweapon = wp_chaingun;
			} else {
				player->pendingweapon = wp_pistol;	/* Try the pistol */
			}
		}
		break;
	case am_shell:
		if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol) {
			if (player->weaponowned[wp_shotgun]) {
				player->pendingweapon = wp_shotgun;
			}
		}
		break;
	case am_cell:
		if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol) {
			if (player->weaponowned[wp_plasma]) {
				player->pendingweapon = wp_plasma;
			}
		}
		break;
	case am_misl:
		if (player->readyweapon == wp_fist) {		/* Only using fists? */
			if (player->weaponowned[wp_missile]) {
				player->pendingweapon = wp_missile;	/* Use rocket launcher */
			}
		}
	}
	return TRUE;			/* I picked it up! */
}

/**********************************

	Pick up a weapon.
	The weapon name may have a MF_DROPPED flag or'd in
	so that it will affect the amount of ammo contained inside.

**********************************/

static Word GiveWeapon(player_t *player,weapontype_t weapon,Word dropped)
{
	Word PickedUp;

	PickedUp = FALSE;		/* Init my vars */

	/* Give one clip with a dropped weapon, two clips with a found weapon */

	if (WeaponAmmos[weapon] != am_noammo) {		/* Any ammo inside? */
		dropped = dropped ? 1 : 2;		/* 1 or 2 clips */
		PickedUp = GiveAmmo(player,WeaponAmmos[weapon],dropped);
	}

	if (!player->weaponowned[weapon]) {		/* Already had such a weapon? */
		PickedUp = TRUE;
		player->weaponowned[weapon] = TRUE;		/* I have it now */
		player->pendingweapon = weapon;			/* Use this weapon */
		stbar.specialFace = f_gotgat;		/* He he he! Evil grin! */	
	}
	return PickedUp;		/* Did you pick it up? */
}

/**********************************

	Increase the player's health
	Returns false if the health isn't needed at all

**********************************/

static Word GiveBody(player_t *player,Word num)
{
	if (player->health >= MAXHEALTH) {		/* Already maxxed out? */
		return FALSE;		/* Don't get anymore */
	}
	num += player->health;		/* Make new health */
	if (num >= MAXHEALTH) {
		num = MAXHEALTH;		/* Set to maximum */
	}
	player->health = num;		/* Save the new health */
	player->mo->MObjHealth = num;	/* Save in MObj record as well */
	return TRUE;				/* Pick it up */
}

/**********************************

	Award a new suit of armor.
	Returns false if the armor is worse than the current armor

**********************************/

static Word GiveArmor(player_t *player,Word armortype)
{
	Word hits;

	hits = armortype*100;		/* 100 or 200% */
	if (player->armorpoints >= hits) {	/* Already has this armor? */
		return FALSE;		/* Don't pick up */
	}
	player->armortype = armortype;	/* Set the type */
	player->armorpoints = hits;		/* Set the new value */
	return TRUE;			/* Pick it up */
}

/**********************************

	Award a keycard or skull key

**********************************/

static void GiveCard(player_t *player,card_t card)
{
	if (!player->cards[card]) {		/* I don't have it already? */
		player->bonuscount = BONUSADD;	/* Add the bonus value for color */
		player->cards[card] = TRUE;		/* I have it now! */
	}
}

/**********************************

	Award a powerup

**********************************/

static Word GivePower(player_t *player,powertype_t power)
{
	switch (power) {
	case pw_invulnerability:		/* God mode? */
		player->powers[power] = INVULNTICS;	/* Set the time */
		break;
	case pw_invisibility:
		player->powers[power] = INVISTICS;	/* I am invisible! */
		player->mo->flags |= MF_SHADOW;
		break;
	case pw_ironfeet:			/* Radiation suit? */
		player->powers[power] = IRONTICS;	/* Set the time */
		break;
	case pw_strength:			/* Berzerker pack */
		GiveBody(player,100);				/* Full health */
		player->powers[power] = TRUE;		/* I have the power */
		break;
	default:
		if (player->powers[power]) {			/* Already have the power up? */
			return FALSE;		/* Already got it, don't get it again */
		}
		player->powers[power] = TRUE;		/* Award the power up */
	}
	return TRUE;			/* Pick it up */
}

/**********************************

	The case statement got too big, moved the rest here.
	Returns sound to play, or -1 if no sound

**********************************/

static Word TouchSpecialThing2(mobj_t *toucher,Word Sprite)
{
	player_t *player;

	player = toucher->player;

	switch (Sprite) {
	case rSPR_GREENARMOR:			/* Green armor */
		if (!GiveArmor(player,1)) {
			return -1;
		}
		player->message = "You pick up the armor.";
		break;

	case rSPR_BLUEARMOR:			/* Blue armor */
		if (!GiveArmor(player,2)) {
			return -1;
		}
		player->message = "You got the MegaArmor!";
		break;

/* Cards, leave cards for everyone */

	case rSPR_BLUEKEYCARD:
		if (!player->cards[it_bluecard]) {
			player->message = "You pick up a blue keycard.";
		}
		GiveCard(player,it_bluecard);
CardSound:
		break;
		
	case rSPR_YELLOWKEYCARD:
		if (!player->cards[it_yellowcard]) {
			player->message = "You pick up a yellow keycard.";
		}
		GiveCard(player,it_yellowcard);
		goto CardSound;
	case rSPR_REDKEYCARD:
		if (!player->cards[it_redcard]) {
			player->message = "You pick up a red keycard.";
		}
		GiveCard(player,it_redcard);
		goto CardSound;
	case rSPR_BLUESKULLKEY:
		if (!player->cards[it_blueskull]) {
			player->message = "You pick up a blue skull key.";
		}
		GiveCard(player,it_blueskull);
		goto CardSound;
	case rSPR_YELLOWSKULLKEY:
		if (!player->cards[it_yellowskull]) {
			player->message = "You pick up a yellow skull key.";
		}
		GiveCard(player,it_yellowskull);
		goto CardSound;
	case rSPR_REDSKULLKEY:
		if (!player->cards[it_redskull]) {
			player->message = "You pick up a red skull key.";
		}
		GiveCard(player,it_redskull);
		goto CardSound;

/* Heals */

	case rSPR_STIMPACK:			/* Stim pack */
		if (!GiveBody(player,10)) {
			return -1;
		}
		player->message = "You pick up a stimpack.";
		break;
	case rSPR_MEDIKIT:
		if (!GiveBody(player,25)) {		/* Medkit */
			return -1;
		}
		if (player->health<50) {
			player->message = "You pick up a medikit that you REALLY need!";
		} else {
			player->message = "You pick up a medikit.";
		}
		break;

/* Power ups */

	case rSPR_INVULNERABILITY:		/* God mode!! */
		if (!GivePower(player,pw_invulnerability)) {
			return -1;
		}
		player->message = "Invulnerability!";
		break;
	case rSPR_BERZERKER:		/* Berserker pack */
		if (!GivePower(player,pw_strength)) {
			return -1;
		}
		player->message = "Berserk!";
		if (player->readyweapon != wp_fist) {	/* Already fists? */
			player->pendingweapon = wp_fist;	/* Set to fists */
		}
		break;
	case rSPR_INVISIBILITY:			/* Invisibility */
		if (!GivePower(player,pw_invisibility)) {
			return -1;
		}
		player->message = "Invisibility!";
		break;
	case rSPR_RADIATIONSUIT:			/* Radiation suit */
		if (!GivePower(player,pw_ironfeet)) {
			return -1;
		}
		player->message = "Radiation Shielding Suit";
		break;
	case rSPR_COMPUTERMAP:			/* Computer map */
		if (!GivePower(player,pw_allmap)) {
			return -1;
		}
		player->message = "Computer Area Map";
		break;
	case rSPR_IRGOGGLES:				/* Light amplification visor */
		break;
	}
	return sfx_itemup;			/* Return the sound effect */
}

/**********************************

	Award the item to the player if needed

**********************************/

void TouchSpecialThing(mobj_t *special,mobj_t *toucher)
{
	player_t *player;
	Word i;
	Fixed delta;
	Word sound;

	delta = special->z - toucher->z;	/* Differances between z's */
	if (delta > toucher->height || (delta < (-8*FRACUNIT))) {
		return;			/* Out of reach */
	}

	player = toucher->player;
	if (!toucher->MObjHealth) {		/* Dead player shape? */
		return;		/* Can happen with a sliding player corpse */
	}
	sound = sfx_itemup;		/* Get item sound */
	i = special->state->SpriteFrame>>FF_SPRITESHIFT;
	switch (i) {

/* bonus items */

	case rSPR_HEALTHBONUS:			/* Health bonus */
		player->message = "You pick up a health bonus.";
		i = 2;				/* Award size */
Healthy:
		i += player->health;	/* Can go over 100% */
		if (i >= 201) {
			i = 200;
		}
		player->health = i;		/* Save new health */
		player->mo->MObjHealth = i;
		break;
	case rSPR_SOULSPHERE:		/* Supercharge sphere */
		player->message = "Supercharge!";
		i = 100;		/* Award 100 points */
		goto Healthy;
	case rSPR_ARMORBONUS:		/* Armor bonus */
		i = player->armorpoints+2;		/* Can go over 100% */
		if (i >= 201) {
			i = 200;			/* But not 200%! */
		}
		player->armorpoints = i;
		if (!player->armortype) {	/* Any armor? */
			player->armortype = 1;	/* Set to green type */
		}
		player->message = "You pick up an armor bonus.";
		break;

/* ammo */

	case rSPR_CLIP:
		i = (special->flags & MF_DROPPED) ? 0 : 1;	/* Half or full clip */
		if (!GiveAmmo(player,am_clip,i)) {	/* Give the ammo */
			return;
		}
		player->message = "Picked up a clip.";
		break;
	case rSPR_BOXAMMO:
		if (!GiveAmmo(player,am_clip,5)) {	/* Give 5 clips worth */
			return;
		}
		player->message = "Picked up a box of bullets.";
		break;
	case rSPR_ROCKET:
		if (!GiveAmmo(player,am_misl,1)) {		/* 1 clip of rockets */
			return;
		}
		player->message = "Picked up a rocket.";
		break;
	case rSPR_BOXROCKETS:
		if (!GiveAmmo(player,am_misl,5)) {		/* 5 rockets */
			return;
		}
		player->message = "Picked up a box of rockets.";
		break;
	case rSPR_CELL:
		if (!GiveAmmo(player,am_cell,1)) {		/* Single charge cell */
			return;
		}
		player->message = "Picked up an energy cell.";
		break;
	case rSPR_CELLPACK:
		if (!GiveAmmo(player,am_cell,5)) {	/* Big energy cell */
			return;
		}
		player->message = "Picked up an energy cell pack.";
		break;
	case rSPR_SHELLS:
		if (!GiveAmmo(player,am_shell,1)) {	/* Clip of shells */
			return;
		}
		player->message = "Picked up 4 shotgun shells.";
		break;
	case rSPR_BOXSHELLS:
		if (!GiveAmmo(player,am_shell,5)) {	/* Box of shells */
			return;
		}
		player->message = "Picked up a box of shotgun shells.";
		break;
	case rSPR_BACKPACK:
		if (!player->backpack) {		/* Already got a backpack? */
			i = 0;
			do {
				player->maxammo[i] *= 2;	/* Double the max ammo */
			} while (++i<NUMAMMO);
			player->backpack = TRUE;		/* I have a backpack now */
		}
		i = 0;
		do {
			GiveAmmo(player,(ammotype_t)i,1);	/* 1 clip of everything */
		} while (++i<NUMAMMO);
		player->message = "Picked up a backpack full of ammo!";
		break;

/* weapons */

	case rSPR_BFG9000:		/* BFG 9000 */
		if (!GiveWeapon(player,wp_bfg,FALSE) ) {
			return;
		}
		player->message = "You got the BFG9000!  Oh, yes.";
		sound = sfx_wpnup;
		break;
	case rSPR_CHAINGUN:		/* Chain gun */
		if (!GiveWeapon(player,wp_chaingun,FALSE) ) {
			return;
		}
		player->message = "You got the chaingun!";
		sound = sfx_wpnup;
		break;
	case rSPR_CHAINSAW:		/* Chainsaw */
		if (!GiveWeapon(player,wp_chainsaw,FALSE) ) {
			return;
		}
		player->message = "A chainsaw!  Find some meat!";
		sound = sfx_wpnup;
		break;
	case rSPR_ROCKETLAUNCHER:		/* Rocket launcher */
		if (!GiveWeapon(player,wp_missile,FALSE) ) {
			return;
		}
		player->message = "You got the rocket launcher!";
		sound = sfx_wpnup;
		break;
	case rSPR_PLASMARIFLE:		/* Plasma rifle */
		if (!GiveWeapon(player,wp_plasma,FALSE)) {
			return;
		}
		player->message = "You got the plasma gun!";
		sound = sfx_wpnup;
		break;
	case rSPR_SHOTGUN:		/* Shotgun */
		if (!GiveWeapon(player,wp_shotgun,(special->flags&MF_DROPPED) ? TRUE : FALSE)) {
			return;
		}
		player->message = "You got the shotgun!";
		sound = sfx_wpnup;
		break;
	default:		/* None of the above? */
		sound = TouchSpecialThing2(toucher,i);	/* Try extra code */
		if (sound == -1) {	/* No good? */
			return;		/* Exit */
		}
	}

	if (special->flags & MF_COUNTITEM) {	/* Bonus items? */
		++player->itemcount;			/* Add to the item count */
	}
	P_RemoveMobj(special);				/* Remove the item */
	player->bonuscount += BONUSADD;		/* Add to the bonus count color */
	S_StartSound(&toucher->x,sound);		/* Play the sound */
}

/**********************************

	The mobj was just killed, adjust the totals
	and turn the object into death warmed over

**********************************/

static void KillMobj(mobj_t *target,Word Overkill)
{
	mobjinfo_t *InfoPtr;
	mobj_t *mo;

	InfoPtr = target->InfoPtr;
	
	target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY);	/* Clear flags */
	if (InfoPtr != &mobjinfo[MT_SKULL]) {		/* Skulls can stay in the air */
		target->flags &= ~MF_NOGRAVITY;	/* Make eye creatures fall */
	}
	target->flags |= MF_CORPSE|MF_DROPOFF;		/* It's dead and can fall */
	target->height >>= 2;			/* Reduce the height a lot */

	if (target->flags & MF_COUNTKILL) {
		++players.killcount;			/* Count all monster deaths, even */
	}									/* those caused by other monsters */

	if (target->player) {			/* Was the dead one a player? */
		target->flags &= ~MF_SOLID;		/* Walk over the body! */
		target->player->playerstate = PST_DEAD;	/* You are dead! */
		LowerPlayerWeapon(target->player);		/* Drop current weapon on screen */
		if (target->player == &players) {
			stbar.gotgibbed = TRUE;		/* Gooey! */
		}
		if (Overkill>=50) {			/* Were you a real mess? */
			S_StartSound(&target->x,sfx_slop);	/* Juicy, gorey death! */
		} else {
			S_StartSound(&target->x,sfx_pldeth);	/* Arrrgh!! */
		}
	}

	if (Overkill >= InfoPtr->spawnhealth		/* Horrible death? */
		&& InfoPtr->xdeathstate) {
		SetMObjState(target,InfoPtr->xdeathstate);	/* Death state */
	} else {
		SetMObjState(target,InfoPtr->deathstate);	/* Boring death */
	}
	Sub1RandomTick(target);		/* Add a little randomness to the gibbing time */

/* Drop stuff */

	if (InfoPtr == &mobjinfo[MT_POSSESSED]) {
		InfoPtr = &mobjinfo[MT_CLIP];
	} else if (InfoPtr == &mobjinfo[MT_SHOTGUY]) {
		InfoPtr = &mobjinfo[MT_SHOTGUN];
	} else {
		return;
	}
	mo = SpawnMObj(target->x,target->y,ONFLOORZ,InfoPtr);	/* Drop it */
	mo->flags |= MF_DROPPED;		/* Avoid respawning! */
}

/**********************************

	Damages both enemies and players
	inflictor is the thing that caused the damage
		creature or missile, can be NULL (slime, etc)
	source is the thing to target after taking damage
		creature or NULL
	Source and inflictor are the same for melee attacks
	source can be null for barrel explosions and other environmental stuff

**********************************/

void DamageMObj(mobj_t *target,mobj_t *inflictor,mobj_t *source,Word damage)
{
	angle_t ang;		/* Angle of impact */
	Word an;			/* Index to angle table */
	player_t *player;	/* Player record of killer */
	Fixed thrust;		/* Thrust of death motion */
	Word saved;			/* Damage armor prevented */

	if ( !(target->flags & MF_SHOOTABLE) ) { /* Failsafe */
		return;
	}

	if (!target->MObjHealth) {		/* Already dead? */
		return;
	}

	if (target->flags & MF_SKULLFLY) {		/* Stop a skull from flying */
		target->momx = target->momy = target->momz = 0;
	}

	player = target->player;
	if (player) {			/* Handle player damage */
		if (gameskill == sk_baby) {
			damage >>= 1;				/* take half damage in trainer mode */
		}
		if ((damage >= 31) && player == &players) {
			stbar.specialFace = f_hurtbad;		/* Ouch face */
		}
	}

/* Kick away unless using the chainsaw */

	if (inflictor && (!source || !source->player
	|| source->player->readyweapon != wp_chainsaw)) {
		ang = PointToAngle(inflictor->x,inflictor->y,target->x,target->y);
		thrust = (damage*(25*FRACUNIT))/target->InfoPtr->mass;

		/* make fall forwards sometimes */
		if (damage<40 && damage>target->MObjHealth &&
			 ((target->z - inflictor->z) > 64*FRACUNIT) && GetRandom(1) ) {
			ang += ANG180;		/* Do a 180 degree turn */
			thrust *= 4;		/* Move a little faster */
		}
		an = ang>>ANGLETOFINESHIFT;		/* Convert to sine table value */
		thrust >>= FRACBITS;			/* Get the integer */
		target->momx += thrust * finecosine[an];	/* Get some momentum */
		target->momy += thrust * finesine[an];
	} else {
		ang = target->angle;		/* Get facing */
	}

/* player specific */

	if (player) {
		if ( (player->AutomapFlags&AF_GODMODE)||player->powers[pw_invulnerability] ) {
			return;		/* Don't hurt in god mode */
		}
			/* Where did the attack come from? */
		if (player == &players) {
			ang -= target->angle;		/* Get angle differance */
			if (ang >= 0x30000000UL && ang < 0x80000000UL) {
				stbar.specialFace = f_faceright;		/* Face toward attacker */
			} else if (ang >= 0x80000000UL && ang < 0xD0000000UL) {
				stbar.specialFace = f_faceleft;
			}
		}
		if (player->armortype) {		/* Remove damage using armor */
			if (player->armortype == 1) {	/* Normal armor */
				saved = damage/3;
			} else {
				saved = damage/2;		/* Mega armor */
			}
			if (player->armorpoints <= saved) {	/* Armor is used up */
				saved = player->armorpoints;	/* Use the maximum from armor */
				player->armortype = 0;		/* Remove the previous armor */
			}
			player->armorpoints -= saved;	/* Deduct from armor */
			damage -= saved;			/* Deduct damage from armor */
		}
		S_StartSound(&target->x,sfx_plpain);	/* Ouch! */
		if (player->health <= damage) {	/* Mirror mobj health here for Dave */
			player->health = 0;		/* You died! */
		} else {
			player->health -= damage;	/* Remove health */
		}
		player->attacker = source;		/* Mark the source of the attack */
		player->damagecount += (damage<<1);	/* Add damage after armor / invuln */
	}

/* Do the damage */

	if (target->MObjHealth <= damage) {	/* Killed? */
		Word Ouch;			/* Get the overkill factor */
		Ouch = damage-target->MObjHealth;
		target->MObjHealth = 0;		/* Zap the health */
		KillMobj(target,Ouch);	/* Perform death animation */
		return;			/* Exit now */
	}
	target->MObjHealth -= damage;	/* Remove damage from target */

	if ( (GetRandom(255)<target->InfoPtr->painchance) && 	/* Should it react in pain? */
		!(target->flags&MF_SKULLFLY) ) {
		target->flags |= MF_JUSTHIT;		/* fight back! */
		SetMObjState(target,target->InfoPtr->painstate);
	}
	target->reactiontime = 0;		/* We're awake now... */

/* If not intent on another player, chase after this one */

	if (!target->threshold && source) {
		target->target = source;		/* Target the attacker */
		target->threshold = BASETHRESHOLD;		/* Reset the threshold */
		if (target->state == target->InfoPtr->spawnstate
		&& target->InfoPtr->seestate) {
			SetMObjState(target,target->InfoPtr->seestate);	/* Reset actor */
		}
	}
}
