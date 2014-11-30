#include "Doom.h"

/**********************************

	Machine independent data for DOOM!

**********************************/

ammotype_t WeaponAmmos[NUMWEAPONS] = {	/* Ammo types for all weapons */
	am_noammo,		/* Fists */
	am_clip,		/* Pistol */
	am_shell,		/* Shotgun */
	am_clip,		/* Chain gun */
	am_misl,		/* Rocket launcher */
	am_cell,		/* Plasma rifle */
	am_cell,		/* BFG 9000 */
	am_noammo		/* Chainsaw */
};
Word maxammo[NUMAMMO] = {200,50,300,50};		/* Max ammo for ammo types */

Word PadAttack = PadA;	/* Joypad bit for attack */
Word PadUse = PadB;		/* Joypad bit for use */
Word PadSpeed = PadC;	/* Joypad bit for high speed */
Word ControlType;		/* Determine settings for PadAttack,Use,Speed */
Word TotalGameTicks;	/* Total number of ticks since game start */
Word ElapsedTime;		/* Ticks elapsed between frames */
Word MaxLevel;			/* Highest level selectable in menu (1-23) */
Word *DemoDataPtr;		/* Running pointer to demo data */
Word *DemoBuffer;		/* Pointer to demo data */
Word JoyPadButtons;		/* Current joypad */
Word PrevJoyPadButtons;	/* Previous joypad */
Word NewJoyPadButtons;	/* New joypad button downs */
skill_t StartSkill;		/* Default skill level */
Word StartMap;			/* Default map start */
void *BigNumFont;		/* Cached pointer to the big number font */
Word TotalKillsInLevel;		/* Number of monsters killed */
Word ItemsFoundInLevel;		/* Number of items found */
Word SecretsFoundInLevel;		/* Number of secrets discovered */
Word tx_texturelight;	/* Light value to pass to hardware */
Fixed lightsub;
Fixed lightcoef;
Fixed lightmin;
Fixed lightmax;
player_t players;	/* Current player stats */
gameaction_t gameaction;	/* Current game state */
skill_t gameskill;		/* Current skill level */
Word gamemap; 			/* Current game map # */
Word nextmap;			/* The map to go to after the stats */
Word ScreenSize;		/* Screen size to use */
Boolean LowDetail;		/* Use low detail mode */
Boolean DemoRecording;	/* True if demo is being recorded */
Boolean DemoPlayback;	/* True if demo is being played */
Boolean DoWipe;			/* True if I should do the DOOM wipe */
