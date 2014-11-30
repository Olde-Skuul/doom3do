#include "Doom.h"

/**********************************

	This data determines each state of the sprites in DOOM.
	The first value determines the shape GROUP that will be used to draw the sprite.
	The first also will determine which shape in the GROUP that will be drawn. The
		upper bit MAY be or'd with FF_FULLBRIGHT) so that the shape will be at maximum
		brightness at all times.
	The second value is the timer in ticks before the next state is executed. If it is -1 then
		it will NEVER change states. If 0 then I go the next state NOW and I don't care about
		values #1 and 2.
		I assume a time base of 60hz.
	The third value is a procedure pointer to execute when the state is entered.
	The fourth value is the index to the next state when time runs out.

**********************************/

#define Spr(s,f) ((s<<FF_SPRITESHIFT)|f)

state_t	states[NUMSTATES] = {
{Spr(rSPR_HEALTHBONUS,0),-1,0,0},	/* S_NULL Dummy entry */

{Spr(rSPR_BIGFISTS,0),0*4,A_Light0,0},	// S_LIGHTDONE
{Spr(rSPR_BIGFISTS,0),1*4,A_WeaponReady,&states[S_PUNCH]},	// S_PUNCH
{Spr(rSPR_BIGFISTS,0),1*4,A_Lower,&states[S_PUNCHDOWN]},	// S_PUNCHDOWN
{Spr(rSPR_BIGFISTS,0),1*4,A_Raise,&states[S_PUNCHUP]},	// S_PUNCHUP
{Spr(rSPR_BIGFISTS,1),2*4,0,&states[S_PUNCH2]},	// S_PUNCH1
{Spr(rSPR_BIGFISTS,2),2*4,A_Punch,&states[S_PUNCH3]},	// S_PUNCH2
{Spr(rSPR_BIGFISTS,3),2*4,0,&states[S_PUNCH4]},	// S_PUNCH3
{Spr(rSPR_BIGFISTS,2),2*4,0,&states[S_PUNCH5]},	// S_PUNCH4
{Spr(rSPR_BIGFISTS,1),2*4,A_ReFire,&states[S_PUNCH]},	// S_PUNCH5

{Spr(rSPR_BIGPISTOL,0),1*4,A_WeaponReady,&states[S_PISTOL]},	// S_PISTOL
{Spr(rSPR_BIGPISTOL,0),1*4,A_Lower,&states[S_PISTOLDOWN]},	// S_PISTOLDOWN
{Spr(rSPR_BIGPISTOL,0),1*4,A_Raise,&states[S_PISTOLUP]},	// S_PISTOLUP
{Spr(rSPR_BIGPISTOL,0),2*4,0,&states[S_PISTOL2]},	// S_PISTOL1
{Spr(rSPR_BIGPISTOL,1),3*4,A_FirePistol,&states[S_PISTOL3]},	// S_PISTOL2
{Spr(rSPR_BIGPISTOL,2),3*4,0,&states[S_PISTOL4]},	// S_PISTOL3
{Spr(rSPR_BIGPISTOL,1),2*4,A_ReFire,&states[S_PISTOL]},	// S_PISTOL4
{Spr(rSPR_BIGPISTOL,3|FF_FULLBRIGHT),3*4,A_Light1,&states[S_LIGHTDONE]},	// S_PISTOLFLASH

{Spr(rSPR_BIGSHOTGUN,0),1*4,A_WeaponReady,&states[S_SGUN]},	// S_SGUN
{Spr(rSPR_BIGSHOTGUN,0),1*4,A_Lower,&states[S_SGUNDOWN]},	// S_SGUNDOWN
{Spr(rSPR_BIGSHOTGUN,0),1*4,A_Raise,&states[S_SGUNUP]},	// S_SGUNUP
{Spr(rSPR_BIGSHOTGUN,0),2*4,0,&states[S_SGUN2]},	// S_SGUN1
{Spr(rSPR_BIGSHOTGUN,0),2*4,A_FireShotgun,&states[S_SGUN3]},	// S_SGUN2
{Spr(rSPR_BIGSHOTGUN,1),3*4,0,&states[S_SGUN4]},	// S_SGUN3
{Spr(rSPR_BIGSHOTGUN,2),2*4,0,&states[S_SGUN5]},	// S_SGUN4
{Spr(rSPR_BIGSHOTGUN,3),2*4,0,&states[S_SGUN6]},	// S_SGUN5
{Spr(rSPR_BIGSHOTGUN,2),2*4,0,&states[S_SGUN7]},	// S_SGUN6
{Spr(rSPR_BIGSHOTGUN,1),2*4,0,&states[S_SGUN8]},	// S_SGUN7
{Spr(rSPR_BIGSHOTGUN,0),2*4,0,&states[S_SGUN9]},	// S_SGUN8
{Spr(rSPR_BIGSHOTGUN,0),3*4,A_ReFire,&states[S_SGUN]},	// S_SGUN9
{Spr(rSPR_BIGSHOTGUN,4|FF_FULLBRIGHT),2*4,A_Light1,&states[S_SGUNFLASH2]},	// S_SGUNFLASH1
{Spr(rSPR_BIGSHOTGUN,5|FF_FULLBRIGHT),1*4,A_Light2,&states[S_LIGHTDONE]},	// S_SGUNFLASH2

{Spr(rSPR_BIGCHAINGUN,0),1*4,A_WeaponReady,&states[S_CHAIN]},	// S_CHAIN
{Spr(rSPR_BIGCHAINGUN,0),1*4,A_Lower,&states[S_CHAINDOWN]},	// S_CHAINDOWN
{Spr(rSPR_BIGCHAINGUN,0),1*4,A_Raise,&states[S_CHAINUP]},	// S_CHAINUP
{Spr(rSPR_BIGCHAINGUN,0),2*4,A_FireCGun,&states[S_CHAIN2]},	// S_CHAIN1
{Spr(rSPR_BIGCHAINGUN,1),2*4,A_FireCGun,&states[S_CHAIN3]},	// S_CHAIN2
{Spr(rSPR_BIGCHAINGUN,1),0*4,A_ReFire,&states[S_CHAIN]},	// S_CHAIN3
{Spr(rSPR_BIGCHAINGUN,2|FF_FULLBRIGHT),3*4,A_Light1,&states[S_LIGHTDONE]},	// S_CHAINFLASH1
{Spr(rSPR_BIGCHAINGUN,3|FF_FULLBRIGHT),2*4,A_Light2,&states[S_LIGHTDONE]},	// S_CHAINFLASH2

{Spr(rSPR_BIGROCKET,0),1*4,A_WeaponReady,&states[S_MISSILE]},	// S_MISSILE
{Spr(rSPR_BIGROCKET,0),1*4,A_Lower,&states[S_MISSILEDOWN]},	// S_MISSILEDOWN
{Spr(rSPR_BIGROCKET,0),1*4,A_Raise,&states[S_MISSILEUP]},	// S_MISSILEUP
{Spr(rSPR_BIGROCKET,1),4*4,A_GunFlash,&states[S_MISSILE2]},	// S_MISSILE1
{Spr(rSPR_BIGROCKET,1),6*4,A_FireMissile,&states[S_MISSILE3]},	// S_MISSILE2
{Spr(rSPR_BIGROCKET,1),0*4,A_ReFire,&states[S_MISSILE]},	// S_MISSILE3
{Spr(rSPR_BIGROCKET,2|FF_FULLBRIGHT),2*4,A_Light1,&states[S_MISSILEFLASH2]},	// S_MISSILEFLASH1
{Spr(rSPR_BIGROCKET,3|FF_FULLBRIGHT),2*4,0,&states[S_MISSILEFLASH3]},	// S_MISSILEFLASH2
{Spr(rSPR_BIGROCKET,4|FF_FULLBRIGHT),2*4,A_Light2,&states[S_MISSILEFLASH4]},	// S_MISSILEFLASH3
{Spr(rSPR_BIGROCKET,5|FF_FULLBRIGHT),2*4,A_Light2,&states[S_LIGHTDONE]},	// S_MISSILEFLASH4

{Spr(rSPR_BIGCHAINSAW,2),2*4,A_WeaponReady,&states[S_SAWB]},	// S_SAW
{Spr(rSPR_BIGCHAINSAW,3),2*4,A_WeaponReady,&states[S_SAW]},	// S_SAWB
{Spr(rSPR_BIGCHAINSAW,2),1*4,A_Lower,&states[S_SAWDOWN]},	// S_SAWDOWN
{Spr(rSPR_BIGCHAINSAW,2),1*4,A_Raise,&states[S_SAWUP]},	// S_SAWUP
{Spr(rSPR_BIGCHAINSAW,0),2*4,A_Saw,&states[S_SAW2]},	// S_SAW1
{Spr(rSPR_BIGCHAINSAW,1),2*4,A_Saw,&states[S_SAW3]},	// S_SAW2
{Spr(rSPR_BIGCHAINSAW,1),0*4,A_ReFire,&states[S_SAW]},	// S_SAW3

{Spr(rSPR_BIGPLASMA,0),1*4,A_WeaponReady,&states[S_PLASMA]},	// S_PLASMA
{Spr(rSPR_BIGPLASMA,0),1*4,A_Lower,&states[S_PLASMADOWN]},	// S_PLASMADOWN
{Spr(rSPR_BIGPLASMA,0),1*4,A_Raise,&states[S_PLASMAUP]},	// S_PLASMAUP
{Spr(rSPR_BIGPLASMA,0),2*4,A_FirePlasma,&states[S_PLASMA2]},	// S_PLASMA1
{Spr(rSPR_BIGPLASMA,1),10*4,A_ReFire,&states[S_PLASMA]},	// S_PLASMA2
{Spr(rSPR_BIGPLASMA,2|FF_FULLBRIGHT),2*4,A_Light1,&states[S_LIGHTDONE]},	// S_PLASMAFLASH1
{Spr(rSPR_BIGPLASMA,3|FF_FULLBRIGHT),2*4,A_Light1,&states[S_LIGHTDONE]},	// S_PLASMAFLASH2

{Spr(rSPR_BIGBFG,0),1*4,A_WeaponReady,&states[S_BFG]},	// S_BFG
{Spr(rSPR_BIGBFG,0),1*4,A_Lower,&states[S_BFGDOWN]},	// S_BFGDOWN
{Spr(rSPR_BIGBFG,0),1*4,A_Raise,&states[S_BFGUP]},	// S_BFGUP
{Spr(rSPR_BIGBFG,0),10*4,A_BFGsound,&states[S_BFG2]},	// S_BFG1
{Spr(rSPR_BIGBFG,1),5*4,A_GunFlash,&states[S_BFG3]},	// S_BFG2
{Spr(rSPR_BIGBFG,1),5*4,A_FireBFG,&states[S_BFG4]},	// S_BFG3
{Spr(rSPR_BIGBFG,1),10*4,A_ReFire,&states[S_BFG]},	// S_BFG4
{Spr(rSPR_BIGBFG,2|FF_FULLBRIGHT),5*4,A_Light1,&states[S_BFGFLASH2]},	// S_BFGFLASH1
{Spr(rSPR_BIGBFG,3|FF_FULLBRIGHT),3*4,A_Light2,&states[S_LIGHTDONE]},	// S_BFGFLASH2

{Spr(rSPR_BLUD,2),4*4,0,&states[S_BLOOD2]},	// S_BLOOD1
{Spr(rSPR_BLUD,1),4*4,0,&states[S_BLOOD3]},	// S_BLOOD2
{Spr(rSPR_BLUD,0),4*4,0,0},	// S_BLOOD3
{Spr(rSPR_PUFF,0|FF_FULLBRIGHT),2*4,0,&states[S_PUFF2]},	// S_PUFF1
{Spr(rSPR_PUFF,1),2*4,0,&states[S_PUFF3]},	// S_PUFF2
{Spr(rSPR_PUFF,2),2*4,0,&states[S_PUFF4]},	// S_PUFF3
{Spr(rSPR_PUFF,3),2*4,0,0},	// S_PUFF4
{Spr(rSPR_IMP,21|FF_FULLBRIGHT),2*4,0,&states[S_TBALL2]},	// S_TBALL1
{Spr(rSPR_IMP,22|FF_FULLBRIGHT),2*4,0,&states[S_TBALL1]},	// S_TBALL2
{Spr(rSPR_IMP,23|FF_FULLBRIGHT),3*4,0,&states[S_TBALLX2]},	// S_TBALLX1
{Spr(rSPR_IMP,24|FF_FULLBRIGHT),3*4,0,&states[S_TBALLX3]},	// S_TBALLX2
{Spr(rSPR_IMP,25|FF_FULLBRIGHT),3*4,0,0},	// S_TBALLX3
{Spr(rSPR_CACOBOLT,0|FF_FULLBRIGHT),2*4,0,&states[S_RBALL2]},	// S_RBALL1
{Spr(rSPR_CACOBOLT,1|FF_FULLBRIGHT),2*4,0,&states[S_RBALL1]},	// S_RBALL2
{Spr(rSPR_CACOBOLT,2|FF_FULLBRIGHT),3*4,0,&states[S_RBALLX2]},	// S_RBALLX1
{Spr(rSPR_CACOBOLT,3|FF_FULLBRIGHT),3*4,0,&states[S_RBALLX3]},	// S_RBALLX2
{Spr(rSPR_CACOBOLT,4|FF_FULLBRIGHT),3*4,0,0},	// S_RBALLX3
{Spr(rSPR_BARONBLT,0|FF_FULLBRIGHT),2*4,0,&states[S_BRBALL2]},	// S_BRBALL1
{Spr(rSPR_BARONBLT,1|FF_FULLBRIGHT),2*4,0,&states[S_BRBALL1]},	// S_BRBALL2
{Spr(rSPR_BARONBLT,2|FF_FULLBRIGHT),3*4,0,&states[S_BRBALLX2]},	// S_BRBALLX1
{Spr(rSPR_BARONBLT,3|FF_FULLBRIGHT),3*4,0,&states[S_BRBALLX3]},	// S_BRBALLX2
{Spr(rSPR_BARONBLT,4|FF_FULLBRIGHT),3*4,0,0},	// S_BRBALLX3
{Spr(rSPR_PLSS,0|FF_FULLBRIGHT),3*4,0,&states[S_PLASBALL2]},	// S_PLASBALL
{Spr(rSPR_PLSS,1|FF_FULLBRIGHT),3*4,0,&states[S_PLASBALL]},	// S_PLASBALL2
{Spr(rSPR_PLSE,0|FF_FULLBRIGHT),2*4,0,&states[S_PLASEXP2]},	// S_PLASEXP
{Spr(rSPR_PLSE,1|FF_FULLBRIGHT),2*4,0,&states[S_PLASEXP3]},	// S_PLASEXP2
{Spr(rSPR_PLSE,2|FF_FULLBRIGHT),2*4,0,&states[S_PLASEXP4]},	// S_PLASEXP3
{Spr(rSPR_PLSE,3|FF_FULLBRIGHT),2*4,0,&states[S_PLASEXP5]},	// S_PLASEXP4
{Spr(rSPR_PLSE,4|FF_FULLBRIGHT),2*4,0,0},	// S_PLASEXP5
{Spr(rSPR_MISL,0|FF_FULLBRIGHT),1*4,0,&states[S_ROCKET]},	// S_ROCKET
{Spr(rSPR_BFS1,0|FF_FULLBRIGHT),2*4,0,&states[S_BFGSHOT2]},	// S_BFGSHOT
{Spr(rSPR_BFS1,1|FF_FULLBRIGHT),2*4,0,&states[S_BFGSHOT]},	// S_BFGSHOT2
{Spr(rSPR_BFE1,0|FF_FULLBRIGHT),4*4,0,&states[S_BFGLAND2]},	// S_BFGLAND
{Spr(rSPR_BFE1,1|FF_FULLBRIGHT),4*4,0,&states[S_BFGLAND3]},	// S_BFGLAND2
{Spr(rSPR_BFE1,2|FF_FULLBRIGHT),4*4,A_BFGSpray,&states[S_BFGLAND4]},	// S_BFGLAND3
{Spr(rSPR_BFE1,3|FF_FULLBRIGHT),4*4,0,&states[S_BFGLAND5]},	// S_BFGLAND4
{Spr(rSPR_BFE1,4|FF_FULLBRIGHT),4*4,0,&states[S_BFGLAND6]},	// S_BFGLAND5
{Spr(rSPR_BFE1,5|FF_FULLBRIGHT),4*4,0,0},	// S_BFGLAND6
{Spr(rSPR_BFE2,0|FF_FULLBRIGHT),4*4,0,&states[S_BFGEXP2]},	// S_BFGEXP
{Spr(rSPR_BFE2,1|FF_FULLBRIGHT),4*4,0,&states[S_BFGEXP3]},	// S_BFGEXP2
{Spr(rSPR_BFE2,2|FF_FULLBRIGHT),4*4,0,&states[S_BFGEXP4]},	// S_BFGEXP3
{Spr(rSPR_BFE2,3|FF_FULLBRIGHT),4*4,0,0},	// S_BFGEXP4
{Spr(rSPR_MISL,1|FF_FULLBRIGHT),4*4,A_Explode,&states[S_EXPLODE2]},	// S_EXPLODE1
{Spr(rSPR_MISL,2|FF_FULLBRIGHT),3*4,0,&states[S_EXPLODE3]},	// S_EXPLODE2
{Spr(rSPR_MISL,3|FF_FULLBRIGHT),2*4,0,0},	// S_EXPLODE3
{Spr(rSPR_TELEFOG,0|FF_FULLBRIGHT),3*4,0,&states[S_TFOG01]},	// S_TFOG
{Spr(rSPR_TELEFOG,1|FF_FULLBRIGHT),3*4,0,&states[S_TFOG02]},	// S_TFOG01
{Spr(rSPR_TELEFOG,0|FF_FULLBRIGHT),3*4,0,&states[S_TFOG2]},	// S_TFOG02
{Spr(rSPR_TELEFOG,1|FF_FULLBRIGHT),3*4,0,&states[S_TFOG3]},	// S_TFOG2
{Spr(rSPR_TELEFOG,2|FF_FULLBRIGHT),3*4,0,&states[S_TFOG4]},	// S_TFOG3
{Spr(rSPR_TELEFOG,3|FF_FULLBRIGHT),3*4,0,&states[S_TFOG5]},	// S_TFOG4
{Spr(rSPR_TELEFOG,4|FF_FULLBRIGHT),3*4,0,&states[S_TFOG6]},	// S_TFOG5
{Spr(rSPR_TELEFOG,5|FF_FULLBRIGHT),3*4,0,&states[S_TFOG7]},	// S_TFOG6
{Spr(rSPR_TELEFOG,6|FF_FULLBRIGHT),3*4,0,&states[S_TFOG8]},	// S_TFOG7
{Spr(rSPR_TELEFOG,7|FF_FULLBRIGHT),3*4,0,&states[S_TFOG9]},	// S_TFOG8
{Spr(rSPR_TELEFOG,8|FF_FULLBRIGHT),3*4,0,&states[S_TFOG10]},	// S_TFOG9
{Spr(rSPR_TELEFOG,9|FF_FULLBRIGHT),3*4,0,0},	// S_TFOG10
{Spr(rSPR_IFOG,0|FF_FULLBRIGHT),3*4,0,&states[S_IFOG01]},	// S_IFOG
{Spr(rSPR_IFOG,1|FF_FULLBRIGHT),3*4,0,&states[S_IFOG02]},	// S_IFOG01
{Spr(rSPR_IFOG,0|FF_FULLBRIGHT),3*4,0,&states[S_IFOG2]},	// S_IFOG02
{Spr(rSPR_IFOG,1|FF_FULLBRIGHT),3*4,0,&states[S_IFOG3]},	// S_IFOG2
{Spr(rSPR_IFOG,2|FF_FULLBRIGHT),3*4,0,&states[S_IFOG4]},	// S_IFOG3
{Spr(rSPR_IFOG,3|FF_FULLBRIGHT),3*4,0,&states[S_IFOG5]},	// S_IFOG4
{Spr(rSPR_IFOG,4|FF_FULLBRIGHT),3*4,0,0},	// S_IFOG5
{Spr(rSPR_OURHERO,0),-1,0,0},	// S_PLAY
{Spr(rSPR_OURHERO,0),2*4,0,&states[S_PLAY_RUN2]},	// S_PLAY_RUN1
{Spr(rSPR_OURHERO,1),2*4,0,&states[S_PLAY_RUN3]},	// S_PLAY_RUN2
{Spr(rSPR_OURHERO,2),2*4,0,&states[S_PLAY_RUN4]},	// S_PLAY_RUN3
{Spr(rSPR_OURHERO,3),2*4,0,&states[S_PLAY_RUN1]},	// S_PLAY_RUN4
{Spr(rSPR_OURHERO,4),2*4,0,&states[S_PLAY_ATK2]},	// S_PLAY_ATK1
{Spr(rSPR_OURHERO,5|FF_FULLBRIGHT),4*4,0,&states[S_PLAY]},	// S_PLAY_ATK2
{Spr(rSPR_OURHERO,6),2*4,0,&states[S_PLAY_PAIN2]},	// S_PLAY_PAIN
{Spr(rSPR_OURHERO,6),2*4,A_Pain,&states[S_PLAY]},	// S_PLAY_PAIN2
{Spr(rSPR_OURHERO,7),5*4,0,&states[S_PLAY_DIE2]},	// S_PLAY_DIE1
{Spr(rSPR_OURHERO,8),5*4,A_Scream,&states[S_PLAY_DIE3]},	// S_PLAY_DIE2
{Spr(rSPR_OURHERO,9),5*4,A_Fall,&states[S_PLAY_DIE4]},	// S_PLAY_DIE3
{Spr(rSPR_OURHERO,10),5*4,0,&states[S_PLAY_DIE5]},	// S_PLAY_DIE4
{Spr(rSPR_OURHERO,11),5*4,0,&states[S_PLAY_DIE6]},	// S_PLAY_DIE5
{Spr(rSPR_OURHERO,12),5*4,0,&states[S_PLAY_DIE7]},	// S_PLAY_DIE6
{Spr(rSPR_OURHEROBDY,0),-1,0,0},	// S_PLAY_DIE7
{Spr(rSPR_OURHERO,13),2*4,0,&states[S_PLAY_XDIE2]},	// S_PLAY_XDIE1
{Spr(rSPR_OURHERO,14),2*4,A_XScream,&states[S_PLAY_XDIE3]},	// S_PLAY_XDIE2
{Spr(rSPR_OURHERO,15),2*4,A_Fall,&states[S_PLAY_XDIE4]},	// S_PLAY_XDIE3
{Spr(rSPR_OURHERO,16),2*4,0,&states[S_PLAY_XDIE5]},	// S_PLAY_XDIE4
{Spr(rSPR_OURHERO,17),2*4,0,&states[S_PLAY_XDIE6]},	// S_PLAY_XDIE5
{Spr(rSPR_OURHERO,18),2*4,0,&states[S_PLAY_XDIE7]},	// S_PLAY_XDIE6
{Spr(rSPR_OURHERO,19),2*4,0,&states[S_PLAY_XDIE8]},	// S_PLAY_XDIE7
{Spr(rSPR_OURHERO,20),2*4,0,&states[S_PLAY_XDIE9]},	// S_PLAY_XDIE8
{Spr(rSPR_OURHEROBDY,1),-1,0,0},	// S_PLAY_XDIE9
{Spr(rSPR_ZOMBIE,0),5*4,A_Look,&states[S_POSS_STND2]},	// S_POSS_STND
{Spr(rSPR_ZOMBIE,1),5*4,A_Look,&states[S_POSS_STND]},	// S_POSS_STND2
{Spr(rSPR_ZOMBIE,0),2*4,A_Chase,&states[S_POSS_RUN2]},	// S_POSS_RUN1
{Spr(rSPR_ZOMBIE,0),2*4,A_Chase,&states[S_POSS_RUN3]},	// S_POSS_RUN2
{Spr(rSPR_ZOMBIE,1),2*4,A_Chase,&states[S_POSS_RUN4]},	// S_POSS_RUN3
{Spr(rSPR_ZOMBIE,1),2*4,A_Chase,&states[S_POSS_RUN5]},	// S_POSS_RUN4
{Spr(rSPR_ZOMBIE,2),2*4,A_Chase,&states[S_POSS_RUN6]},	// S_POSS_RUN5
{Spr(rSPR_ZOMBIE,2),2*4,A_Chase,&states[S_POSS_RUN7]},	// S_POSS_RUN6
{Spr(rSPR_ZOMBIE,3),2*4,A_Chase,&states[S_POSS_RUN8]},	// S_POSS_RUN7
{Spr(rSPR_ZOMBIE,3),2*4,A_Chase,&states[S_POSS_RUN1]},	// S_POSS_RUN8
{Spr(rSPR_ZOMBIE,4),5*4,A_FaceTarget,&states[S_POSS_ATK2]},	// S_POSS_ATK1
{Spr(rSPR_ZOMBIE,5),4*4,A_PosAttack,&states[S_POSS_ATK3]},	// S_POSS_ATK2
{Spr(rSPR_ZOMBIE,4),4*4,0,&states[S_POSS_RUN1]},	// S_POSS_ATK3
{Spr(rSPR_ZOMBIE,6),1*4,0,&states[S_POSS_PAIN2]},	// S_POSS_PAIN
{Spr(rSPR_ZOMBIE,6),2*4,A_Pain,&states[S_POSS_RUN1]},	// S_POSS_PAIN2
{Spr(rSPR_ZOMBIE,7),2*4,0,&states[S_POSS_DIE2]},	// S_POSS_DIE1
{Spr(rSPR_ZOMBIE,8),3*4,A_Scream,&states[S_POSS_DIE3]},	// S_POSS_DIE2
{Spr(rSPR_ZOMBIE,9),2*4,A_Fall,&states[S_POSS_DIE4]},	// S_POSS_DIE3
{Spr(rSPR_ZOMBIE,10),3*4,0,&states[S_POSS_DIE5]},	// S_POSS_DIE4
{Spr(rSPR_ZOMBIEBODY,0),-1,0,0},	// S_POSS_DIE5
{Spr(rSPR_ZOMBIE,11),2*4,0,&states[S_POSS_XDIE2]},	// S_POSS_XDIE1
{Spr(rSPR_ZOMBIE,12),3*4,A_XScream,&states[S_POSS_XDIE3]},	// S_POSS_XDIE2
{Spr(rSPR_ZOMBIE,13),2*4,A_Fall,&states[S_POSS_XDIE4]},	// S_POSS_XDIE3
{Spr(rSPR_ZOMBIE,14),3*4,0,&states[S_POSS_XDIE5]},	// S_POSS_XDIE4
{Spr(rSPR_ZOMBIE,15),2*4,0,&states[S_POSS_XDIE6]},	// S_POSS_XDIE5
{Spr(rSPR_ZOMBIE,16),3*4,0,&states[S_POSS_XDIE7]},	// S_POSS_XDIE6
{Spr(rSPR_ZOMBIE,17),2*4,0,&states[S_POSS_XDIE8]},	// S_POSS_XDIE7
{Spr(rSPR_ZOMBIE,18),3*4,0,&states[S_POSS_XDIE9]},	// S_POSS_XDIE8
{Spr(rSPR_ZOMBIEBODY,1),-1,0,0},	// S_POSS_XDIE9
{Spr(rSPR_SHOTGUY,0),5*4,A_Look,&states[S_SPOS_STND2]},	// S_SPOS_STND
{Spr(rSPR_SHOTGUY,1),5*4,A_Look,&states[S_SPOS_STND]},	// S_SPOS_STND2
{Spr(rSPR_SHOTGUY,0),1*4,A_Chase,&states[S_SPOS_RUN2]},	// S_SPOS_RUN1
{Spr(rSPR_SHOTGUY,0),2*4,A_Chase,&states[S_SPOS_RUN3]},	// S_SPOS_RUN2
{Spr(rSPR_SHOTGUY,1),1*4,A_Chase,&states[S_SPOS_RUN4]},	// S_SPOS_RUN3
{Spr(rSPR_SHOTGUY,1),2*4,A_Chase,&states[S_SPOS_RUN5]},	// S_SPOS_RUN4
{Spr(rSPR_SHOTGUY,2),1*4,A_Chase,&states[S_SPOS_RUN6]},	// S_SPOS_RUN5
{Spr(rSPR_SHOTGUY,2),2*4,A_Chase,&states[S_SPOS_RUN7]},	// S_SPOS_RUN6
{Spr(rSPR_SHOTGUY,3),1*4,A_Chase,&states[S_SPOS_RUN8]},	// S_SPOS_RUN7
{Spr(rSPR_SHOTGUY,3),2*4,A_Chase,&states[S_SPOS_RUN1]},	// S_SPOS_RUN8
{Spr(rSPR_SHOTGUY,4),5*4,A_FaceTarget,&states[S_SPOS_ATK2]},	// S_SPOS_ATK1
{Spr(rSPR_SHOTGUY,5|FF_FULLBRIGHT),5*4,A_SPosAttack,&states[S_SPOS_ATK3]},	// S_SPOS_ATK2
{Spr(rSPR_SHOTGUY,4),5*4,0,&states[S_SPOS_RUN1]},	// S_SPOS_ATK3
{Spr(rSPR_SHOTGUY,6),1*4,0,&states[S_SPOS_PAIN2]},	// S_SPOS_PAIN
{Spr(rSPR_SHOTGUY,6),2*4,A_Pain,&states[S_SPOS_RUN1]},	// S_SPOS_PAIN2
{Spr(rSPR_SHOTGUY,7),2*4,0,&states[S_SPOS_DIE2]},	// S_SPOS_DIE1
{Spr(rSPR_SHOTGUY,8),3*4,A_Scream,&states[S_SPOS_DIE3]},	// S_SPOS_DIE2
{Spr(rSPR_SHOTGUY,9),2*4,A_Fall,&states[S_SPOS_DIE4]},	// S_SPOS_DIE3
{Spr(rSPR_SHOTGUY,10),3*4,0,&states[S_SPOS_DIE5]},	// S_SPOS_DIE4
{Spr(rSPR_SHOTGUY,11),-1,0,0},	// S_SPOS_DIE5
{Spr(rSPR_SHOTGUY,12),2*4,0,&states[S_SPOS_XDIE2]},	// S_SPOS_XDIE1
{Spr(rSPR_SHOTGUY,13),3*4,A_XScream,&states[S_SPOS_XDIE3]},	// S_SPOS_XDIE2
{Spr(rSPR_SHOTGUY,14),2*4,A_Fall,&states[S_SPOS_XDIE4]},	// S_SPOS_XDIE3
{Spr(rSPR_SHOTGUY,15),3*4,0,&states[S_SPOS_XDIE5]},	// S_SPOS_XDIE4
{Spr(rSPR_SHOTGUY,16),2*4,0,&states[S_SPOS_XDIE6]},	// S_SPOS_XDIE5
{Spr(rSPR_SHOTGUY,17),3*4,0,&states[S_SPOS_XDIE7]},	// S_SPOS_XDIE6
{Spr(rSPR_SHOTGUY,18),2*4,0,&states[S_SPOS_XDIE8]},	// S_SPOS_XDIE7
{Spr(rSPR_SHOTGUY,19),3*4,0,&states[S_SPOS_XDIE9]},	// S_SPOS_XDIE8
{Spr(rSPR_SHOTGUY,20),-1,0,0},	// S_SPOS_XDIE9
{Spr(rSPR_IMP,0),5*4,A_Look,&states[S_TROO_STND2]},	// S_TROO_STND
{Spr(rSPR_IMP,1),5*4,A_Look,&states[S_TROO_STND]},	// S_TROO_STND2
{Spr(rSPR_IMP,0),1*4,A_Chase,&states[S_TROO_RUN2]},	// S_TROO_RUN1
{Spr(rSPR_IMP,0),2*4,A_Chase,&states[S_TROO_RUN3]},	// S_TROO_RUN2
{Spr(rSPR_IMP,1),1*4,A_Chase,&states[S_TROO_RUN4]},	// S_TROO_RUN3
{Spr(rSPR_IMP,1),2*4,A_Chase,&states[S_TROO_RUN5]},	// S_TROO_RUN4
{Spr(rSPR_IMP,2),1*4,A_Chase,&states[S_TROO_RUN6]},	// S_TROO_RUN5
{Spr(rSPR_IMP,2),2*4,A_Chase,&states[S_TROO_RUN7]},	// S_TROO_RUN6
{Spr(rSPR_IMP,3),1*4,A_Chase,&states[S_TROO_RUN8]},	// S_TROO_RUN7
{Spr(rSPR_IMP,3),2*4,A_Chase,&states[S_TROO_RUN1]},	// S_TROO_RUN8
{Spr(rSPR_IMP,4),4*4,A_FaceTarget,&states[S_TROO_ATK2]},	// S_TROO_ATK1
{Spr(rSPR_IMP,5),4*4,A_FaceTarget,&states[S_TROO_ATK3]},	// S_TROO_ATK2
{Spr(rSPR_IMP,6),3*4,A_TroopAttack,&states[S_TROO_RUN1]},	// S_TROO_ATK3
{Spr(rSPR_IMP,7),1*4,0,&states[S_TROO_PAIN2]},	// S_TROO_PAIN
{Spr(rSPR_IMP,7),1*4,A_Pain,&states[S_TROO_RUN1]},	// S_TROO_PAIN2
{Spr(rSPR_IMP,8),4*4,0,&states[S_TROO_DIE2]},	// S_TROO_DIE1
{Spr(rSPR_IMP,9),4*4,A_Scream,&states[S_TROO_DIE3]},	// S_TROO_DIE2
{Spr(rSPR_IMP,10),3*4,0,&states[S_TROO_DIE4]},	// S_TROO_DIE3
{Spr(rSPR_IMP,11),3*4,A_Fall,&states[S_TROO_DIE5]},	// S_TROO_DIE4
{Spr(rSPR_IMP,12),-1,0,0},	// S_TROO_DIE5
{Spr(rSPR_IMP,13),2*4,0,&states[S_TROO_XDIE2]},	// S_TROO_XDIE1
{Spr(rSPR_IMP,14),3*4,A_XScream,&states[S_TROO_XDIE3]},	// S_TROO_XDIE2
{Spr(rSPR_IMP,15),2*4,0,&states[S_TROO_XDIE4]},	// S_TROO_XDIE3
{Spr(rSPR_IMP,16),3*4,A_Fall,&states[S_TROO_XDIE5]},	// S_TROO_XDIE4
{Spr(rSPR_IMP,17),2*4,0,&states[S_TROO_XDIE6]},	// S_TROO_XDIE5
{Spr(rSPR_IMP,18),3*4,0,&states[S_TROO_XDIE7]},	// S_TROO_XDIE6
{Spr(rSPR_IMP,19),2*4,0,&states[S_TROO_XDIE8]},	// S_TROO_XDIE7
{Spr(rSPR_IMP,20),-1,0,0},	// S_TROO_XDIE8
{Spr(rSPR_DEMON,0),5*4,A_Look,&states[S_SARG_STND2]},	// S_SARG_STND
{Spr(rSPR_DEMON,1),5*4,A_Look,&states[S_SARG_STND]},	// S_SARG_STND2
{Spr(rSPR_DEMON,0),1*4,A_Chase,&states[S_SARG_RUN2]},	// S_SARG_RUN1
{Spr(rSPR_DEMON,0),1*4,A_Chase,&states[S_SARG_RUN3]},	// S_SARG_RUN2
{Spr(rSPR_DEMON,1),1*4,A_Chase,&states[S_SARG_RUN4]},	// S_SARG_RUN3
{Spr(rSPR_DEMON,1),1*4,A_Chase,&states[S_SARG_RUN5]},	// S_SARG_RUN4
{Spr(rSPR_DEMON,2),1*4,A_Chase,&states[S_SARG_RUN6]},	// S_SARG_RUN5
{Spr(rSPR_DEMON,2),1*4,A_Chase,&states[S_SARG_RUN7]},	// S_SARG_RUN6
{Spr(rSPR_DEMON,3),1*4,A_Chase,&states[S_SARG_RUN8]},	// S_SARG_RUN7
{Spr(rSPR_DEMON,3),1*4,A_Chase,&states[S_SARG_RUN1]},	// S_SARG_RUN8
{Spr(rSPR_DEMON,4),4*4,A_FaceTarget,&states[S_SARG_ATK2]},	// S_SARG_ATK1
{Spr(rSPR_DEMON,5),4*4,A_FaceTarget,&states[S_SARG_ATK3]},	// S_SARG_ATK2
{Spr(rSPR_DEMON,6),4*4,A_SargAttack,&states[S_SARG_RUN1]},	// S_SARG_ATK3
{Spr(rSPR_DEMON,7),1*4,0,&states[S_SARG_PAIN2]},	// S_SARG_PAIN
{Spr(rSPR_DEMON,7),1*4,A_Pain,&states[S_SARG_RUN1]},	// S_SARG_PAIN2
{Spr(rSPR_DEMON,8),4*4,0,&states[S_SARG_DIE2]},	// S_SARG_DIE1
{Spr(rSPR_DEMON,9),4*4,A_Scream,&states[S_SARG_DIE3]},	// S_SARG_DIE2
{Spr(rSPR_DEMON,10),2*4,0,&states[S_SARG_DIE4]},	// S_SARG_DIE3
{Spr(rSPR_DEMON,11),2*4,A_Fall,&states[S_SARG_DIE5]},	// S_SARG_DIE4
{Spr(rSPR_DEMON,12),2*4,0,&states[S_SARG_DIE6]},	// S_SARG_DIE5
{Spr(rSPR_DEMON,13),-1,0,0},	// S_SARG_DIE6
{Spr(rSPR_CACODEMON,0),5*4,A_Look,&states[S_HEAD_STND]},	// S_HEAD_STND
{Spr(rSPR_CACODEMON,0),1*4,A_Chase,&states[S_HEAD_RUN1]},	// S_HEAD_RUN1
{Spr(rSPR_CACODEMON,1),2*4,A_FaceTarget,&states[S_HEAD_ATK2]},	// S_HEAD_ATK1
{Spr(rSPR_CACODEMON,2),2*4,A_FaceTarget,&states[S_HEAD_ATK3]},	// S_HEAD_ATK2
{Spr(rSPR_CACODEMON,3|FF_FULLBRIGHT),3*4,A_HeadAttack,&states[S_HEAD_RUN1]},	// S_HEAD_ATK3
{Spr(rSPR_CACODEMON,4),1*4,0,&states[S_HEAD_PAIN2]},	// S_HEAD_PAIN
{Spr(rSPR_CACODEMON,4),2*4,A_Pain,&states[S_HEAD_PAIN3]},	// S_HEAD_PAIN2
{Spr(rSPR_CACODEMON,5),3*4,0,&states[S_HEAD_RUN1]},	// S_HEAD_PAIN3
{Spr(rSPR_CACODIE,0),4*4,0,&states[S_HEAD_DIE2]},	// S_HEAD_DIE1
{Spr(rSPR_CACODIE,1),4*4,A_Scream,&states[S_HEAD_DIE3]},	// S_HEAD_DIE2
{Spr(rSPR_CACODIE,2),4*4,0,&states[S_HEAD_DIE4]},	// S_HEAD_DIE3
{Spr(rSPR_CACODIE,3),4*4,0,&states[S_HEAD_DIE5]},	// S_HEAD_DIE4
{Spr(rSPR_CACODIE,4),4*4,A_Fall,&states[S_HEAD_DIE6]},	// S_HEAD_DIE5
{Spr(rSPR_CACOBDY,0),-1,0,0},	// S_HEAD_DIE6
{Spr(rSPR_BARON,0),5*4,A_Look,&states[S_BOSS_STND2]},	// S_BOSS_STND
{Spr(rSPR_BARON,1),5*4,A_Look,&states[S_BOSS_STND]},	// S_BOSS_STND2
{Spr(rSPR_BARON,0),1*4,A_Chase,&states[S_BOSS_RUN2]},	// S_BOSS_RUN1
{Spr(rSPR_BARON,0),2*4,A_Chase,&states[S_BOSS_RUN3]},	// S_BOSS_RUN2
{Spr(rSPR_BARON,1),1*4,A_Chase,&states[S_BOSS_RUN4]},	// S_BOSS_RUN3
{Spr(rSPR_BARON,1),2*4,A_Chase,&states[S_BOSS_RUN5]},	// S_BOSS_RUN4
{Spr(rSPR_BARON,2),1*4,A_Chase,&states[S_BOSS_RUN6]},	// S_BOSS_RUN5
{Spr(rSPR_BARON,2),2*4,A_Chase,&states[S_BOSS_RUN7]},	// S_BOSS_RUN6
{Spr(rSPR_BARON,3),1*4,A_Chase,&states[S_BOSS_RUN8]},	// S_BOSS_RUN7
{Spr(rSPR_BARON,3),2*4,A_Chase,&states[S_BOSS_RUN1]},	// S_BOSS_RUN8
{Spr(rSPR_BARONATK,0),4*4,A_FaceTarget,&states[S_BOSS_ATK2]},	// S_BOSS_ATK1
{Spr(rSPR_BARONATK,1),4*4,A_FaceTarget,&states[S_BOSS_ATK3]},	// S_BOSS_ATK2
{Spr(rSPR_BARONATK,2),4*4,A_BruisAttack,&states[S_BOSS_RUN1]},	// S_BOSS_ATK3
{Spr(rSPR_BARONATK,3),1*4,0,&states[S_BOSS_PAIN2]},	// S_BOSS_PAIN
{Spr(rSPR_BARONATK,3),1*4,A_Pain,&states[S_BOSS_RUN1]},	// S_BOSS_PAIN2
{Spr(rSPR_BARONDIE,0),4*4,0,&states[S_BOSS_DIE2]},	// S_BOSS_DIE1
{Spr(rSPR_BARONDIE,1),4*4,A_Scream,&states[S_BOSS_DIE3]},	// S_BOSS_DIE2
{Spr(rSPR_BARONDIE,2),4*4,0,&states[S_BOSS_DIE4]},	// S_BOSS_DIE3
{Spr(rSPR_BARONDIE,3),4*4,A_Fall,&states[S_BOSS_DIE5]},	// S_BOSS_DIE4
{Spr(rSPR_BARONDIE,4),4*4,0,&states[S_BOSS_DIE6]},	// S_BOSS_DIE5
{Spr(rSPR_BARONDIE,5),4*4,0,&states[S_BOSS_DIE7]},	// S_BOSS_DIE6
{Spr(rSPR_BARONBDY,0),-1,A_BossDeath,0},	// S_BOSS_DIE7
{Spr(rSPR_LOSTSOUL,0|FF_FULLBRIGHT),5*4,A_Look,&states[S_SKULL_STND2]},	// S_SKULL_STND
{Spr(rSPR_LOSTSOUL,1|FF_FULLBRIGHT),5*4,A_Look,&states[S_SKULL_STND]},	// S_SKULL_STND2
{Spr(rSPR_LOSTSOUL,0|FF_FULLBRIGHT),3*4,A_Chase,&states[S_SKULL_RUN2]},	// S_SKULL_RUN1
{Spr(rSPR_LOSTSOUL,1|FF_FULLBRIGHT),3*4,A_Chase,&states[S_SKULL_RUN1]},	// S_SKULL_RUN2
{Spr(rSPR_LOSTSOUL,2|FF_FULLBRIGHT),5*4,A_FaceTarget,&states[S_SKULL_ATK2]},	// S_SKULL_ATK1
{Spr(rSPR_LOSTSOUL,3|FF_FULLBRIGHT),2*4,A_SkullAttack,&states[S_SKULL_ATK3]},	// S_SKULL_ATK2
{Spr(rSPR_LOSTSOUL,2|FF_FULLBRIGHT),2*4,0,&states[S_SKULL_ATK4]},	// S_SKULL_ATK3
{Spr(rSPR_LOSTSOUL,3|FF_FULLBRIGHT),2*4,0,&states[S_SKULL_ATK3]},	// S_SKULL_ATK4
{Spr(rSPR_LOSTSOUL,4|FF_FULLBRIGHT),1*4,0,&states[S_SKULL_PAIN2]},	// S_SKULL_PAIN
{Spr(rSPR_LOSTSOUL,4|FF_FULLBRIGHT),2*4,A_Pain,&states[S_SKULL_RUN1]},	// S_SKULL_PAIN2
{Spr(rSPR_LOSTSOUL,5|FF_FULLBRIGHT),3*4,0,&states[S_SKULL_DIE2]},	// S_SKULL_DIE1
{Spr(rSPR_LOSTSOUL,6|FF_FULLBRIGHT),3*4,A_Scream,&states[S_SKULL_DIE3]},	// S_SKULL_DIE2
{Spr(rSPR_LOSTSOUL,7|FF_FULLBRIGHT),3*4,0,&states[S_SKULL_DIE4]},	// S_SKULL_DIE3
{Spr(rSPR_LOSTSOUL,8|FF_FULLBRIGHT),3*4,A_Fall,&states[S_SKULL_DIE5]},	// S_SKULL_DIE4
{Spr(rSPR_LOSTSOUL,9),3*4,0,&states[S_SKULL_DIE6]},	// S_SKULL_DIE5
{Spr(rSPR_LOSTSOUL,10),3*4,0,0},	// S_SKULL_DIE6
{Spr(rSPR_GREENARMOR,0),3*4,0,&states[S_ARM1A]},	// S_ARM1
{Spr(rSPR_GREENARMOR,1|FF_FULLBRIGHT),3*4,0,&states[S_ARM1]},	// S_ARM1A
{Spr(rSPR_BLUEARMOR,0),3*4,0,&states[S_ARM2A]},	// S_ARM2
{Spr(rSPR_BLUEARMOR,1|FF_FULLBRIGHT),3*4,0,&states[S_ARM2]},	// S_ARM2A
{Spr(rSPR_BARREL,0),3*4,0,&states[S_BAR2]},	// S_BAR1
{Spr(rSPR_BARREL,1),3*4,0,&states[S_BAR1]},	// S_BAR2
{Spr(rSPR_BARREL,2|FF_FULLBRIGHT),2*4,0,&states[S_BEXP2]},	// S_BEXP
{Spr(rSPR_BARREL,3|FF_FULLBRIGHT),3*4,A_Scream,&states[S_BEXP3]},	// S_BEXP2
{Spr(rSPR_BARREL,4|FF_FULLBRIGHT),3*4,0,&states[S_BEXP4]},	// S_BEXP3
{Spr(rSPR_BARREL,5|FF_FULLBRIGHT),5*4,A_Explode,&states[S_BEXP5]},	// S_BEXP4
{Spr(rSPR_BARREL,6|FF_FULLBRIGHT),5*4,0,0},	// S_BEXP5
{Spr(rSPR_FIRECAN,0|FF_FULLBRIGHT),2*4,0,&states[S_BBAR2]},	// S_BBAR1
{Spr(rSPR_FIRECAN,1|FF_FULLBRIGHT),2*4,0,&states[S_BBAR3]},	// S_BBAR2
{Spr(rSPR_FIRECAN,2|FF_FULLBRIGHT),2*4,0,&states[S_BBAR1]},	// S_BBAR3
{Spr(rSPR_HEALTHBONUS,0),3*4,0,&states[S_BON1A]},	// S_BON1
{Spr(rSPR_HEALTHBONUS,1),3*4,0,&states[S_BON1B]},	// S_BON1A
{Spr(rSPR_HEALTHBONUS,2),3*4,0,&states[S_BON1C]},	// S_BON1B
{Spr(rSPR_HEALTHBONUS,3),3*4,0,&states[S_BON1D]},	// S_BON1C
{Spr(rSPR_HEALTHBONUS,2),3*4,0,&states[S_BON1E]},	// S_BON1D
{Spr(rSPR_HEALTHBONUS,1),3*4,0,&states[S_BON1]},	// S_BON1E
{Spr(rSPR_ARMORBONUS,0),3*4,0,&states[S_BON2A]},	// S_BON2
{Spr(rSPR_ARMORBONUS,1),3*4,0,&states[S_BON2B]},	// S_BON2A
{Spr(rSPR_ARMORBONUS,2),3*4,0,&states[S_BON2C]},	// S_BON2B
{Spr(rSPR_ARMORBONUS,3),3*4,0,&states[S_BON2D]},	// S_BON2C
{Spr(rSPR_ARMORBONUS,2),3*4,0,&states[S_BON2E]},	// S_BON2D
{Spr(rSPR_ARMORBONUS,1),3*4,0,&states[S_BON2]},	// S_BON2E
{Spr(rSPR_BLUEKEYCARD,0),5*4,0,&states[S_BKEY2]},	// S_BKEY
{Spr(rSPR_BLUEKEYCARD,1|FF_FULLBRIGHT),5*4,0,&states[S_BKEY]},	// S_BKEY2
{Spr(rSPR_REDKEYCARD,0),5*4,0,&states[S_RKEY2]},	// S_RKEY
{Spr(rSPR_REDKEYCARD,1|FF_FULLBRIGHT),5*4,0,&states[S_RKEY]},	// S_RKEY2
{Spr(rSPR_YELLOWKEYCARD,0),5*4,0,&states[S_YKEY2]},	// S_YKEY
{Spr(rSPR_YELLOWKEYCARD,1|FF_FULLBRIGHT),5*4,0,&states[S_YKEY]},	// S_YKEY2
{Spr(rSPR_BLUESKULLKEY,0),5*4,0,&states[S_BSKULL2]},	// S_BSKULL
{Spr(rSPR_BLUESKULLKEY,1|FF_FULLBRIGHT),5*4,0,&states[S_BSKULL]},	// S_BSKULL2
{Spr(rSPR_REDSKULLKEY,0),5*4,0,&states[S_RSKULL2]},	// S_RSKULL
{Spr(rSPR_REDSKULLKEY,1|FF_FULLBRIGHT),5*4,0,&states[S_RSKULL]},	// S_RSKULL2
{Spr(rSPR_YELLOWSKULLKEY,0),5*4,0,&states[S_YSKULL2]},	// S_YSKULL
{Spr(rSPR_YELLOWSKULLKEY,1|FF_FULLBRIGHT),5*4,0,&states[S_YSKULL]},	// S_YSKULL2
{Spr(rSPR_STIMPACK,0),-1,0,0},	// S_STIM
{Spr(rSPR_MEDIKIT,0),-1,0,0},	// S_MEDI
{Spr(rSPR_SOULSPHERE,0|FF_FULLBRIGHT),3*4,0,&states[S_SOUL2]},	// S_SOUL
{Spr(rSPR_SOULSPHERE,1|FF_FULLBRIGHT),3*4,0,&states[S_SOUL3]},	// S_SOUL2
{Spr(rSPR_SOULSPHERE,2|FF_FULLBRIGHT),3*4,0,&states[S_SOUL4]},	// S_SOUL3
{Spr(rSPR_SOULSPHERE,3|FF_FULLBRIGHT),3*4,0,&states[S_SOUL5]},	// S_SOUL4
{Spr(rSPR_SOULSPHERE,2|FF_FULLBRIGHT),3*4,0,&states[S_SOUL6]},	// S_SOUL5
{Spr(rSPR_SOULSPHERE,1|FF_FULLBRIGHT),3*4,0,&states[S_SOUL]},	// S_SOUL6
{Spr(rSPR_INVULNERABILITY,0|FF_FULLBRIGHT),3*4,0,&states[S_PINV2]},	// S_PINV
{Spr(rSPR_INVULNERABILITY,1|FF_FULLBRIGHT),3*4,0,&states[S_PINV3]},	// S_PINV2
{Spr(rSPR_INVULNERABILITY,2|FF_FULLBRIGHT),3*4,0,&states[S_PINV4]},	// S_PINV3
{Spr(rSPR_INVULNERABILITY,3|FF_FULLBRIGHT),3*4,0,&states[S_PINV]},	// S_PINV4
{Spr(rSPR_BERZERKER,0|FF_FULLBRIGHT),-1,0,0},	// S_PSTR
{Spr(rSPR_INVISIBILITY,0|FF_FULLBRIGHT),3*4,0,&states[S_PINS2]},	// S_PINS
{Spr(rSPR_INVISIBILITY,1|FF_FULLBRIGHT),3*4,0,&states[S_PINS3]},	// S_PINS2
{Spr(rSPR_INVISIBILITY,2|FF_FULLBRIGHT),3*4,0,&states[S_PINS4]},	// S_PINS3
{Spr(rSPR_INVISIBILITY,3|FF_FULLBRIGHT),3*4,0,&states[S_PINS]},	// S_PINS4
{Spr(rSPR_RADIATIONSUIT,0|FF_FULLBRIGHT),-1,0,0},	// S_SUIT
{Spr(rSPR_COMPUTERMAP,0|FF_FULLBRIGHT),3*4,0,&states[S_PMAP2]},	// S_PMAP
{Spr(rSPR_COMPUTERMAP,1|FF_FULLBRIGHT),3*4,0,&states[S_PMAP3]},	// S_PMAP2
{Spr(rSPR_COMPUTERMAP,2|FF_FULLBRIGHT),3*4,0,&states[S_PMAP4]},	// S_PMAP3
{Spr(rSPR_COMPUTERMAP,3|FF_FULLBRIGHT),3*4,0,&states[S_PMAP5]},	// S_PMAP4
{Spr(rSPR_COMPUTERMAP,2|FF_FULLBRIGHT),3*4,0,&states[S_PMAP6]},	// S_PMAP5
{Spr(rSPR_COMPUTERMAP,1|FF_FULLBRIGHT),3*4,0,&states[S_PMAP]},	// S_PMAP6
{Spr(rSPR_IRGOGGLES,0|FF_FULLBRIGHT),3*4,0,&states[S_PVIS2]},	// S_PVIS
{Spr(rSPR_IRGOGGLES,1),3*4,0,&states[S_PVIS]},	// S_PVIS2
{Spr(rSPR_CLIP,0),-1,0,0},	// S_CLIP
{Spr(rSPR_BOXAMMO,0),-1,0,0},	// S_AMMO
{Spr(rSPR_ROCKET,0),-1,0,0},	// S_ROCK
{Spr(rSPR_BOXROCKETS,0),-1,0,0},	// S_BROK
{Spr(rSPR_CELL,0),-1,0,0},	// S_CELL
{Spr(rSPR_CELLPACK,0),-1,0,0},	// S_CELP
{Spr(rSPR_SHELLS,0),-1,0,0},	// S_SHEL
{Spr(rSPR_BOXSHELLS,0),-1,0,0},	// S_SBOX
{Spr(rSPR_BACKPACK,0),-1,0,0},	// S_BPAK
{Spr(rSPR_BFG9000,0),-1,0,0},	// S_BFUG
{Spr(rSPR_CHAINGUN,0),-1,0,0},	// S_MGUN
{Spr(rSPR_CHAINSAW,0),-1,0,0},	// S_CSAW
{Spr(rSPR_ROCKETLAUNCHER,0),-1,0,0},	// S_LAUN
{Spr(rSPR_PLASMARIFLE,0),-1,0,0},	// S_PLAS
{Spr(rSPR_SHOTGUN,0),-1,0,0},	// S_SHOT
{Spr(rSPR_LIGHTCOLUMN,0|FF_FULLBRIGHT),-1,0,0},	// S_COLU
{Spr(rSPR_SMALLSTAL,0),-1,0,0},	// S_STALAG
{Spr(rSPR_OURHERO,13),-1,0,0},	// S_DEADTORSO
{Spr(rSPR_OURHERO,18),-1,0,0},	// S_DEADBOTTOM
{Spr(rSPR_FIVESKULLPOLE,0),-1,0,0},	// S_HEADSONSTICK
{Spr(rSPR_POOLBLOOD,0),-1,0,0},	// S_GIBS
{Spr(rSPR_TALLSKULLPOLE,0),-1,0,0},	// S_HEADONASTICK
{Spr(rSPR_BODYPOLE,0),-1,0,0},	// S_DEADSTICK
{Spr(rSPR_HANGINGRUMP,0),-1,0,0},	// S_MEAT5
{Spr(rSPR_LARGESTAL,0),-1,0,0},	// S_STALAGTITE
{Spr(rSPR_SHORTGREENPILLAR,0),-1,0,0},	// S_SHRTGRNCOL
{Spr(rSPR_SHORTREDPILLAR,0),-1,0,0},	// S_SHRTREDCOL
{Spr(rSPR_CANDLE,0|FF_FULLBRIGHT),-1,0,0},	// S_CANDLESTIK
{Spr(rSPR_CANDLEABRA,0|FF_FULLBRIGHT),-1,0,0},	// S_CANDELABRA
{Spr(rSPR_MEDDEADTREE,0),-1,0,0},	// S_TORCHTREE
{Spr(rSPR_TECHPILLAR,0),-1,0,0},	// S_TECHPILLAR
{Spr(rSPR_FLAMINGSKULLS,0|FF_FULLBRIGHT),3*4,0,&states[S_FLOATSKULL2]},	// S_FLOATSKULL
{Spr(rSPR_FLAMINGSKULLS,1|FF_FULLBRIGHT),3*4,0,&states[S_FLOATSKULL3]},	// S_FLOATSKULL2
{Spr(rSPR_FLAMINGSKULLS,2|FF_FULLBRIGHT),3*4,0,&states[S_FLOATSKULL]},	// S_FLOATSKULL3
{Spr(rSPR_BLUETORCH,0|FF_FULLBRIGHT),2*4,0,&states[S_BTORCHSHRT2]},	// S_BTORCHSHRT
{Spr(rSPR_BLUETORCH,1|FF_FULLBRIGHT),2*4,0,&states[S_BTORCHSHRT3]},	// S_BTORCHSHRT2
{Spr(rSPR_BLUETORCH,2|FF_FULLBRIGHT),2*4,0,&states[S_BTORCHSHRT4]},	// S_BTORCHSHRT3
{Spr(rSPR_BLUETORCH,3|FF_FULLBRIGHT),2*4,0,&states[S_BTORCHSHRT]},	// S_BTORCHSHRT4
{Spr(rSPR_GREENTORCH,0|FF_FULLBRIGHT),2*4,0,&states[S_GTORCHSHRT2]},	// S_GTORCHSHRT
{Spr(rSPR_GREENTORCH,1|FF_FULLBRIGHT),2*4,0,&states[S_GTORCHSHRT3]},	// S_GTORCHSHRT2
{Spr(rSPR_GREENTORCH,2|FF_FULLBRIGHT),2*4,0,&states[S_GTORCHSHRT4]},	// S_GTORCHSHRT3
{Spr(rSPR_GREENTORCH,3|FF_FULLBRIGHT),2*4,0,&states[S_GTORCHSHRT]},	// S_GTORCHSHRT4
{Spr(rSPR_REDTORCH,0|FF_FULLBRIGHT),2*4,0,&states[S_RTORCHSHRT2]},	// S_RTORCHSHRT
{Spr(rSPR_REDTORCH,1|FF_FULLBRIGHT),2*4,0,&states[S_RTORCHSHRT3]},	// S_RTORCHSHRT2
{Spr(rSPR_REDTORCH,2|FF_FULLBRIGHT),2*4,0,&states[S_RTORCHSHRT4]},	// S_RTORCHSHRT3
{Spr(rSPR_REDTORCH,3|FF_FULLBRIGHT),2*4,0,&states[S_RTORCHSHRT]}	// S_RTORCHSHRT4
};

/**********************************

	Object info...

doomednum
spawnstate
spawnhealth
seestate
seesound
reactiontime
attacksound
painstate
painchance
painsound
meleestate
missilestate
deathstate
xdeathstate
deathsound
speed
radius
height
mass
damage
activesound
flags

**********************************/


mobjinfo_t mobjinfo[NUMMOBJTYPES] = {
{		// MT_PLAYER
&states[S_PLAY],		/* Spawn */
&states[S_PLAY_RUN1],	/* See */
&states[S_PLAY_PAIN],	/* Pain */
S_NULL,		/* Melee */
&states[S_PLAY_ATK1],	/* Missile */
&states[S_PLAY_DIE1],	/* Death */
&states[S_PLAY_XDIE1],	/* Gross death */
16*FRACUNIT,	/* Radius */
56*FRACUNIT,	/* Height */
-1,			/* Spawn number */
100,		/* Health */
255,		/* Pain chance */
100,		/* Mass */
MF_SOLID|MF_SHOOTABLE|MF_DROPOFF|MF_PICKUP|MF_NOTDMATCH,	/* Flags */
0,			/* Speed */
0,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_plpain,
sfx_pldeth,
sfx_None
},

{		// MT_POSSESSED
&states[S_POSS_STND],		/* Spawn */
&states[S_POSS_RUN1],	/* See */
&states[S_POSS_PAIN],	/* Pain */
S_NULL,		/* Melee */
&states[S_POSS_ATK1],	/* Missile */
&states[S_POSS_DIE1],	/* Death */
&states[S_POSS_XDIE1],	/* Gross death */
20*FRACUNIT,	/* Radius */
56*FRACUNIT,	/* Height */
3004,			/* Spawn number */
20,		/* Health */
200,		/* Pain chance */
100,		/* Mass */
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,	/* Flags */
8,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_posit1,
sfx_pistol,
sfx_popain,
sfx_podth1,
sfx_posact
},

{		// MT_SHOTGUY
&states[S_SPOS_STND],		/* Spawn */
&states[S_SPOS_RUN1],	/* See */
&states[S_SPOS_PAIN],	/* Pain */
S_NULL,		/* Melee */
&states[S_SPOS_ATK1],	/* Missile */
&states[S_SPOS_DIE1],	/* Death */
&states[S_SPOS_XDIE1],	/* Gross death */
20*FRACUNIT,	/* Radius */
56*FRACUNIT,	/* Height */
9,			/* Spawn number */
30,		/* Health */
170,		/* Pain chance */
100,		/* Mass */
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,	/* Flags */
8,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_posit2,
0,
sfx_popain,
sfx_podth2,
sfx_posact
},

{		// MT_TROOP
&states[S_TROO_STND],		/* Spawn */
&states[S_TROO_RUN1],	/* See */
&states[S_TROO_PAIN],	/* Pain */
&states[S_TROO_ATK1],		/* Melee */
&states[S_TROO_ATK1],	/* Missile */
&states[S_TROO_DIE1],	/* Death */
&states[S_TROO_XDIE1],	/* Gross death */
20*FRACUNIT,	/* Radius */
56*FRACUNIT,	/* Height */
3001,			/* Spawn number */
60,		/* Health */
200,		/* Pain chance */
100,		/* Mass */
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,	/* Flags */
8,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_bgsit1,
0,
sfx_popain,
sfx_bgdth1,
sfx_bgact
},

{		// MT_SERGEANT
&states[S_SARG_STND],		/* Spawn */
&states[S_SARG_RUN1],	/* See */
&states[S_SARG_PAIN],	/* Pain */
&states[S_SARG_ATK1],		/* Melee */
S_NULL,	/* Missile */
&states[S_SARG_DIE1],	/* Death */
S_NULL,	/* Gross death */
30*FRACUNIT,	/* Radius */
56*FRACUNIT,	/* Height */
3002,			/* Spawn number */
150,		/* Health */
180,		/* Pain chance */
400,		/* Mass */
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,	/* Flags */
8,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_sgtsit,
sfx_sgtatk,
sfx_dmpain,
sfx_sgtdth,
sfx_dmact
},

{		// MT_SHADOWS
&states[S_SARG_STND],		/* Spawn */
&states[S_SARG_RUN1],	/* See */
&states[S_SARG_PAIN],	/* Pain */
&states[S_SARG_ATK1],		/* Melee */
S_NULL,	/* Missile */
&states[S_SARG_DIE1],	/* Death */
S_NULL,	/* Gross death */
30*FRACUNIT,	/* Radius */
56*FRACUNIT,	/* Height */
58,			/* Spawn number */
150,		/* Health */
180,		/* Pain chance */
400,		/* Mass */
MF_SOLID|MF_SHOOTABLE|MF_SHADOW|MF_COUNTKILL,	/* Flags */
8,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_sgtsit,
sfx_sgtatk,
sfx_dmpain,
sfx_sgtdth,
sfx_dmact
},

{		// MT_HEAD
&states[S_HEAD_STND],		/* Spawn */
&states[S_HEAD_RUN1],	/* See */
&states[S_HEAD_PAIN],	/* Pain */
S_NULL,		/* Melee */
&states[S_HEAD_ATK1],	/* Missile */
&states[S_HEAD_DIE1],	/* Death */
S_NULL,	/* Gross death */
31*FRACUNIT,	/* Radius */
56*FRACUNIT,	/* Height */
3005,			/* Spawn number */
400,		/* Health */
128,		/* Pain chance */
400,		/* Mass */
MF_SOLID|MF_SHOOTABLE|MF_FLOAT|MF_NOGRAVITY|MF_COUNTKILL,	/* Flags */
4,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_cacsit,
0,
sfx_dmpain,
sfx_cacdth,
sfx_dmact
},

{		// MT_BRUISER
&states[S_BOSS_STND],		/* Spawn */
&states[S_BOSS_RUN1],	/* See */
&states[S_BOSS_PAIN],	/* Pain */
&states[S_BOSS_ATK1],		/* Melee */
&states[S_BOSS_ATK1],	/* Missile */
&states[S_BOSS_DIE1],	/* Death */
S_NULL,	/* Gross death */
24*FRACUNIT,	/* Radius */
64*FRACUNIT,	/* Height */
3003,			/* Spawn number */
1000,		/* Health */
50,		/* Pain chance */
1000,		/* Mass */
MF_SOLID|MF_SHOOTABLE|MF_COUNTKILL,	/* Flags */
8,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_brssit,
0,
sfx_dmpain,
sfx_brsdth,
sfx_dmact
},

{		// MT_SKULL
&states[S_SKULL_STND],		/* Spawn */
&states[S_SKULL_RUN1],	/* See */
&states[S_SKULL_PAIN],	/* Pain */
S_NULL,		/* Melee */
&states[S_SKULL_ATK1],	/* Missile */
&states[S_SKULL_DIE1],	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
56*FRACUNIT,	/* Height */
3006,			/* Spawn number */
60,		/* Health */
256,		/* Pain chance */
50,		/* Mass */
MF_SOLID|MF_SHOOTABLE|MF_FLOAT|MF_NOGRAVITY|MF_COUNTKILL,	/* Flags */
8,			/* Speed */
8,			/* Reaction */
3,			/* Damage amount */
0,
sfx_sklatk,
sfx_dmpain,
sfx_firxpl,
sfx_dmact
},

{		// MT_BARREL
&states[S_BAR1],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
&states[S_BEXP],	/* Death */
S_NULL,	/* Gross death */
10*FRACUNIT,	/* Radius */
42*FRACUNIT,	/* Height */
2035,			/* Spawn number */
10,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID|MF_SHOOTABLE|MF_NOBLOOD,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_barexp,
sfx_None
},

{		// MT_TROOPSHOT
&states[S_TBALL1],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
&states[S_TBALLX1],	/* Death */
S_NULL,	/* Gross death */
6*FRACUNIT,	/* Radius */
8*FRACUNIT,	/* Height */
-1,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,	/* Flags */
20,			/* Speed */
8,			/* Reaction */
3,			/* Damage amount */
sfx_firsht,
sfx_None,
sfx_None,
sfx_firxpl,
sfx_None
},

{		// MT_HEADSHOT
&states[S_RBALL1],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
&states[S_RBALLX1],	/* Death */
S_NULL,	/* Gross death */
6*FRACUNIT,	/* Radius */
8*FRACUNIT,	/* Height */
-1,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,	/* Flags */
20,			/* Speed */
8,			/* Reaction */
5,			/* Damage amount */
sfx_firsht,
sfx_None,
sfx_None,
sfx_firxpl,
sfx_None
},

{		// MT_BRUISERSHOT
&states[S_BRBALL1],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
&states[S_BRBALLX1],	/* Death */
S_NULL,	/* Gross death */
6*FRACUNIT,	/* Radius */
8*FRACUNIT,	/* Height */
-1,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,	/* Flags */
30,			/* Speed */
8,			/* Reaction */
8,			/* Damage amount */
sfx_firsht,
sfx_None,
sfx_None,
sfx_firxpl,
sfx_None
},

{		// MT_ROCKET
&states[S_ROCKET],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
&states[S_EXPLODE1],	/* Death */
S_NULL,	/* Gross death */
11*FRACUNIT,	/* Radius */
8*FRACUNIT,	/* Height */
-1,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,	/* Flags */
40,			/* Speed */
8,			/* Reaction */
20,			/* Damage amount */
sfx_rlaunc,
sfx_None,
sfx_None,
sfx_barexp,
sfx_None
},

{		// MT_PLASMA
&states[S_PLASBALL],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
&states[S_PLASEXP],	/* Death */
S_NULL,	/* Gross death */
13*FRACUNIT,	/* Radius */
8*FRACUNIT,	/* Height */
-1,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,	/* Flags */
50,			/* Speed */
8,			/* Reaction */
5,			/* Damage amount */
sfx_plasma,
sfx_None,
sfx_None,
sfx_firxpl,
sfx_None
},

{		// MT_BFG
&states[S_BFGSHOT],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
&states[S_BFGLAND],	/* Death */
S_NULL,	/* Gross death */
13*FRACUNIT,	/* Radius */
8*FRACUNIT,	/* Height */
-1,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP|MF_MISSILE|MF_DROPOFF|MF_NOGRAVITY,	/* Flags */
30,			/* Speed */
8,			/* Reaction */
100,			/* Damage amount */
0,
sfx_None,
sfx_None,
sfx_rxplod,
sfx_None
},

{		// MT_PUFF
&states[S_PUFF1],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
-1,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP|MF_NOGRAVITY,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_BLOOD
&states[S_BLOOD1],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
-1,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_TFOG
&states[S_TFOG],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
-1,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP|MF_NOGRAVITY,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_IFOG
&states[S_IFOG],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
-1,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP|MF_NOGRAVITY,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_TELEPORTMAN
S_NULL,		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
14,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP|MF_NOSECTOR,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_EXTRABFG
&states[S_BFGEXP],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
-1,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_NOBLOCKMAP|MF_NOGRAVITY,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC0
&states[S_ARM1],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2018,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC1
&states[S_ARM2],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2019,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC2
&states[S_BON1],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2014,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_COUNTITEM,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC3
&states[S_BON2],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2015,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_COUNTITEM,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC4
&states[S_BKEY],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
5,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_NOTDMATCH,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC5
&states[S_RKEY],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
13,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_NOTDMATCH,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC6
&states[S_YKEY],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
6,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_NOTDMATCH,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC7
&states[S_YSKULL],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
39,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_NOTDMATCH,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC8
&states[S_RSKULL],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
38,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_NOTDMATCH,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC9
&states[S_BSKULL],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
40,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_NOTDMATCH,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		/* Stimpack */
&states[S_STIM],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2011,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC11
&states[S_MEDI],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2012,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC12
&states[S_SOUL],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2013,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_COUNTITEM,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_INVULNERABILITY
&states[S_PINV],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2022,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_COUNTITEM,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC13
&states[S_PSTR],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2023,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_COUNTITEM,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_INVISIBILITY
&states[S_PINS],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2024,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_COUNTITEM,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC14
&states[S_SUIT],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2025,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC15
&states[S_PMAP],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2026,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_COUNTITEM,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC16
&states[S_PVIS],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2045,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL|MF_COUNTITEM,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_CLIP
&states[S_CLIP],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2007,			/* Spawn number */
10,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC17
&states[S_AMMO],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2048,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC18
&states[S_ROCK],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2010,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC19
&states[S_BROK],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2046,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC20
&states[S_CELL],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2047,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC21
&states[S_CELP],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
17,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC22
&states[S_SHEL],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2008,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC23
&states[S_SBOX],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2049,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC24
&states[S_BPAK],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
8,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC25
&states[S_BFUG],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2006,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_CHAINGUN
&states[S_MGUN],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2002,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC26
&states[S_CSAW],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2005,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC27
&states[S_LAUN],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2003,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC28
&states[S_PLAS],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2004,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_SHOTGUN
&states[S_SHOT],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2001,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPECIAL,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC29
&states[S_COLU],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
2028,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC30
&states[S_SHRTGRNCOL],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
31,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC31
&states[S_SHRTREDCOL],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
33,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC32
&states[S_FLOATSKULL],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
42,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC33
&states[S_TORCHTREE],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
43,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC34
&states[S_BTORCHSHRT],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
55,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC35
&states[S_GTORCHSHRT],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
56,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC36
&states[S_RTORCHSHRT],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
57,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC37
&states[S_STALAGTITE],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
47,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC38
&states[S_TECHPILLAR],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
48,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC39
&states[S_CANDLESTIK],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
34,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
0,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC40
&states[S_CANDELABRA],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
35,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC41
&states[S_MEAT5],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
52*FRACUNIT,	/* Height */
53,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID|MF_SPAWNCEILING|MF_NOGRAVITY,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC42
&states[S_MEAT5],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
52*FRACUNIT,	/* Height */
62,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SPAWNCEILING|MF_NOGRAVITY,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_DEADCACODEMON
&states[S_HEAD_DIE6],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
22,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
0,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC44
&states[S_POSS_DIE5],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
18,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
0,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_MISC45
&states[S_SARG_DIE6],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
21,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
0,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_DEADIMP
&states[S_TROO_DIE5],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
20,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
0,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_DEADSHOTGUY
&states[S_SPOS_DIE5],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
19,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
0,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_GROSSPLAYER1
&states[S_PLAY_XDIE9],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
10,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
0,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_GROSSPLAYER2
&states[S_PLAY_XDIE9],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
12,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
0,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_FIVESKULLS
&states[S_HEADSONSTICK],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
28,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_GIBS
&states[S_GIBS],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
20*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
24,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
0,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_SKULLONPOLE
&states[S_HEADONASTICK],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
27,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_IMPALEDHUMAN
&states[S_DEADSTICK],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
25,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
},

{		// MT_BURNINGBARREL
&states[S_BBAR1],		/* Spawn */
S_NULL,	/* See */
S_NULL,	/* Pain */
S_NULL,		/* Melee */
S_NULL,	/* Missile */
S_NULL,	/* Death */
S_NULL,	/* Gross death */
16*FRACUNIT,	/* Radius */
16*FRACUNIT,	/* Height */
70,			/* Spawn number */
1000,		/* Health */
0,		/* Pain chance */
100,		/* Mass */
MF_SOLID,	/* Flags */
0,			/* Speed */
8,			/* Reaction */
0,			/* Damage amount */
sfx_None,
sfx_None,
sfx_None,
sfx_None,
sfx_None
}
};
