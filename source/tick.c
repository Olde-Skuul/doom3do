#include "Doom.h"
#include <String.h>

typedef struct thinker_s {
	struct thinker_s *next,*prev;
	void (*function)();
} thinker_t;

mobj_t mobjhead;	/* Head and tail of mobj list */
Boolean Tick4;		/* True 4 times a second */
Boolean Tick2;		/* True 2 times a second */
Boolean Tick1;		/* True 1 time a second */
Boolean gamepaused;	/* True if the game is currently paused */

static Word TimeMark1;	/* Timer for ticks */
static Word TimeMark2;	/* Timer for ticks */
static Word TimeMark4;	/* Timer for ticks */
static thinker_t thinkercap;	/* Both the head and tail of the thinker list */
static Boolean refreshdrawn;		/* Used to refresh "Paused" */

/**********************************

	Remove a thinker structure from the linked list and from
	memory.

**********************************/

static void RemoveMeThink(thinker_t *Current)
{
	thinker_t *Next;
	thinker_t *Prev;
	--Current;			/* Index to the REAL pointer */
	Next = Current->next;
	Prev = Current->prev;
	Next->prev = Prev;	/* Unlink it */
	Prev->next = Next;
	DeallocAPointer(Current);		/* Release the memory */
}

/**********************************

    Init the mobj list and the thinker list
    I use a circular linked list with a mobjhead and thinkercap
    structure used only as an anchor point. These header structures
    are not used in actual gameplay, only for a referance point.
    If this is the first time through (!thinkercap.next) then just
    init the vars, else follow the memory chain and dispose of the
    memory.

**********************************/

void InitThinkers(void)
{
    ResetPlats();				/* Reset the platforms */
    ResetCeilings();			/* Reset the ceilings */
	if (mobjhead.next) {		/* Initialized before? */
		mobj_t *m,*NextM;
		m=mobjhead.next;
		while (m!=&mobjhead) {	/* Any player object */
			NextM = m->next;		/* Get the next record */
			DeallocAPointer(m);	/* Delete the object from the list */
			m=NextM;
		}
	}
	if (thinkercap.next) {
		thinker_t *t,*NextT;
		t = thinkercap.next;
		while (t!=&thinkercap) {	/* Is there a think struct here? */
			NextT = t->next;
			DeallocAPointer(t);		/* Delete it from memory */
			t = NextT;
		}
	}
	thinkercap.prev = thinkercap.next  = &thinkercap;	/* Loop around */
	mobjhead.next = mobjhead.prev = &mobjhead;		/* Loop around */
}

/**********************************

	Adds a new thinker at the end of the list, this way,
	I can get my code executed before the think execute routine
	finishes.

**********************************/

void *AddThinker(void (*FuncProc)(),Word MemSize)
{
	thinker_t *Prev;
	thinker_t *thinker;
	
	MemSize += sizeof(thinker_t);		/* Add size for the thinker prestructure */
	thinker = (thinker_t *)AllocAPointer(MemSize);	/* Get memory */
	memset(thinker,0,MemSize);		/* Blank it out */
	Prev = thinkercap.prev;		/* Get the last thinker in the list */
	thinker->next = &thinkercap;	/* Mark as last entry in list */
	thinker->prev = Prev;	/* Set prev link to final entry */
	thinker->function = FuncProc;
	Prev->next = thinker;	/* Next link to the new link */
	thinkercap.prev = thinker;	/* Mark the reverse link */
	return thinker+1;		/* Index AFTER the thinker structure */
}

/**********************************

	Deallocation is lazy -- it will not actually be freed until its
	thinking turn comes up

**********************************/

void RemoveThinker(void *thinker)
{
	thinker = ((thinker_t *)thinker)-1;		/* Index to the true structure */
	((thinker_t*)thinker)->function = RemoveMeThink;	/* Delete the structure on the next pass */
}

/**********************************

	Modify a thinker's code function
	
**********************************/

void ChangeThinkCode(void *thinker,void (*FuncProc)())
{
	thinker = ((thinker_t *)thinker)-1;
	((thinker_t *)thinker)->function = FuncProc;
}

/**********************************

	Execute all the think logic in the object list

**********************************/

void RunThinkers (void)
{
	thinker_t *currentthinker;
	thinker_t *NextThinker;

	currentthinker = thinkercap.next;		/* Get the first entry */
	while (currentthinker != &thinkercap) {	/* Looped back? */
		/* Get the next entry (In case of change or removal) */
		NextThinker = currentthinker->next;
		if (currentthinker->function) {		/* Is the function ptr ok? */
			/* Call the think logic */
			/* Note : It may be a call to a think remove routine! */
			currentthinker->function(currentthinker+1);
		}
		currentthinker = NextThinker;	/* Go to the next entry */
	}
}

/**********************************

	Check the cheat keys... :)

**********************************/

static void CheckCheats(void)
{
	if ((NewJoyPadButtons & PadStart) && !(players.AutomapFlags & AF_OPTIONSACTIVE)) {		/* Pressed pause? */
		if (gamepaused || !(JoyPadButtons&PadUse)) {
			gamepaused ^= 1;		/* Toggle the pause flag */
			if (gamepaused) {
				PauseMusic();
			} else {
				ResumeMusic();
			}
		}
	}
}

/**********************************

	Code that gets executed every game frame

**********************************/

Word P_Ticker(void)
{
	player_t *pl;

	/* wait for refresh to latch all needed data before */
	/* running the next tick */

	gameaction = ga_nothing;		/* Game in progress */
	Tick1 = FALSE;			/* Reset the flags */
	Tick2 = FALSE;
	Tick4 = FALSE;

	TimeMark1+=ElapsedTime;	/* Timer for ticks */
	TimeMark2+=ElapsedTime;
	TimeMark4+=ElapsedTime;

	if (TimeMark1>=TICKSPERSEC) {	/* Now see if the time has passed... */
		TimeMark1-=TICKSPERSEC;
		Tick1 = TRUE;
	}
	if (TimeMark2>=(TICKSPERSEC/2)) {
		TimeMark2-=(TICKSPERSEC/2);
		Tick2 = TRUE;
	}
	if (TimeMark4>=(TICKSPERSEC/4)) {
		TimeMark4-=(TICKSPERSEC/4);
		Tick4 = TRUE;
	}

	CheckCheats();		/* Handle pause and cheats */

/* Do option screen processing and control reading */

	if (gamepaused) {		/* If in pause mode, then don't do any game logic */
		return gameaction;
	}

/* Run player actions */

	pl = &players;
	
	if (pl->playerstate == PST_REBORN) {	/* Restart player? */
		G_DoReborn();		/* Poof!! */
	}
	AM_Control(pl);		/* Handle automap controls */
	O_Control(pl);		/* Handle option controls */
	P_PlayerThink(pl);	/* Process player in the game */
		
	if (!(players.AutomapFlags & AF_OPTIONSACTIVE)) {
		RunThinkers();		/* Handle logic for doors, walls etc... */
		P_RunMobjBase();	/* Handle critter think logic */
	}
	P_UpdateSpecials();	/* Handle wall and floor animations */
	ST_Ticker();		/* Update status bar */
	return gameaction;	/* may have been set to ga_died, ga_completed, */
						/* or ga_secretexit */
}

/**********************************

	Draw current display

**********************************/

void P_Drawer(void)
{
	if (gamepaused && refreshdrawn) {
		DrawPlaque(rPAUSED);		/* Draw paused */
	} else if (players.AutomapFlags & AF_OPTIONSACTIVE) {
		R_RenderPlayerView();	/* Render the 3D view */
		ST_Drawer();			/* Draw the status bar */
		O_Drawer();			/* Draw the console handler */
		refreshdrawn = FALSE;
	} else if (players.AutomapFlags & AF_ACTIVE) {
		AM_Drawer();		/* Draw the automap */
		ST_Drawer();		/* Draw the status bar */
		UpdateAndPageFlip();	/* Update and page flip */
		refreshdrawn = TRUE;
	} else {
		R_RenderPlayerView();	/* Render the 3D view */
		ST_Drawer();			/* Draw the status bar */
		UpdateAndPageFlip();
		refreshdrawn = TRUE;
	}
}

/**********************************

	Start a game

**********************************/

void P_Start(void)
{
	TimeMark1 = 0;		/* Init the static timers */
	TimeMark2 = 0;
	TimeMark4 = 0;
	players.AutomapFlags &= AF_GODMODE;		/* No automapping specials (but allow godmode) */
	AM_Start();			/* Start the automap system */
	ST_Start();			/* Init the status bar this level */
	G_DoLoadLevel();	/* Load a level into memory */
	Randomize();		/* Reset the random number generator */
	S_StartSong(Song_e1m1-1+gamemap,TRUE);
}

/**********************************

	Shut down a game

**********************************/

void P_Stop(void)
{
	S_StopSong();
	ST_Stop();				/* Release the status bar memory */
	ReleaseMapMemory();		/* Release all the map's memory */
	PurgeLineSpecials();	/* Release the memory for line specials */
}
