/**********************************

	Resource map for all data in DOOM

**********************************/

#define rTEXTURE1 1		/* List of wall textures to preload */
enum {
	rT_START=2,
	rBIGDOOR2=rT_START,
	rBIGDOOR6,
	rBRNPOIS,
	rBROWNGRN,
	rBROWN1,
	rCOMPSPAN,
	rCOMPTALL,
	rCRATE1,
	rCRATELIT,
	rCRATINY,
	rDOOR1,
	rDOOR3,
	rDOORBLU,
	rDOORRED,
	rDOORSTOP,
	rDOORTRAK,
	rDOORYEL,
	rEXITDOOR,
	rEXITSIGN,
	rGRAY5,
	rGSTSATYR,
	rLITE5,
	rMARBFAC3,
	rMETAL,
	rMETAL1,
	rNUKE24,
	rPIPE2,
	rPLAT1,
	rSHAWN2,
	rSKINEDGE,
	rSKY1,
	rSKY2,
	rSKY3,
	rSLADWALL,
	rSP_DUDE4,
	rSP_HOT1,
	rSTEP6,
	rSUPPORT2,
	rSUPPORT3,
	rSW1BRN1,
	rSW1GARG,
	rSW1GSTON,
	rSW1HOT,
	rSW1WOOD,
	rSW2BRN1,
	rSW2GARG,
	rSW2GSTON,
	rSW2HOT,
	rSW2WOOD,
	rBRICK01,
	rBRICK02,
	rBRICK03,
	rDFACE01,
	rMARBLE01,
	rMARBLE02,
	rMARBLE03,
	rMARBLE04,
	rWOOD01,
	rASH01,
	rCEMENT01,
	rCBLUE01,
	rTECH01,
	rTECH02,
	rTECH03,
	rTECH04,
	rSKIN01,
	rSKIN02,
	rSKIN03,
	rSKULLS01,
	rSTWAR01,
	rSTWAR02,
	rCOMTAL02,
	rSW1STAR,
	rSW2STAR,
	rT_END
};

enum {
	rF_START=rT_END,
	rFLAT14=rF_START,
	rFLAT23,
	rFLAT5_2,
	rFLAT5_4,
	rNUKAGE1,
	rNUKAGE2,
	rNUKAGE3,
	rFLOOR0_1,
	rFLOOR0_3,
	rFLOOR0_6,
	rFLOOR3_3,
	rFLOOR4_6,
	rFLOOR4_8,
	rFLOOR5_4,
	rSTEP1,
	rSTEP2,
	rFLOOR6_1,
	rFLOOR6_2,
	rTLITE6_4,
	rTLITE6_6,
	rFLOOR7_1,
	rFLOOR7_2,
	rMFLR8_1,
	rMFLR8_4,
	rCEIL3_2,
	rCEIL3_4,
	rCEIL5_1,
	rCRATOP1,
	rCRATOP2,
	rFLAT4,
	rFLAT8,
	rGATE3,
	rGATE4,
	rFWATER1,
	rFWATER2,
	rFWATER3,
	rFWATER4,
	rLAVA1,
	rLAVA2,
	rLAVA3,
	rLAVA4,
	rGRASS,
	rROCKS,
	rF_END
};

enum {
	rBACKGROUNDMASK=rF_END,		// Background for actual game play
	rTITLE=rBACKGROUNDMASK+6,	// Large doom logo
	rIDCREDITS,	// ID Credits
	rCREDITS,	// Art data Credits
	rLOGCREDITS,	// Logicware credits
	rBACKGRND,	// Background of skulls
	rBACKGRNDBROWN,	// Brown skulls in background
	rCHARSET,	// Large font
	rPAUSED,	// Game is paused
	rLOADING,	// Game is loading
	rBIGNUMB,	// Numbers, minus and percent
	rINTERMIS,	// Intermission text
	rSTBAR,		// Normal status bar
	rSBARSHP,	// Shapes for status bar
	rFACES,		// Shapes for faces
	rSKULLS,	// Skull cursors
	rMAINDOOM,	// Main menu background
	rMAINMENU,	// Shapes for main menu
	rSLIDER,	// Shapes for menu slider

	rDEMO1,
	rDEMO2,

	rMAP01,			/* Game maps */
	rMAP02=rMAP01+10,
	rMAP03=rMAP02+10,
	rMAP04=rMAP03+10,
	rMAP05=rMAP04+10,
	rMAP06=rMAP05+10,
	rMAP07=rMAP06+10,
	rMAP08=rMAP07+10,
	rMAP09=rMAP08+10,
	rMAP10=rMAP09+10,
	rMAP11=rMAP10+10,
	rMAP12=rMAP11+10,
	rMAP13=rMAP12+10,
	rMAP14=rMAP13+10,
	rMAP15=rMAP14+10,
	rMAP16=rMAP15+10,
	rMAP17=rMAP16+10,
	rMAP18=rMAP17+10,
	rMAP19=rMAP18+10,
	rMAP20=rMAP19+10,
	rMAP21=rMAP20+10,
	rMAP22=rMAP21+10,
	rMAP23=rMAP22+10,
	rMAP24=rMAP23+10,

	rFIRSTSPRITE=rMAP24+10,	/* Sprite shapes */
	rSPR_BIGFISTS=rFIRSTSPRITE,	/* Player's fists */
	rSPR_BIGPISTOL,			/* Player's pistol */
	rSPR_BIGSHOTGUN,		/* Player's shotgun */
	rSPR_BIGCHAINGUN,		/* Player's chain gun */
	rSPR_BIGROCKET,			/* Player's rocket launcher */
	rSPR_BIGPLASMA,			/* Player's plasma rifle */
	rSPR_BIGBFG,			/* Player's BFG 9000 */
	rSPR_BIGCHAINSAW,		/* Player's chainsaw */
	rSPR_ZOMBIE,			/* Zombiemen */
	rSPR_ZOMBIEBODY,		/* Dead zombiemen */
	rSPR_SHOTGUY,			/* Shotgun guys */
	rSPR_IMP,				/* Imps */
	rSPR_DEMON,				/* Demons */
	rSPR_CACODEMON,			/* Cacodemons */
	rSPR_CACODIE,			/* Cacodemon's dying animation */
	rSPR_CACOBDY,			/* Cacodemon's dead body */
	rSPR_CACOBOLT,			/* Cacodemon's fireball */
	rSPR_LOSTSOUL,			/* Lost souls */
	rSPR_BARON,				/* Baron of Hell */
	rSPR_BARONATK,			/* Baron of Hell attacking */
	rSPR_BARONDIE,			/* Baron of Hell dying */
	rSPR_BARONBDY,			/* Baron of Hell's dead body */
	rSPR_BARONBLT,			/* Baron of Hell's fireball */
	rSPR_OURHERO,			/* Our hero */
	rSPR_OURHEROBDY,		/* Our dead hero */
	rSPR_BARREL,			/* Exploding barrel */
	rSPR_IFOG,				/* Respawn fog */
	rSPR_FIRECAN,			/* Burning barrel */
	rSPR_POOLBLOOD,			/* Pool of blood */
	rSPR_CANDLE,			/* Candlestick */
	rSPR_CANDLEABRA,		/* Candleabra */
	rSPR_SHOTGUN,			/* Shotgun on floor */
	rSPR_CHAINGUN,			/* Chaingun on floor */
	rSPR_ROCKETLAUNCHER,	/* Rocket launcher on floor */
	rSPR_CHAINSAW,			/* Chainsaw on floor */
	rSPR_CLIP,				/* Clip of bullets */
	rSPR_SHELLS,			/* 4 shotgun shells */
	rSPR_ROCKET,			/* A single rocket */
	rSPR_STIMPACK,			/* Stimpack */
	rSPR_MEDIKIT,			/* Med-kit */
	rSPR_GREENARMOR,		/* Normal armor */
	rSPR_BLUEARMOR,			/* Mega armor */
	rSPR_LIGHTCOLUMN,		/* Light column */	
	rSPR_BACKPACK,			/* Backpack */
	rSPR_BOXROCKETS,		/* Box of rockets */
	rSPR_BOXAMMO,			/* Box of bullets */
	rSPR_BOXSHELLS,			/* Box of shotgun shells */
	rSPR_TECHPILLAR,		/* Techincal pillar */
	rSPR_BLUEKEYCARD,		/* Blue key card */
	rSPR_YELLOWKEYCARD,		/* Yellow key card */
	rSPR_REDKEYCARD,		/* Red key card */
	rSPR_RADIATIONSUIT,		/* Radiation suit */
	rSPR_IRGOGGLES,			/* Night vision goggles */
	rSPR_COMPUTERMAP,		/* Computer area map */
	rSPR_INVISIBILITY,		/* Invisibility */
	rSPR_HEALTHBONUS,		/* Health bonus */
	rSPR_SOULSPHERE,		/* Soul sphere */
	rSPR_ARMORBONUS,		/* Armor bonus */
	rSPR_BODYPOLE,			/* Skewered paraplegic */
	rSPR_FIVESKULLPOLE,		/* 5 skulls on a pole */
	rSPR_TALLSKULLPOLE,		/* Skull on tall pole */
	rSPR_SHORTGREENPILLAR,	/* Short green pillar */
	rSPR_SHORTREDPILLAR,	/* Short red pillar */
	rSPR_FLAMINGSKULLS,		/* Floating skull */
	rSPR_MEDDEADTREE,		/* Medium dead tree */
	rSPR_LARGESTAL,			/* Large stalactite */
	rSPR_SMALLSTAL,			/* Small stalactite */
	rSPR_BLUESKULLKEY,		/* Blue skull key */
	rSPR_REDSKULLKEY,		/* Red skull key */
	rSPR_YELLOWSKULLKEY,	/* Yellow skull key */
	rSPR_PLASMARIFLE,		/* Plasma rifle on floor */
	rSPR_BFG9000,			/* BFG 9000 on floor */
	rSPR_CELL,				/* Small cell pack */
	rSPR_BERZERKER,			/* Berzerker pack */
	rSPR_CELLPACK,			/* Large cell pack */
	rSPR_HANGINGRUMP,		/* Rump hanging from ceiling */
	rSPR_REDTORCH,			/* Red torch */
	rSPR_BLUETORCH,			/* Blue torch */
	rSPR_GREENTORCH,		/* Green torch */
	rSPR_INVULNERABILITY,	/* Invulnerability sphere */
	rSPR_TELEFOG,			/* Teleport fog */
	rSPR_BFS1,				/* BFG shot */
	rSPR_BFE1,				/* BFG explosion */
	rSPR_BFE2,				/* BFG target hit */
	rSPR_PLSS,				/* Plasma rifle round */
	rSPR_PLSE,				/* Plasma rifle explosion */
	rSPR_MISL,				/* Rocket in flight */
	rSPR_BLUD,				/* Blood from bullet hit */
	rSPR_PUFF,				/* Gun sparks on wall */
	rLASTSPRITE
};
