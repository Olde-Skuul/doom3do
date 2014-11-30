#include <Burger.h>
#include "States.h"
#include "Sounds.h"
#include "DoomRez.h"

/****************************

	Global defines

****************************/

#define	FRACBITS 16				/* Number of fraction bits in Fixed */
#define	FRACUNIT (1<<FRACBITS)	/* 1.0 in fixed point */
#define	ANG45 0x20000000		/* 45 degrees in angle_t */
#define	ANG90 0x40000000		/* 90 degrees in angle_t */
#define	ANG180 0x80000000		/* 180 degrees in angle_t */
#define	ANG270 0xC0000000		/* 270 degrees in angle_t */
#define	FINEANGLES 8192			/* Size of the fineangle table */
#define	FINEMASK (FINEANGLES-1)	/* Rounding mask for table */
#define	ANGLETOFINESHIFT 19		/* Convert angle_t to fineangle table */
#define	ANGLETOSKYSHIFT	22		/* sky map is 256*128*4 maps */
#define	MINZ (FRACUNIT*4)		/* Closest z allowed */
#define FIELDOFVIEW 2048		/* 90 degrees of view */
#define BFGCELLS 40				/* Number of energy units per blast */
#define	MAXHEALTH 100			/* Normal health at start of game */
#define SKULLSPEED (40*FRACUNIT)	/* Speed of the skull to attack */
#define	MAXVISSPRITES 128			/* Maximum number of visible sprites */
#define	MAXVISPLANES 64	/* Maximum number of visible floor and ceiling textures */
#define	MAXWALLCMDS 128	/* Maximum number of visible walls */
#define MAXOPENINGS MAXSCREENWIDTH*64	/* Space for sil tables */
#define MAXSCREENHEIGHT 160		/* Maximum height allowed */
#define MAXSCREENWIDTH 280		/* Maximum width allowed */
#define	SLOPERANGE 2048			/* Number of entries in tantoangle table */
#define	SLOPEBITS 11			/* Power of 2 for SLOPERANGE (2<<11) */
#define	HEIGHTBITS 6			/* Number of bits for texture height */
#define	FIXEDTOSCALE (FRACBITS-SCALEBITS)	/* Number of unused bits from fixed to HEIGHTBITS */
#define	SCALEBITS 9				/* Number of bits for texture scale */
#define	FIXEDTOHEIGHT (FRACBITS-HEIGHTBITS) /* Number of unused bits from fixed to SCALEBITS */
#define MAXINT ((int)0x7fffffff)	/* max pos 32-bit int */
#define MININT ((int)0x80000000)	/* max negative 32-bit integer */
#define ONFLOORZ MININT		/* Attach object to floor with this z */
#define	ONCEILINGZ MAXINT	/* Attach object to ceiling with this z */
#define	FLOATSPEED (8*FRACUNIT)	/* Speed an object can float vertically */
#define	GRAVITY (4*FRACUNIT)	/* Rate of fall */
#define	MAXMOVE (16*FRACUNIT)	/* Maximum velocity */
#define	VIEWHEIGHT (41*FRACUNIT)	/* Height to render from */
#define GLOWSPEED 16				/* Steps for a glowing light */
#define	STROBEBRIGHT (TICKSPERSEC/5)	/* Time to increase brightness for strobe */
#define	FASTDARK (TICKSPERSEC/2)	/* Ticks to glow quickly */
#define	SLOWDARK (TICKSPERSEC)		/* Ticks to glow slowly */
#define	INVULNTICS (30*TICKSPERSEC)	/* Time for invulnerability */
#define	INVISTICS (60*TICKSPERSEC)	/* Time for invisibility */
#define	INFRATICS (120*TICKSPERSEC)	/* Time for I/R goggles */
#define	IRONTICS (60*TICKSPERSEC)	/* Time for rad suit */
#define	MAPBLOCKSHIFT (FRACBITS+7)	/* Shift value to convert Fixed to 128 pixel blocks */
#define	PLAYERRADIUS (16<<FRACBITS)	/* Radius of the player */
#define	MAXRADIUS (32<<FRACBITS)	/* Largest radius of any critter */
#define	USERANGE (70<<FRACBITS)		/* Range of pressing spacebar to activate something */
#define	MELEERANGE (70<<FRACBITS)	/* Range of hand to hand combat */
#define	MISSILERANGE (32*64<<FRACBITS)	/* Range of guns targeting */


/****************************

	Global typedefs

****************************/

typedef LongWord angle_t;			/* Global angle */

/****************************

	Global Enums

****************************/

typedef enum {		/* Skill level settings */
	sk_baby,
	sk_easy,
	sk_medium,
	sk_hard,
	sk_nightmare
} skill_t;

typedef enum {		/* Current game state setting */
	ga_nothing,
	ga_died,
	ga_completed,
	ga_secretexit,
	ga_warped,
	ga_exitdemo
} gameaction_t;

typedef enum {		/* Current state of the player */
	PST_LIVE,		/* playing */
	PST_DEAD,		/* dead on the ground */
	PST_REBORN		/* ready to restart */
} playerstate_t;

typedef enum {		/* Sprites to draw on the playscreen */
	ps_weapon,		/* Currently selected weapon */
	ps_flash,		/* Weapon muzzle flash */
	NUMPSPRITES		/* Number of shapes for array sizing */
} psprnum_t;

typedef enum {		/* Item types for keycards or skulls */
	it_bluecard,
	it_yellowcard,
	it_redcard,
	it_blueskull,
	it_yellowskull,
	it_redskull,
	NUMCARDS		/* Number of keys for array sizing */
} card_t;

typedef enum {		/* Currently selected weapon */
	wp_fist,
	wp_pistol,
	wp_shotgun,
	wp_chaingun,
	wp_missile,
	wp_plasma,
	wp_bfg,
	wp_chainsaw,
	NUMWEAPONS,		/* Number of valid weapons for array sizing */
	wp_nochange		/* Inbetween weapons (Can't fire) */
} weapontype_t;

typedef enum {		/* Current ammo being used */
	am_clip,		/* pistol / chaingun */
	am_shell,		/* shotgun */
	am_cell,		/* BFG */
	am_misl,		/* missile launcher */
	NUMAMMO,		/* Number of valid ammo types for array sizing */
	am_noammo		/* chainsaw / fist */
} ammotype_t;

typedef enum {		/* Power index flags */
	pw_invulnerability,	/* God mode */
	pw_strength,		/* Strength */
	pw_ironfeet,		/* Radiation suit */
	pw_allmap,			/* Mapping */
	pw_invisibility,	/* Can't see me! */
	NUMPOWERS			/* Number of powerups */
} powertype_t;

typedef enum {		/* Slopes for BSP tree */
	ST_HORIZONTAL,ST_VERTICAL,ST_POSITIVE,ST_NEGATIVE
} slopetype_t;

typedef enum {		/* Faces for our hero */
	f_none,			/* Normal state */
	f_faceleft,		/* turn face left */
	f_faceright,	/* turn face right */
	f_hurtbad,		/* surprised look when slammed hard */
	f_gotgat,		/* picked up a weapon smile */
	f_mowdown,		/* grimace while continuous firing */
	NUMSPCLFACES
} spclface_e;

typedef enum {		/* Basic direction list */
	DI_EAST,		/* East etc... */
	DI_NORTHEAST,
	DI_NORTH,
	DI_NORTHWEST,
	DI_WEST,
	DI_SOUTHWEST,
	DI_SOUTH,
	DI_SOUTHEAST,
	DI_NODIR,		/* No direction at all */
	NUMDIRS
} dirtype_t;

typedef enum {		/* Enums for floor types */
	lowerFloor,			/* lower floor to highest surrounding floor */
	lowerFloorToLowest,	/* lower floor to lowest surrounding floor */
	turboLower,			/* lower floor to highest surrounding floor VERY FAST */
	raiseFloor,			/* raise floor to lowest surrounding CEILING */
	raiseFloorToNearest,	/* raise floor to next highest surrounding floor */
	raiseToTexture,		/* raise floor to shortest height texture around it */
	lowerAndChange,		/* lower floor to lowest surrounding floor and change */
						/* floorpic */
	raiseFloor24,
	raiseFloor24AndChange,
	raiseFloorCrush,
	donutRaise
} floor_e;

typedef enum {		/* Enums for moving sector results */
	moveok,
	crushed,
	pastdest
} result_e;

typedef enum {			/* Enums for ceiling types */
	lowerToFloor,
	raiseToHighest,
	lowerAndCrush,
	crushAndRaise,
	fastCrushAndRaise
} ceiling_e;

typedef enum {			/* Enums for platform types */
	perpetualRaise,
	downWaitUpStay,
	raiseAndChange,
	raiseToNearestAndChange
} plattype_e;

typedef enum {		/* Enums for door types */
	normaldoor,
	close30ThenOpen,
	close,
	open,
	raiseIn5Mins
} vldoor_e;

enum {BOXTOP,BOXBOTTOM,BOXLEFT,BOXRIGHT,BOXCOUNT};	/* bbox coordinates */

/****************************

	Global structs

****************************/

typedef struct {		/* Struct for animating textures */
	Word LastPicNum;	/* Picture referance number */
	Word BasePic;		/* Base texture # */
	Word CurrentPic;	/* Current index */
} anim_t;

#define	MTF_EASY 1
#define	MTF_NORMAL 2
#define	MTF_HARD 4
#define	MTF_AMBUSH 8
#define MTF_DEATHMATCH 16

typedef struct {	/* Struct describing an object on the map */
	Fixed x,y;		/* X,Y position (Signed) */
	angle_t angle;	/* Angle facing */
	Word type;		/* Object type */
	Word ThingFlags;	/* mapthing flags */
} mapthing_t;

#define	FF_FULLBRIGHT	0x00000100	/* Flag in thing->frame for full light */
#define FF_FRAMEMASK	0x000000FF	/* Use the rest */
#define FF_SPRITESHIFT	16			/* Bits to shift for group */

typedef struct state_s {  		/* An actor's state */
	Word SpriteFrame;	/* Which sprite to display? */
	Word Time;	  		/* Time before next action */
	void (*action)();	/* Logic to execute for next state */
	struct state_s *nextstate;	/* Index to state table for next state */
} state_t;

typedef struct {	/* Describe a 2d object's placement in the 3d world */
	state_t	*StatePtr;	/* a NULL state means not active */
	Word Time;			/* Time to elapse before next state */
	int WeaponX,WeaponY; /* X and Y in pixels */
} pspdef_t;

typedef struct {		/* Describe an actor's basic variables */
	state_t *spawnstate;	/* State number to be spawned */
	state_t *seestate; 		/* First action state */
	state_t *painstate;		/* State when in pain */
	state_t *meleestate;	/* State when attacking */
	state_t *missilestate;	/* State for missile weapon */
	state_t *deathstate;	/* State for normal death */
	state_t *xdeathstate;	/* State for gruesome death */
	Fixed Radius;		/* Radius for collision checking */
	Fixed Height;		/* Height for collision checking */
	Word doomednum;		/* Number in doom ed */
	Word spawnhealth;	/* Hit points at spawning */
	Word painchance;		/* % chance of pain when hit */
	Word mass;			/* Mass for impact recoil */
	Word flags;			/* Generic state flags */
	Boolean Speed;		/* Rate of speed for normal chase or walk */
	Boolean reactiontime;	/* Time before first action */
	Boolean damage;		/* Damage done for attack */
	Boolean seesound;		/* Sound effect after first action */
	Boolean attacksound;	/* Sound when attacking */
	Boolean painsound;		/* Pain sound */
	Boolean deathsound;	/* Sound for normal death */
	Boolean activesound;	/* Sound to play at random times for mood */
} mobjinfo_t;

/* a patch holds one or more columns */
/* patches are used for sprites and all masked pictures */

#define PT_FLIP 0x80000000
#define PT_NOROTATE 0x40000000

typedef struct {
	short leftoffset;	/* pixels to the left of origin */
	short topoffset;	/* pixels below the origin */
	Byte Data[1];
} patch_t;

typedef struct {		/* Describe a 2D shape to the screen */
	int	x1,x2,y1,y2;	/* Clipped to screen edges column range */
	Fixed xscale;		/* Scale factor */
	Fixed yscale;		/* Y Scale factor */
	Word PatchLump;		/* Resource to the sprite art data */
	LongWord PatchOffset;	/* Offset to the proper record */
	Word colormap;		/* 0x8000 = shadow draw,0x4000 flip, 0x3FFF color map */
	struct mobj_s *thing;		/* Used for clipping... */
} vissprite_t;

typedef struct {		/* Describe a floor texture */
	Word open[MAXSCREENWIDTH+1];	/* top<<8 | bottom */
	Fixed height;		/* Height of the floor */
	void **PicHandle;	/* Texture handle */
	Word PlaneLight;	/* Light override */
	int minx,maxx;		/* Minimum x, max x */
} visplane_t;

#define	MF_SPECIAL		1			// call P_SpecialThing when touched
#define	MF_SOLID		2
#define	MF_SHOOTABLE	4
#define	MF_NOSECTOR		8			// don't use the sector links
									// (invisible but touchable)
#define	MF_NOBLOCKMAP	16			// don't use the BlockLinkPtr
									// (inert but displayable)
#define	MF_AMBUSH		32			/* Only attack when seen */
#define	MF_JUSTHIT		64			// try to attack right back
#define	MF_JUSTATTACKED	128			// take at least one step before attacking
#define	MF_SPAWNCEILING	0x100		// hang from ceiling instead of floor
#define	MF_NOGRAVITY	0x200		// don't apply gravity every tic
#define	MF_DROPOFF		0x400		// allow jumps from high places
#define	MF_PICKUP		0x800		// for players to pick up items
#define	MF_NOCLIP		0x1000		// player cheat
#define	MF_SLIDE		0x2000		// keep info about sliding along walls
#define	MF_FLOAT		0x4000		// allow moves to any height, no gravity
#define	MF_TELEPORT		0x8000		// don't cross lines or look at heights
#define MF_MISSILE		0x10000		// don't hit same species, explode on block
#define	MF_DROPPED		0x20000		// dropped by a demon, not level spawned
#define	MF_SHADOW		0x40000		// use fuzzy draw (shadow demons / invis)
#define	MF_NOBLOOD		0x80000		// don't bleed when shot (use puff)
#define	MF_CORPSE		0x100000	// don't stop moving halfway off a step
#define	MF_INFLOAT		0x200000	// floating to a height for a move, don't
									// auto float to target's height
#define	MF_COUNTKILL	0x400000	// count towards intermission kill total
#define	MF_COUNTITEM	0x800000	// count towards intermission item total
#define	MF_SKULLFLY		0x1000000	// skull in flight
#define	MF_NOTDMATCH	0x2000000	// don't spawn in death match (key cards)
#define	MF_SEETARGET	0x4000000	// is target visible?

typedef struct mobj_s {	
	struct mobj_s *prev,*next;	/* Linked list entries */
	Fixed x,y,z;		/* Location in 3Space */

/* info for drawing */

	struct mobj_s *snext,*sprev;	/* links in sector (if needed) */
	angle_t	angle;		/* Angle of view */

/* interaction info */

	struct mobj_s *bnext,*bprev;	/* links in blocks (if needed) */
	struct subsector_s *subsector;	/* Subsector currently standing on */
	Fixed floorz,ceilingz;		// closest together of contacted secs
	Fixed radius,height;		// for movement checking
	Fixed momx,momy,momz;		// momentums

	mobjinfo_t *InfoPtr;	/* Pointer to mobj info record */
	Word tics;		/* Time before next state */
	state_t	*state;	/* Pointer to current state record (Can't be NULL!) */
	Word flags;		/* State flags for object */
	Word MObjHealth;	/* Object's health */
	Word movedir;	/* 0-7 */
	Word movecount;	/* when 0, select a new dir */
	struct mobj_s *target;	/* thing being chased/attacked (or NULL) */
							/* also the originator for missiles */
	Word reactiontime;	/* if non 0, don't attack yet */
						/* used by player to freeze a bit after teleporting */
	Word threshold;		/* if >0, the target will be chased */
						/* no matter what (even if shot) */
	struct player_s	*player;	/* only valid if type == MT_PLAYER */
} mobj_t;

#define	AF_ACTIVE 1		/* Automap active */
#define	AF_FOLLOW 2		/* Follow mode on */
#define	AF_ALLLINES 4	/* All lines cheat */
#define	AF_ALLMOBJ 8	/* All objects cheat */
#define AF_NOCLIP 0x10	/* Can walk through walls */
#define	AF_GODMODE 0x20	/* No one can hurt me! */
#define	AF_OPTIONSACTIVE 0x80 /* options screen running */

typedef struct player_s {	/* Player's current game state */
	mobj_t *mo;				/* Pointer to sprite object */

	Word health;			/* only used between levels, mo->health */ 
							/* is used during levels */
	Word armorpoints;		/* Amount of armor */
	Word armortype;			/* armor type is 0-2 */
	Word ammo[NUMAMMO];		/* Current ammo */
	Word maxammo[NUMAMMO];	/* Maximum ammo */
	Word killcount;			/* Number of critters killed */
	Word itemcount;			/* Number of items gathered */
	Word secretcount;		/* Number of secret sectors touched */
	char *message;		/* Hint messages */
	mobj_t *attacker;	/* Who did damage (NULL for floors) */
	Word extralight;		/* so gun flashes light up areas */
	Word fixedcolormap;	/* can be set to REDCOLORMAP, etc */
	Word colormap;		/* 0-3 for which color to draw player */
	pspdef_t psprites[NUMPSPRITES];	/* view sprites (gun, etc) */
	struct sector_s *lastsoundsector;	/* Pointer to sector where I last made a sound */
	Fixed forwardmove;		/* Motion ahead (- for reverse) */
	Fixed sidemove;			/* Motion to the side (- for left) */
	Fixed viewz;			/* focal origin above r.z */
	Fixed viewheight;		/* base height above floor for viewz */
	Fixed deltaviewheight;	/* squat speed */
	Fixed bob;				/* bounded/scaled total momentum */
	Fixed automapx,automapy;	/* X and Y coord on the auto map */
	angle_t	angleturn;		/* Speed of turning */
	Word turnheld;			/* For accelerative turning */
	Word damagecount;		/* Redness factor */
	Word bonuscount;		/* Goldness factor */
	Word powers[NUMPOWERS];		/* invinc and invis are tic counters */
	Word AutomapFlags;			/* Bit flags for options and automap */
	weapontype_t readyweapon;	/* Weapon being used */
	weapontype_t pendingweapon;	/* wp_nochange if not changing */
	playerstate_t playerstate;	/* Alive/dead... */
	Boolean	cards[NUMCARDS];	/* Keycards held */
	Boolean	backpack;		/* Got the backpack? */
	Boolean	attackdown;		/* Held the attack key if true */
	Boolean usedown;		/* Held the use button down if true */
	Boolean	weaponowned[NUMWEAPONS];	/* Do I own these weapons? */
	Boolean refire;			/* refired shots are less accurate */
} player_t;

typedef	struct sector_s {		/* Describe a playfield sector (Polygon) */
	Fixed floorheight;	/* Floor height */
	Fixed ceilingheight;	/* Top and bottom height */
	Word FloorPic;		/* Floor texture # */
	Word CeilingPic;	/* If ceilingpic==-1, draw sky */
	Word lightlevel;	/* Light override */
	Word special;		/* Special event number */
	Word tag;			/* Event tag */
	Word soundtraversed;	/* 0 = untraversed, 1,2 = sndlines -1 */
	mobj_t *soundtarget;	/* thing that made a sound (or null) */

	Word blockbox[BOXCOUNT];	/* mapblock bounding box for height changes */
	Fixed SoundX,SoundY;			/* For any sounds played by the sector */
	Word validcount;	/* if == validcount, already checked */
	mobj_t *thinglist;	/* list of mobjs in sector */
	void *specialdata;	/* Thinker struct for reversable actions */
	Word linecount;		/* Number of lines in polygon */
	struct line_s **lines;	/* [linecount] size */
} sector_t;

typedef struct {		/* Vector struct */
	Fixed x,y;			/* X,Y start of line */
	Fixed dx,dy;		/* Distance traveled */
} vector_t;

typedef struct {	/* Point in a map */
	Fixed x,y;		/* X and Y coord of dot */
} vertex_t;

typedef struct {	/* Data for a line side */
	Fixed textureoffset;	/* Column texture offset (X) */
	Fixed rowoffset;		/* Row texture offset (Y) */
	Word toptexture,bottomtexture,midtexture;	/* Wall textures */
	sector_t *sector;		/* Pointer to parent sector */
} side_t;

#define	ML_BLOCKING		1		/* Line blocks all movement */
#define	ML_BLOCKMONSTERS 2		/* Line blocks monster movement */	
#define	ML_TWOSIDED		4		/* This line has two sides */
#define	ML_DONTPEGTOP	8		/* Top texture is bottom anchored */
#define	ML_DONTPEGBOTTOM 16		/* Bottom texture is bottom anchored */
#define ML_SECRET		32	/* don't map as two sided: IT'S A SECRET! */
#define ML_SOUNDBLOCK	64	/* don't let sound cross two of these */
#define	ML_DONTDRAW		128	/* don't draw on the automap */
#define	ML_MAPPED		256	/* set if allready drawn in automap */

typedef struct line_s {
	vertex_t v1,v2;		/* X,Ys for the line ends */
	Word flags;			/* Bit flags (ML_) for states */
	Word special;		/* Event number */
	Word tag;			/* Event tag number */
	side_t *SidePtr[2];	/* Sidenum[1] will be NULL if one sided */
	Fixed bbox[BOXCOUNT];	/* Bounding box for quick clipping */
	slopetype_t	slopetype;	/* To aid move clipping */
	sector_t *frontsector;	/* Front line side sector */
	sector_t *backsector;	/* Back side sector (NULL if one sided) */
	Word validcount;		/* Prevent recursion flag */
	Word fineangle;			/* Index to get sine / cosine for sliding */
} line_t;

typedef struct seg_s {		/* Structure for a line segment */
	vertex_t v1,v2;			/* Source and dest points */
	angle_t angle;			/* Angle of the vector */
	Fixed offset;			/* Extra shape offset */
	side_t *sidedef;		/* Pointer to the connected side */
	line_t *linedef;		/* Pointer to the connected line */
	sector_t *frontsector;	/* Sector on the front side */
	sector_t *backsector;	/* NULL for one sided lines */
} seg_t;

typedef struct subsector_s {	/* Subsector structure */
	sector_t *sector;			/* Pointer to parent sector */
	Word numsublines;			/* Number of subsector lines */
	seg_t *firstline;			/* Pointer to the first line */
} subsector_t;

typedef struct {
	vector_t Line;		/* BSP partition line */
	Fixed bbox[2][BOXCOUNT]; /* Bounding box for each child */
	void *Children[2];		/* If low bit is set then it's a subsector */
} node_t;

typedef struct {		/* Describe all wall textures */
	Word width;			/* Width of the texture in pixels */
	Word height;		/* Height of the texture in pixels */
	void **data;		/* Handle to cached data to draw from */
} texture_t;

#define	AC_ADDFLOOR	1
#define	AC_ADDCEILING 2
#define	AC_TOPTEXTURE 4
#define	AC_BOTTOMTEXTURE 8
#define	AC_NEWCEILING 16
#define	AC_NEWFLOOR	32
#define	AC_ADDSKY 64
#define AC_TOPSIL 128
#define AC_BOTTOMSIL 256
#define AC_SOLIDSIL 512

typedef struct {		/* Describe a wall segment to be drawn */
	Word LeftX;			/* Leftmost x screen coord */
	Word RightX; 		/* Rightmost inclusive x coordinates */
	void **FloorPic;	/* Picture handle to floor shape */
	void **CeilingPic;	/* Picture handle to ceiling shape */
	Word WallActions;	/* Actions to perform for draw */

	int	t_topheight;	/* Describe the top texture */
	int	t_bottomheight;
	int	t_texturemid;
	texture_t *t_texture; /* Pointer to the top texture */

	int	b_topheight;	/* Describe the bottom texture */
	int	b_bottomheight;
	int	b_texturemid;
	texture_t *b_texture;		/* Pointer to the bottom texture */

	int	floorheight;
	int	floornewheight;

	int	ceilingheight;
	int	ceilingnewheight;

	Fixed LeftScale;	/* LeftX Scale */
	Fixed RightScale;	/* RightX scale */
	Fixed SmallScale;
	Fixed LargeScale;
	Byte *TopSil;		/* YClips for the top line */
	Byte *BottomSil;	/* YClips for the bottom line */
	Fixed ScaleStep;		/* Scale step factor */
	angle_t CenterAngle;	/* Center angle */
	Fixed offset;			/* Offset to the texture */
	Word distance;
	Word seglightlevel;
	seg_t *SegPtr;			/* Pointer to line segment for clipping */	
} viswall_t;

typedef struct {		/* Describe data on the status bar */
	spclface_e specialFace;	/* Which type of special face to make */
	Boolean	gotgibbed;			/* Got gibbed */
	Boolean tryopen[NUMCARDS];	/* Tried to open a card or skull door */
} stbar_t;

/* In Stdlib.c */

extern int abs(int val);

/* In Data.c */

extern ammotype_t WeaponAmmos[NUMWEAPONS];	/* Ammo for weapons */
extern Word maxammo[NUMAMMO];	/* Max ammo for ammo types */
extern Word PadAttack;	/* Joypad bit for attack */
extern Word PadUse;		/* Joypad bit for use */
extern Word PadSpeed;	/* Joypad bit for high speed */
extern Word ControlType;	/* Determine settings for PadAttack,Use,Speed */
extern Word TotalGameTicks;	/* Total number of ticks since game start */
extern Word ElapsedTime;	/* Ticks elapsed between frames */
extern Word MaxLevel;		/* Highest level selectable in menu (1-23) */
extern Word *DemoDataPtr;	/* Running pointer to demo data */
extern Word *DemoBuffer;	/* Pointer to demo data */
extern Word JoyPadButtons;	/* Current joypad */
extern Word PrevJoyPadButtons;	/* Previous joypad */
extern Word NewJoyPadButtons;	/* New joypad button downs */
extern skill_t StartSkill;	/* Default skill level */
extern Word StartMap;		/* Default map start */
extern void *BigNumFont;	/* Cached pointer to the big number font (rBIGNUMB) */
extern Word TotalKillsInLevel;		/* Number of monsters killed */
extern Word ItemsFoundInLevel;		/* Number of items found */
extern Word SecretsFoundInLevel;		/* Number of secrets discovered */
extern Word tx_texturelight;	/* Light value to pass to hardware */
extern Fixed lightsub;
extern Fixed lightcoef;
extern Fixed lightmin;
extern Fixed lightmax;
extern player_t players;	/* Current player stats */
extern gameaction_t gameaction;	/* Current game state */
extern skill_t gameskill;		/* Current skill level */
extern Word gamemap; 			/* Current game map # */
extern Word nextmap;			/* The map to go to after the stats */
extern Word ScreenSize;		/* Screen size to use */
extern Boolean LowDetail;		/* Use low detail mode */
extern Boolean DemoRecording;	/* True if demo is being recorded */
extern Boolean DemoPlayback;	/* True if demo is being played */
extern Boolean DoWipe;			/* True if I should do the DOOM wipe */

/* In Tables.c */

extern Fixed finetangent[4096];
extern Fixed *finecosine;
extern Fixed finesine[10240];
extern angle_t tantoangle[2049];
extern angle_t xtoviewangle[MAXSCREENWIDTH+1];
extern int viewangletox[FINEANGLES/4];
extern Word yslope[MAXSCREENHEIGHT];			/* 6.10 frac */
extern Word distscale[MAXSCREENWIDTH];		/* 1.15 frac */
extern Word IDivTable[8192];			/* 1.0 / 0-5500 for recipocal muls */	
extern Word CenterX;		/* Center X coord in fixed point */
extern Word CenterY;		/* Center Y coord in fixed point */
extern Word ScreenWidth;	/* Width of the view screen */
extern Word ScreenHeight;	/* Height of the view screen */
extern Fixed Stretch;		/* Stretch factor */
extern Fixed StretchWidth;	/* Stretch factor * ScreenWidth */
extern Word ScreenXOffset;	/* True X coord for projected screen */
extern Word ScreenYOffset;	/* True Y coord for projected screen */
extern LongWord GunXScale;	/* Scale factor for player's weapon for X */
extern LongWord GunYScale;	/* Scale factor for player's weapon for Y */
extern Fixed lightmins[256];	/* Minimum light factors */
extern Fixed lightsubs[256];	/* Light subtraction */
extern Fixed lightcoefs[256];	/* Light coeffecient */
extern Fixed planelightcoef[256];	/* Plane light coeffecient */

/* In Info.c */

extern state_t states[NUMSTATES];
extern mobjinfo_t mobjinfo[NUMMOBJTYPES];

/* In PSpr.c */

extern void LowerPlayerWeapon(player_t *player);
extern void A_WeaponReady(player_t *player,pspdef_t *psp);
extern void A_ReFire(player_t *player,pspdef_t *psp);
extern void A_Lower(player_t *player,pspdef_t *psp);
extern void A_Raise(player_t *player,pspdef_t *psp);
extern void A_GunFlash(player_t *player,pspdef_t *psp);
extern void A_Punch(player_t *player,pspdef_t *psp);
extern void A_Saw(player_t *player,pspdef_t *psp);
extern void A_FireMissile(player_t *player,pspdef_t *psp);
extern void A_FireBFG(player_t *player,pspdef_t *psp);
extern void A_FirePlasma(player_t *player,pspdef_t *psp);
extern void A_FirePistol(player_t *player,pspdef_t *psp);
extern void A_FireShotgun(player_t *player,pspdef_t *psp);
extern void A_CockSgun(player_t *player,pspdef_t *psp);
extern void A_FireCGun(player_t *player,pspdef_t *psp);
extern void A_Light0(player_t *player,pspdef_t *psp);
extern void A_Light1(player_t *player,pspdef_t *psp);
extern void A_Light2(player_t *player,pspdef_t *psp);
extern void A_BFGSpray(mobj_t *mo);
extern void A_BFGsound(player_t *player,pspdef_t *psp);
extern void SetupPSprites(player_t *curplayer);
extern void MovePSprites(player_t *curplayer);

/* Enemy.c */

extern void A_Look(mobj_t *actor);
extern void A_Chase(mobj_t *actor);
extern void A_FaceTarget(mobj_t *actor);
extern void A_PosAttack(mobj_t *actor);
extern void A_SPosAttack(mobj_t *actor);
extern void A_SpidRefire(mobj_t *actor);
extern void A_TroopAttack(mobj_t *actor);
extern void A_SargAttack(mobj_t *actor);
extern void A_HeadAttack(mobj_t *actor);
extern void A_CyberAttack(mobj_t *actor);
extern void A_BruisAttack(mobj_t *actor);
extern void A_SkullAttack(mobj_t *actor);
extern void A_Scream(mobj_t *actor);
extern void A_XScream(mobj_t *actor);
extern void A_Pain(mobj_t *actor);
extern void A_Fall(mobj_t *actor);
extern void A_Explode(mobj_t *thingy);
extern void A_BossDeath(mobj_t *mo);
extern void A_Hoof(mobj_t *mo);
extern void A_Metal(mobj_t *mo);
extern void L_MissileHit(mobj_t *mo,mobj_t *missilething);
extern void L_SkullBash(mobj_t *mo,mobj_t *skullthing);

/* In RMain.c */

extern viswall_t viswalls[MAXWALLCMDS];			/* Visible wall array */
extern viswall_t *lastwallcmd;					/* Pointer to free wall entry */
extern visplane_t visplanes[MAXVISPLANES];		/* Visible floor array */
extern visplane_t *lastvisplane;				/* Pointer to free floor entry */
extern vissprite_t	vissprites[MAXVISSPRITES];	/* Visible sprite array */
extern vissprite_t *vissprite_p;		/* Pointer to free sprite entry */
extern Byte openings[MAXOPENINGS];
extern Byte *lastopening;
extern Fixed viewx,viewy,viewz;		/* Camera x,y,z */
extern angle_t viewangle;				/* Camera angle */
extern Fixed viewcos,viewsin;			/* Camera sine,cosine from angle */
extern Word validcount;		/* Increment every time a check is made */
extern Word extralight;		/* bumped light from gun blasts */
extern angle_t clipangle;	/* Leftmost clipping angle */
extern angle_t doubleclipangle;	/* Doubled leftmost clipping angle */

extern void R_Init(void);
extern void R_Setup(void);
extern void R_RenderPlayerView(void);

/* In Sound.c */

extern void S_Clear(void);
extern void S_StartSound(Fixed *OriginXY,Word sound_id);
extern void S_StartSong(Word music_id,Boolean looping);
extern void S_StopSong(void);

/* In MObj.c */

extern void P_RemoveMobj(mobj_t *th);
extern Word SetMObjState(mobj_t *mobj,state_t *StatePtr);
extern void Sub1RandomTick(mobj_t *mobj);
extern void ExplodeMissile(mobj_t *mo);
extern mobj_t *SpawnMObj(Fixed x,Fixed y,Fixed z,mobjinfo_t *InfoPtr);
extern void P_SpawnPlayer(mapthing_t *mthing);
extern void SpawnMapThing(mapthing_t *mthing);
extern void P_SpawnPuff(Fixed x,Fixed y,Fixed z);
extern void P_SpawnBlood(Fixed x,Fixed y,Fixed z,Word damage);
extern void P_SpawnMissile(mobj_t *source,mobj_t *dest,mobjinfo_t *InfoPtr);
extern void SpawnPlayerMissile(mobj_t *source,mobjinfo_t *InfoPtr);

/* In DMain.c */

extern void AddToBox(Fixed *box,Fixed x,Fixed y);
extern Word MiniLoop(void(*start)(void),void(*stop)(void),
	Word(*ticker)(void),void(*drawer)(void));
extern void D_DoomMain(void);

/* In Tick.c */

extern Boolean Tick4;		/* True 4 times a second */
extern Boolean Tick2;		/* True 2 times a second */
extern Boolean Tick1;		/* True 1 time a second */
extern Boolean gamepaused;	/* True if the game is currently paused */
extern mobj_t mobjhead;		/* Head and tail of mobj list */

extern void InitThinkers(void);
extern void *AddThinker(void (*FuncProc)(),Word MemSize);
extern void RemoveThinker(void *thinker);
extern void ChangeThinkCode(void *thinker,void (*FuncProc)());
extern void RunThinkers(void);
extern Word P_Ticker(void);
extern void P_Drawer(void);
extern void P_Start(void);
extern void P_Stop(void);

/* In Map.c */

extern mobj_t *linetarget;	 	/* Object that was targeted */
extern mobj_t *tmthing;			/* mobj_t to be checked */
extern Fixed tmx,tmy;			/* Temp x,y for a position to be checked */
extern Boolean checkposonly; 	/* If true, just check the position, no actions */
extern mobj_t *shooter;			/* Source of a direct line shot */
extern angle_t attackangle;		/* Angle to target */
extern Fixed attackrange;		/* Range to target */
extern Fixed aimtopslope;		/* Range of slope to target weapon */
extern Fixed aimbottomslope;

extern Boolean P_CheckPosition(mobj_t *thing,Fixed x,Fixed y);
extern Boolean P_TryMove(mobj_t *thing,Fixed x,Fixed y);
extern void P_UseLines(player_t *player);
extern void RadiusAttack(mobj_t *spot,mobj_t *source,Word damage);
extern Fixed AimLineAttack(mobj_t *t1,angle_t angle,Fixed distance);
extern void LineAttack(mobj_t *t1,angle_t angle,Fixed distance,Fixed slope,Word damage);

/* In Inter.c */

extern void TouchSpecialThing(mobj_t *special,mobj_t *toucher);
extern void DamageMObj(mobj_t *target,mobj_t *inflictor,mobj_t *source,Word damage);

/* In MapUtl.c */

extern angle_t SlopeAngle(LongWord num,LongWord den);
extern angle_t PointToAngle(Fixed x1,Fixed y1,Fixed x2,Fixed y2);
extern Fixed PointToDist(Fixed x,Fixed y);
extern Fixed GetApproxDistance(Fixed dx,Fixed dy);
extern Word PointOnVectorSide(Fixed x,Fixed y,vector_t *line);
extern subsector_t *PointInSubsector(Fixed x,Fixed y);
extern void MakeVector(line_t *li,vector_t *dl);
extern Fixed InterceptVector(vector_t *v2,vector_t *v1);
extern Word LineOpening(line_t *linedef);
extern void UnsetThingPosition(mobj_t *thing);
extern void SetThingPosition(mobj_t *thing);
extern Word BlockLinesIterator(Word x,Word y,Word(*func)(line_t*));
extern Word BlockThingsIterator(Word x,Word y,Word(*func)(mobj_t*));

/* In Move.c */

extern Boolean trymove2;		/* Result from P_TryMove2 */
extern Boolean floatok;			/* if true, move would be ok if within tmfloorz - tmceilingz */
extern Fixed tmfloorz;		/* Current floor z for P_TryMove2 */
extern Fixed tmceilingz;		/* Current ceiling z for P_TryMove2 */
extern mobj_t *movething;		/* Either a skull/missile target or a special pickup */
extern line_t *blockline;		/* Might be a door that can be opened */

extern void P_TryMove2(void);
extern void PM_CheckPosition(void);
extern Boolean PM_BoxCrossLine(line_t *ld);
extern Boolean PIT_CheckLine(line_t *ld);
extern Word PIT_CheckThing(mobj_t *thing);

/* In Switch.c */

extern Word NumSwitches;		/* Number of switches * 2 */
extern Word SwitchList[];
extern void P_InitSwitchList(void);
extern void P_ChangeSwitchTexture(line_t *line,Boolean useAgain);
extern Boolean P_UseSpecialLine(mobj_t *thing,line_t *line);

/* In Game.c */

extern void G_DoLoadLevel(void);
extern void G_PlayerFinishLevel(void);
extern void G_PlayerReborn(void);
extern void G_DoReborn(void);
extern void G_ExitLevel(void);
extern void G_SecretExitLevel(void);
extern void G_InitNew(skill_t skill,Word map);
extern void G_RunGame(void);
extern Word G_PlayDemoPtr(Word *demo);
extern void G_RecordDemo(void);

/* In Floor.c */

extern result_e T_MovePlane(sector_t *sector,Fixed speed,
			Fixed dest,Boolean crush,Boolean Ceiling,int direction);
extern Boolean EV_DoFloor(line_t *line,floor_e floortype);
extern Boolean EV_BuildStairs(line_t *line);
extern Boolean EV_DoDonut(line_t *line);

/* In FMain.c */

extern void F_Start(void);
extern void F_Stop(void);
extern Word F_Ticker(void);
extern void F_Drawer(void);

/* In Spec.c */

extern Word NumFlatAnims;		/* Number of flat anims */
extern anim_t FlatAnims[];		/* Array of flat animations */

extern void P_InitPicAnims(void);
extern side_t *getSide(sector_t *sec,Word line,Word side);
extern sector_t *getSector(sector_t *sec,Word line,Word side);
extern Boolean twoSided(sector_t *sec,Word line);
extern sector_t *getNextSector(line_t *line,sector_t *sec);
extern Fixed P_FindLowestFloorSurrounding(sector_t *sec);
extern Fixed P_FindHighestFloorSurrounding(sector_t *sec);
extern Fixed P_FindNextHighestFloor(sector_t *sec,Fixed currentheight);
extern Fixed P_FindLowestCeilingSurrounding(sector_t *sec);
extern Fixed P_FindHighestCeilingSurrounding(sector_t *sec);
extern Word P_FindSectorFromLineTag(line_t *line,Word start);
extern Word P_FindMinSurroundingLight(sector_t *sector,Word max);
extern void P_CrossSpecialLine(line_t *line,mobj_t *thing);
extern void P_ShootSpecialLine(mobj_t *thing,line_t *line);
extern void PlayerInSpecialSector(player_t *player,sector_t *sector);
extern void P_UpdateSpecials(void);
extern void SpawnSpecials(void);
extern void PurgeLineSpecials(void);

/* In Ceilng.c */

extern Boolean EV_DoCeiling(line_t *line,ceiling_e type);
extern Boolean EV_CeilingCrushStop(line_t *line);
extern void ResetCeilings(void);

/* In Plats.c */

extern Boolean EV_DoPlat(line_t *line,plattype_e type,Word amount);
extern void EV_StopPlat(line_t *line);
extern void ResetPlats(void);

/* In Doors.c */

extern Boolean EV_DoDoor(line_t *line,vldoor_e type);
extern void EV_VerticalDoor(line_t *line,mobj_t *thing);
extern void P_SpawnDoorCloseIn30(sector_t *sec);
extern void P_SpawnDoorRaiseIn5Mins(sector_t *sec);

/* In Lights.c */

extern void P_SpawnLightFlash(sector_t *sector);
extern void P_SpawnStrobeFlash(sector_t *sector,Word fastOrSlow,Boolean inSync);
extern void EV_StartLightStrobing(line_t *line);
extern void EV_TurnTagLightsOff(line_t *line);
extern void EV_LightTurnOn(line_t *line,Word bright);
extern void P_SpawnGlowingLight(sector_t *sector);

/* In Setup.c */

extern seg_t *segs;				/* Pointer to array of loaded segs */
extern Word numsectors;			/* Number of sectors loaded */
extern sector_t *sectors;			/* Pointer to array of loaded sectors */
extern subsector_t *subsectors;	/* Pointer to array of loaded subsectors */
extern node_t *FirstBSPNode;	/* First BSP node */
extern Word numlines;		/* Number of lines loaded */
extern line_t *lines;		/* Pointer to array of loaded lines */
extern line_t ***BlockMapLines;	/* Pointer to line lists based on blockmap */
extern Word BlockMapWidth,BlockMapHeight;	/* Size of blockmap in blocks */
extern Fixed BlockMapOrgX,BlockMapOrgY;	/* Origin of block map */
extern mobj_t **BlockLinkPtr;		/* Starting link for thing chains */
extern Byte *RejectMatrix;			/* For fast sight rejection */
extern mapthing_t deathmatchstarts[10],*deathmatch_p;	/* Deathmatch starts */
extern mapthing_t playerstarts;	/* Starting position for players */

extern void SetupLevel(Word map);
extern void ReleaseMapMemory(void);
extern void P_Init(void);

/* In Telept.c */

extern Boolean EV_Teleport(line_t *line,mobj_t *thing);

/* In RData.c */

extern Word NumTextures;		/* Number of textures in the game */
extern Word FirstTexture;		/* First texture resource */
extern Word NumFlats;			/* Number of flats in the game */
extern Word FirstFlat;			/* Resource number to first flat texture */
extern texture_t *TextureInfo;	/* Array describing textures */
extern void ***FlatInfo;		/* Array describing flats */
extern texture_t **TextureTranslation; /* Indexs to textures for global animation */
extern void ***FlatTranslation;	/* Indexs to textures for global animation */
extern texture_t *SkyTexture;	/* Pointer to the sky texture */

extern void R_InitData(void);
extern void InitMathTables(void);

/* In AMMain.c */

extern void AM_Start(void);
extern void AM_Control(player_t *player);
extern void AM_Drawer(void);

/* In Shoot.c */

extern line_t *shootline;
extern mobj_t *shootmobj;
extern Fixed shootslope;					// between aimtop and aimbottom
extern Fixed shootx, shooty, shootz;		// location for puff/blood

extern void P_Shoot2(void);
extern Boolean PA_DoIntercept(void *value,Boolean isline,int frac);
extern Boolean PA_ShootLine(line_t *li,Fixed interceptfrac);
extern Boolean PA_ShootThing(mobj_t *th,Fixed interceptfrac);
extern Fixed PA_SightCrossLine(line_t *line);
extern Boolean PA_CrossSubsector(subsector_t *sub);

/* In Change.c */

extern Word ChangeSector(sector_t *sector,Word crunch);

/* In User.c */

extern void P_PlayerThink(player_t *player);

/* In Slide.c */

extern Fixed slidex, slidey;		/* the final position */
extern line_t *specialline;

extern void P_SlideMove(mobj_t *mo);
extern Fixed P_CompletableFrac(Fixed dx,Fixed dy);
extern int SL_PointOnSide(int x, int y);
extern Fixed SL_CrossFrac(void);
extern Boolean CheckLineEnds(void);
extern void ClipToLine(void);
extern Word SL_CheckLine(line_t *ld);
extern void SL_CheckSpecialLines(int x1, int y1, int x2, int y2);

/* In Sight.c */

extern Word CheckSight(mobj_t *t1,mobj_t *t2);

/* In Base.c */

extern void P_RunMobjBase(void);

/* In InMain.c */

#define PNPercent 1	/* Flags for PrintNumber, Percent sign appended? */
#define PNCenter 2	/* Use X as center and not left x */
#define PNRight 4	/* Use right justification */
extern void PrintBigFont(Word x,Word y,Byte *string);
extern Word GetBigStringWidth(Byte *string);
extern void PrintNumber(Word x,Word y,Word value,Word Flags);
extern void PrintBigFontCenter(Word x,Word y,Byte *String);
extern void IN_Start(void);
extern void IN_Stop(void);
extern Word IN_Ticker(void);
extern void IN_Drawer(void);

/* In OMain.c */

extern void O_Init(void);
extern void O_Control(player_t *player);
extern void O_Drawer(void);

/* In MMain.c */

extern void M_Start(void);
extern void M_Stop(void);
extern Word M_Ticker(void);
extern void M_Drawer(void);

/* In StMain.c */

extern stbar_t stbar;		/* Pass messages to the status bar */

extern void ST_Start(void);
extern void ST_Stop(void);
extern void ST_Ticker(void);
extern void ST_Drawer(void);

/* In Phase1.c */

extern Word SpriteTotal;		/* Total number of sprites to render */
extern Word *SortedSprites;		/* Pointer to array of words of sprites to render */
extern void BSP(void);

/* In Phase2.c */

extern void WallPrep(Word LeftX,Word RightX,seg_t *LineSeg,angle_t LineAngle);

/* In Phase6.c */

extern void SegCommands(void);

/* In Phase7.c */

extern Byte *PlaneSource;		/* Pointer to floor shape */
extern Fixed planey;		/* latched viewx / viewy for floor drawing */
extern Fixed basexscale,baseyscale;
extern void DrawVisPlane(visplane_t *PlanePtr);

/* In Phase8.c */

extern Word spropening[MAXSCREENWIDTH];		/* clipped range */
extern Word *SortWords(Word *Before,Word *After,Word Total);
extern void DrawVisSprite(vissprite_t *vis);
extern void DrawAllSprites(void);
extern void DrawWeapons(void);

/* In AppleIIgs.c, Threedo.c, IBM.c Etc... */

extern LongWord LastTics;		/* Time elapsed since last page flip */

extern void WritePrefsFile(void);
extern void ClearPrefsFile(void);
extern void ReadPrefsFile(void);
extern void UpdateAndPageFlip(void);
extern void DrawPlaque(Word RezNum);
extern void **LoadAResourceHandle(Word RezNum);
extern void DrawSkyLine(void);
extern void DrawWallColumn(Word y,Word colnum,Byte *Source,Word Run);
extern void DrawFloorColumn(Word ds_y,Word ds_x1,Word Count,LongWord xfrac,
	LongWord yfrac,Fixed ds_xstep,Fixed ds_ystep);
extern void DrawSpriteNoClip(vissprite_t *vis);
extern void DrawSpriteClip(Word x1,Word x2,vissprite_t *vis);
extern void DrawSpriteCenter(Word SpriteNum);
extern void EnableHardwareClipping(void);
extern void DisableHardwareClipping(void);
extern void DrawColors(void);
