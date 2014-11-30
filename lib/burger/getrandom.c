#include "Burger.h"
#include <string.h>

/**********************************

	Burger library module for random number generation

**********************************/

static Word RndSeed = 0;		/* Last random number generated */
static Word indexi = 16;		/* First array index */
static Word indexj = 4;			/* Second array index */
static Word RndArray[17] = {	/* Current array */
	1,1,2,3,5,8,13,21,54,75,129,204,323,527,850,1377,2227
};
static Word baseRndArray[17] = {	/* Default array */
	1,1,2,3,5,8,13,21,54,75,129,204,323,527,850,1377,2227
};

/**********************************

	Init the random number generator from a KNOWN state.
	This will allow games to record just the joystick
	movements and have random actions repeat for demo playback

**********************************/

void Randomize(void)
{
	memcpy((char *)RndArray,(char *)baseRndArray,sizeof(RndArray));
	RndSeed = 0;
	indexi = 16;
	indexj = 4;
	GetRandom(255);
}

/**********************************

	Init the random number generator with an "Anything goes"
	policy so programs will power up in an unknown state.
	Do NOT use this if you wish your title to have recordable demos.

	I use the formula that the tick timer runs at a constant time
	base but the machine in question does not. As a result. The
	number of times GetRandom is called is anyone's guess.

**********************************/

void HardwareRandomize(void)
{
	LongWord TickMark;
	TickMark = ReadTick();	/* Get a current tick mark */
	do {
		GetRandom(0xFFFF);	/* Discard a number from the stream */
	} while (ReadTick()==TickMark);	/* Same time? */
}

/**********************************

	Get a random number. Return a number between 0-MaxVal inclusive

**********************************/

Word GetRandom(Word MaxVal)
{
	Word NewVal;
	Word i,j;

	if (!MaxVal) {	/* Zero? */
		return 0;	/* Don't bother */
	}
	++MaxVal;		/* +1 to force inclusive */
	i = indexi;		/* Cache indexs */
	j = indexj;
	NewVal = RndArray[i] + RndArray[j];	/* Get the delta seed */
	RndArray[i] = NewVal;	/* Save in array */
	NewVal += RndSeed;		/* Add to the base seed */
	RndSeed = NewVal;		/* Save the seed */
	--i;		/* Advance the indexs */
	--j;
	if (i&0x8000) {
		i = 16;
	}
	if (j&0x8000) {
		j =16;
	}
	indexi = i;		/* Save in statics */
	indexj = j;
	NewVal&=0xFFFFU;		/* Make sure they are shorts! */
	MaxVal&=0xFFFFU;
	if (!MaxVal) {		/* No adjustment? */
		return NewVal;	/* Return the random value */
	}
	return ((NewVal*MaxVal)>>16U);	/* Adjust the 16 bit value to range */
}



