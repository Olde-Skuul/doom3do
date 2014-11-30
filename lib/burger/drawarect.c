#include "Burger.h"
#include <Graphics.h>

/**********************************

	Draw a solid colored rect on the screen
	
**********************************/

static LongWord MyCelData;	/* Color to draw with CCB */

static CCB MyCCB2 = {		/* This is for rects */
	CCB_LAST|CCB_SPABS|CCB_LDSIZE|CCB_LDPRS|
	CCB_LDPPMP|CCB_CCBPRE|CCB_YOXY|CCB_ACW|CCB_ACCW|
	CCB_ACE|CCB_BGND|CCB_NOBLK,	/* ccb_flags */
	0x00000000,		/* ccb_NextPtr */
	(CelData *) &MyCelData,	/* ccb_CelData */
	0x00000000,	/* ccb_PIPPtr */
	0x00000000,	/* ccb_X */
	0x00000000,	/* ccb_Y */
	0x00100000,	/* ccb_HDX */
	0x00000000,	/* ccb_HDY */
	0x00000000,	/* ccb_VDX */
	0x00010000,	/* ccb_VDY */
	0x00000000,	/* ccb_DDX */
	0x00000000,	/* ccb_DDY */
	0x1F001F00,	/* ccb_PIXC */
	0x40000016,	/* ccb_PRE0 */
	0x03FF1000	/* ccb_PRE1 Low bit is X control*/
};

void DrawARect(Word x,Word y,Word Width,Word Height,Word Color)
{
	MyCelData = Color<<16;		/* Adjust for BIG endian long to short */
	MyCCB2.ccb_XPos = (x<<16);	/* Set the topmost X */
	MyCCB2.ccb_YPos = (y<<16);	/* Set the topmost Y */
	MyCCB2.ccb_HDX = (Width<<20);	/* Set the width factor */
	MyCCB2.ccb_VDY = (Height<<16);	/* Set the height factor */
	DrawCels(VideoItem,&MyCCB2);	/* Draw the rect */
}
