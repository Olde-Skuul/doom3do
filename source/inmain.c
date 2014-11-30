#include "Doom.h"
#include <String.h>

#define	KVALX	232
#define	KVALY	70
#define	IVALX	232
#define	IVALY	100
#define	SVALX	231
#define	SVALY	130
#define INTERTIME (TICKSPERSEC/30)

enum {		/* Intermission shape group */
	KillShape,
	ItemsShape,
	SecretsShape
};

/**********************************

	Here are the names of each level

**********************************/

static Byte *mapnames[] = {
	(Byte *)"Hangar",
	(Byte *)"Nuclear Plant",
	(Byte *)"Toxin Refinery",
	(Byte *)"Command Control",
	(Byte *)"Phobos Lab",
	(Byte *)"Central Processing",
	(Byte *)"Computer Station",
	(Byte *)"Phobos Anomaly",
	(Byte *)"Deimos Anomaly",
	(Byte *)"Containment Area",
	(Byte *)"Refinery",
	(Byte *)"Deimos Lab",
	(Byte *)"Command Center",
	(Byte *)"Halls of the Damned",
	(Byte *)"Spawning Vats",
	(Byte *)"Tower of Babel",
	(Byte *)"Hell Keep",
	(Byte *)"Pandemonium",
	(Byte *)"House of Pain",
	(Byte *)"Unholy Cathedral",
	(Byte *)"Mt. Erebus",
	(Byte *)"Limbo",
	(Byte *)"Dis",
	(Byte *)"Military Base"
//	(Byte *)"Fortress of Mystery",
//	(Byte *)"Warrens"
};

static Byte Finished[] = "Finished";
static Byte Entering[] = "Entering";

static Word killpercent;		/* Percent to attain */
static Word itempercent;
static Word secretpercent;
static Word killvalue,itemvalue,secretvalue;	/* Displayed percent value */
static Word INDelay;			/* Delay before next inc */
static Word BangCount;			/* Delay for gunshot sound */

/**********************************

	Print a string using the large font.
	I only load the large ASCII font if it is needed.

**********************************/

void PrintBigFont(Word x,Word y,Byte *string)
{
	Word y2,c;
	void *ucharx;
	void *Current;

	c = string[0];		/* Get the first char */
   if (!c) {			/* No string to print? */
    	return;			/* Exit now */
	}
	ucharx = 0;			/* Assume ASCII font is NOT loaded */
    do {
    	++string;		/* Place here so "continue" will work */
    	y2 = y;			/* Assume normal y coord */
		Current = BigNumFont;	/* Assume numeric font */
		if (c >= '0' && c<='9') {
			c-= '0';	
		} else if (c=='%') {		/* Percent */
			c = 10;
		} else if (c=='-') {		/* Minus */
			c = 11;
		} else {
			Current = ucharx;	/* Assume I use the ASCII set */
			if (c >= 'A' && c <= 'Z') {	/* Upper case? */
				c-='A';
			} else if (c >= 'a' && c <= 'z') {
				c -= ('a'-26);		/* Index to lower case text */
				y2+=3;
			} else if (c=='.') {		/* Period */
				c = 52;
				y2+=3;
			} else if (c=='!') {	/* Exclaimation point */
				c = 53;
				y2+=3;
			} else {		/* Hmmm, not supported! */
				x+=6;		/* Fake space */
				continue;
			}
		}
		if (!Current) {		/* Do I need the ASCII set? */
			ucharx = LoadAResource(rCHARSET);	/* Make sure I have the text font */
			Current = ucharx;
		}
		Current = GetShapeIndexPtr(Current,c);	/* Get the shape pointer */
		DrawMShape(x,y2,Current);		/* Draw the char */
		x+=GetShapeWidth(Current)+1;	/* Get the width to tab */
	} while ((c = string[0])!=0);		/* Next index */
	if (ucharx) {						/* Did I load the ASCII font? */
		ReleaseAResource(rCHARSET);		/* Release the ASCII font */
	}
}

/**********************************

	Return the width of an ASCII string in pixels
	using the large font.

**********************************/

Word GetBigStringWidth(Byte *string)
{
	Word c,Width;
	void *ucharx;
	void *Current;

	c = string[0];		/* Get a char */
	if (!c) {			/* No string to print? */
    	return 0;
	}
	ucharx = 0;	/* Only load in the ASCII set if I really need it */
    Width = 0;
    do {
    	++string;
		Current = BigNumFont;	/* Assume numeric font */
		if (c >= '0' && c<='9') {
			c-= '0';	
		} else if (c=='%') {		/* Percent */
			c = 10;
		} else if (c=='-') {		/* Minus */
			c = 11;
		} else {
			Current = ucharx;	/* Assume I use the ASCII set */
			if (c >= 'A' && c <= 'Z') {	/* Upper case? */
				c-='A';
			} else if (c >= 'a' && c <= 'z') {
				c -= ('a'-26);		/* Index to lower case text */
			} else if (c=='.') {		/* Period */
				c = 52;
			} else if (c=='!') {	/* Exclaimation point */
				c = 53;
			} else {		/* Hmmm, not supported! */
				Width+=6;		/* Fake space */
				continue;
			}
		}
		if (!Current) {		/* Do I need ucharx? */
			ucharx = LoadAResource(rCHARSET);	/* Load it in */
			Current = ucharx;		/* Set the pointer */
		}
		Current = GetShapeIndexPtr(Current,c);	/* Get the shape pointer */
		Width+=GetShapeWidth(Current)+1;		/* Get the width to tab */
	} while ((c = string[0])!=0);		/* Next index */
	if (ucharx) {		/* Did I load in the ASCII font? */
		ReleaseAResource(rCHARSET);			/* Release the text font */
	}
	return Width;
}

/**********************************

	Draws a number, this number may be appended with
	a percent sign and or centered upon the x coord.
	I use flags PNPercent and PNCenter.

**********************************/

void PrintNumber(Word x,Word y,Word value,Word Flags)
{
	Byte v[16];		/* Buffer for text string */

	LongWordToAscii(value,v);		/* Convert to string */
	value = strlen((char *)v);		/* Get the length in chars */
	if (Flags&PNPercent) {			/* Append a percent sign? */
		v[value] = '%';				/* Append it */
		++value;
		v[value] = 0;				/* Make sure it's zero terminated */
	}
	if (Flags&PNCenter) {			/* Center it? */
		PrintBigFontCenter(x,y,v);
		return;
	}
	if (Flags&PNRight) {		/* Right justified? */
		x-=GetBigStringWidth(v);
	}
	PrintBigFont(x,y,v);	/* Print the string */
}

/**********************************
	
	Print an ascii string centered on the x coord

**********************************/

void PrintBigFontCenter(Word x,Word y,Byte *String)
{
	x-=(GetBigStringWidth(String)/2);
	PrintBigFont(x,y,String);
}

/**********************************
	
	Init the intermission data

**********************************/

void IN_Start(void)
{
	INDelay = 0;
	BangCount = 0;
	killvalue = itemvalue = secretvalue = 0;	/* All values shown are zero */
	killpercent = itempercent = secretpercent = 100;	/* Init in case of divide by zero */
	if (TotalKillsInLevel) {			/* Prevent divide by zeros */
		killpercent = (players.killcount * 100) / TotalKillsInLevel;
	}
	if (ItemsFoundInLevel) {
		itempercent = (players.itemcount * 100) / ItemsFoundInLevel;
	}
	if (SecretsFoundInLevel) {
		secretpercent = (players.secretcount * 100) / SecretsFoundInLevel;
	}
	S_StartSong(Song_intermission,TRUE);		/* Begin the music */
}

/**********************************
	
	Exit the intermission

**********************************/

void IN_Stop(void)
{
	S_StopSong();		/* Kill the music */
}

/**********************************
	
	Exit the intermission

**********************************/

Word IN_Ticker(void)
{
	Word Bang;
	if (TotalGameTicks < (TICKSPERSEC/2)) {	/* Initial wait before I begin */
		return ga_nothing;		/* Do nothing */
	}

	if (NewJoyPadButtons & (PadA|PadB|PadC)) {	/* Pressed a button? */
	 	killvalue = killpercent;		/* Set to maximums */
		itemvalue = itempercent;
		secretvalue = secretpercent;
		return ga_died;		/* Exit after drawing */
	}

	INDelay+=ElapsedTime;
	if (INDelay>=INTERTIME) {
		Bang = FALSE;
		INDelay-=INTERTIME;
		if (killvalue < killpercent) {		/* Is it too high? */
			++killvalue;
			Bang = TRUE;
		}
		if (itemvalue < itempercent) {
			++itemvalue;
			Bang = TRUE;
		}
		if (secretvalue < secretpercent) {
			++secretvalue;
			Bang = TRUE;
		}
		if (Bang) {
			++BangCount;
			if (!(BangCount&3)) {
				S_StartSound(0,sfx_pistol);
			}
		}
	}
	return ga_nothing;		/* Still here! */
}

/**********************************
	
	Draw the intermission screen
	
**********************************/

void IN_Drawer(void)
{
	void *IntermisShapes;		/* Cached pointer */
	
	DrawRezShape(0,0,rBACKGRNDBROWN);		/* Load and draw the skulls */
	
	IntermisShapes = LoadAResource(rINTERMIS);		/* Load the intermission shapes */
	PrintBigFontCenter(160,10,mapnames[gamemap-1]);	/* Print the current map name */
	PrintBigFontCenter(160,34,Finished);			/* Print "Finished" */
	if (nextmap != 23) {
		PrintBigFontCenter(160,162,Entering);
		PrintBigFontCenter(160,182,mapnames[nextmap-1]);
	}
	DrawMShape(71,KVALY,GetShapeIndexPtr(IntermisShapes,KillShape));	/* Draw the shapes */
	DrawMShape(65,IVALY,GetShapeIndexPtr(IntermisShapes,ItemsShape));
	DrawMShape(27,SVALY,GetShapeIndexPtr(IntermisShapes,SecretsShape));

	PrintNumber(KVALX,KVALY,killvalue,PNPercent|PNRight);	/* Print the numbers */
	PrintNumber(IVALX,IVALY,itemvalue,PNPercent|PNRight);
	PrintNumber(SVALX,SVALY,secretvalue,PNPercent|PNRight);
	ReleaseAResource(rINTERMIS);
	UpdateAndPageFlip();		/* Show the screen */
}
