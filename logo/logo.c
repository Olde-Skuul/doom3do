#include "Portfolio.h"
#include "Utils3DO.h"

static Item vbitem,spitem;
static ScreenContext TheScreen;
static int32 NumFrameBufPages;

#define SCREENS	1

/**********************************

	Clear the current screen to black

**********************************/

static void ClearTheScreen(void)
{
	WaitVBL(vbitem,1);
	SetVRAMPages(spitem,TheScreen.sc_Bitmaps[0]->bm_Buffer,0,NumFrameBufPages,-1);
} 

/**********************************

	Init the 3DO tools

**********************************/

static void Init3DO(void)
{
	TheScreen.sc_nScreens = SCREENS;	/* How many screens do I want? */

	CreateBasicDisplay(&TheScreen,DI_TYPE_DEFAULT,SCREENS);	/* Open the graphics folio */
	OpenMathFolio();			/* Init the math folio (Required) */
	EnableHAVG(TheScreen.sc_Screens[0]);	/* Enable anti-aliasing */
	EnableVAVG(TheScreen.sc_Screens[0]);
	NumFrameBufPages=TheScreen.sc_nFrameBufferPages;	/* Init my var */
	vbitem=GetVBLIOReq();		/* Get IO request item */
	spitem=GetVRAMIOReq();		/* Get VRAM IO request item */
}

/**********************************

	Load and display the 3DO logo

**********************************/

static void Draw3DO(void)
{
	CCB *six;

	six=LoadCel("3DO.logo.cel",MEMTYPE_CEL);	/* Load in the cel */
	ClearTheScreen();							/* Clear the screen */
	DisplayScreen(TheScreen.sc_Screens[0],0);	/* Show the video buffer */
	CenterRectCelInDisplay(six);				/* Center the 3DO logo */
	DrawCels(TheScreen.sc_BitmapItems[0],six);	/* Draw the logo */
	UnloadCel(six);								/* Release the memory */
}

/**********************************

	Main program

**********************************/

int main(int32 argc, char *argv[])
{
	Init3DO();		/* Init 3DO tools */
	Draw3DO();			/* Draw the 3DO Logo */
	WaitVBL(vbitem,3*60);	/* Wait 3 seconds */
	FadeToBlack(&TheScreen,60);	/* Fade out the screen */
	return 0;			/* Exit */
}
