#include "Doom.h"
#include <String.h>

/**********************************

	Prepare to load a game level

**********************************/

void G_DoLoadLevel(void)
{
	Word Sky;

	if (players.playerstate == PST_DEAD) {
		players.playerstate = PST_REBORN;	/* Force rebirth */
	}
	
/* Set the sky map for the episode */

	if (gamemap < 9 || gamemap==24) {			/* First 9 levels? */
		Sky = rSKY1;
	} else if (gamemap < 18) {
		Sky = rSKY2;
	} else {
		Sky = rSKY3;
	}
 	SkyTexture = &TextureInfo[Sky-FirstTexture];	/* Set pointer to sky texture record */
	SkyTexture->data = LoadAResourceHandle(Sky);		/* Preload the sky texture */
	SetupLevel(gamemap);	/* Load the level into memory */
	gameaction = ga_nothing;		/* Game in progress */
}

/**********************************

	Call when a player completes a level

**********************************/

void G_PlayerFinishLevel(void)
{
	player_t *p; 		/* Local pointer */

	p = &players;
	memset(p->powers,0,sizeof(p->powers));	/* Remove powers */
	memset(p->cards,0,sizeof(p->cards));	/* Remove keycards and skulls */
	if (p->mo) {
		p->mo->flags &= ~MF_SHADOW;				/* Allow me to be visible */
	}
	p->extralight = 0;                      /* cancel gun flashes */
	p->fixedcolormap = 0;                   /* cancel ir gogles */
	p->damagecount = 0;                     /* no palette changes */
	p->bonuscount = 0;						/* cancel backpack */
}

/**********************************

	Called after a player dies
	almost everything is cleared and initialized

**********************************/

void G_PlayerReborn(void)
{
	player_t *p;		/* Local */
	Word i;

	p = &players;	/* Get local pointer */
	memset(p,0,sizeof(*p));	/* Zap the player */
	p->usedown = p->attackdown = TRUE;	/* don't do anything immediately */
	p->playerstate = PST_LIVE;	/* I live again! */
	p->health = MAXHEALTH;		/* Restore health */
	p->readyweapon = p->pendingweapon = wp_pistol;	/* Reset weapon */
	p->weaponowned[wp_fist] = TRUE;		/* I have a fist */
	p->weaponowned[wp_pistol] = TRUE;	/* And a pistol */
	p->ammo[am_clip] = 50;			/* Award 50 bullets */
	i = 0;
	do {
		p->maxammo[i] = maxammo[i];	/* Reset ammo counts (No backpack) */
	} while (++i<NUMAMMO);
}


/**********************************

	Player is reborn after death

**********************************/

void G_DoReborn(void)
{
	gameaction = ga_died;	/* Reload the level from scratch */
}

/**********************************

	Set flag for normal level exit

**********************************/

void G_ExitLevel(void)
{
	gameaction = ga_completed;
}

/**********************************

	Set flag for secret level exit

**********************************/

void G_SecretExitLevel(void)
{
	gameaction = ga_secretexit;
}

/**********************************

	Init variables for a new game

**********************************/

void G_InitNew(skill_t skill,Word map)
{
	Randomize();		/* Reset the random number generator */

	gamemap = map;
	gameskill = skill;

/* Force players to be initialized upon first level load */

	players.playerstate = PST_REBORN;
	players.mo = 0;	/* For net consistancy checks */
	
	DemoRecording = FALSE;		/* No demo in progress */
	DemoPlayback = FALSE;

	if (skill == sk_nightmare ) {		/* Hack for really BAD monsters */
		states[S_SARG_ATK1].Time = 2*4;	/* Speed up the demons */
		states[S_SARG_ATK2].Time = 2*4;
		states[S_SARG_ATK3].Time = 2*4;
		mobjinfo[MT_SERGEANT].Speed = 15;
		mobjinfo[MT_SHADOWS].Speed = 15;
		mobjinfo[MT_BRUISERSHOT].Speed = 40;	/* Baron of hell */
		mobjinfo[MT_HEADSHOT].Speed = 40;		/* Cacodemon */
		mobjinfo[MT_TROOPSHOT].Speed = 40;
	} else {
		states[S_SARG_ATK1].Time = 4*4;		/* Set everyone back to normal */
		states[S_SARG_ATK2].Time = 4*4;
		states[S_SARG_ATK3].Time = 4*4;
		mobjinfo[MT_SERGEANT].Speed = 8;
		mobjinfo[MT_SHADOWS].Speed = 8;
		mobjinfo[MT_BRUISERSHOT].Speed = 30;
		mobjinfo[MT_HEADSHOT].Speed = 20;
		mobjinfo[MT_TROOPSHOT].Speed = 20;
	}
}

/**********************************

	The game should already have been initialized or loaded

**********************************/

void G_RunGame(void)
{
	for (;;) {

	/* Run a level until death or completion */

		MiniLoop(P_Start,P_Stop,P_Ticker,P_Drawer);

	/* Take away cards and stuff */

		G_PlayerFinishLevel();
		if ((gameaction == ga_died) ||	/* died, so restart the level */
			(gameaction == ga_warped)) {	/* skip intermission */
			continue;
		}

	/* decide which level to go to next */

		if (gameaction == ga_secretexit) {
			 nextmap = 24;	/* Go to the secret level */
		} else {
			switch (gamemap) {
			case 24:		/* Secret level? */
				nextmap = 4;
				break;
			case 23:		/* Final level! */
				nextmap = 23;
				break;		/* Don't add secret level to prefs */
			default:
				nextmap = gamemap+1;
			}
			if (nextmap>MaxLevel) {
				MaxLevel = nextmap;	/* Save the prefs file */
				WritePrefsFile();
			}
		}

	/* Run a stats intermission */

		MiniLoop(IN_Start,IN_Stop,IN_Ticker,IN_Drawer);

	/* Run the finale if needed */

		if (gamemap == 23) {
			MiniLoop(F_Start,F_Stop,F_Ticker,F_Drawer);
			return;		/* Exit */
		}
		gamemap = nextmap;
	}
}

/**********************************

	Play a demo using a pointer to the demo data

**********************************/

Word G_PlayDemoPtr(Word *demo)
{
	Word exit;
	Word skill,map;

	DemoBuffer = demo;		/* Save the demo buffer pointer */
	skill = demo[0];		/* Get the initial and map */
	map = demo[1];
	DemoDataPtr = &demo[2];		/* Init the pointer */
	G_InitNew((skill_t)skill,map);	/* Init a game */
	DemoPlayback = TRUE;	/* I am playing back data */
	exit = MiniLoop(P_Start,P_Stop,P_Ticker,P_Drawer);	/* Execute game */
	DemoPlayback = FALSE;	/* End demo */
	return exit;
}

/**********************************

	Record a demo
	Only used in testing.

**********************************/

void G_RecordDemo (void)
{
	Word *Dest;

	Dest = (Word *)AllocAPointer(0x8000);		/* Get memory for demo */
	DemoBuffer = Dest;			/* Save the pointer */
	Dest[0] = StartSkill;		/* Save the skill and level */
	Dest[1] = StartMap;
    DemoDataPtr = Dest+2;
	G_InitNew(StartSkill,StartMap);	/* Begin a game */
	DemoRecording = TRUE;		/* Begin recording */
	MiniLoop(P_Start,P_Stop,P_Ticker,P_Drawer);	/* Play it */
	DemoRecording = FALSE;		/* End recording */
	for (;;) {					/* Stay forever */
		G_PlayDemoPtr(DemoBuffer);	/* Play the demo */
	}
}
