##########################################################################
#   File:       Burger.make
#
#	Contains:	make file for the Burger library
#
#	History:
#	6/8/93		Burger	Began scripting
##########################################################################

#####################################
#		Symbol definitions
#####################################
Program			=	Burger
CC				=	armcc
ASM				=	armasm
LINK			=	armlink

#####################################
#	Default compiler options
#####################################
COptions		= -bi -zps1 -za1 -wn -ff -fa -i "{3DOIncludes}"

SOptions		= -bi -i "{3DOIncludes}"

LOptions		= -aif -r -b 0x00

#####################################
#		Object files
#####################################	
					
# NOTE: Add object files here...

OBJECTS	=	"DLZSS.o" "ShowPic.o" "ShowPicPtr.o" "Show3DOLogo.o" ¶
			"YTable.o" "VideoPointer.o" "VideoWidth.o" "ScreenWidth.o" "ScreenHeight.o" ¶
			"InitYTable.o" "GetAPixel.o" "SetAPixel.o" "DrawMShape.o" ¶
			"DrawShape.o" "EraseMBShape.o" "DrawARect.o" "GetShapeWidth.o" "GetShapeHeight.o" ¶
			"GetShapeIndexPtr.o" "ClearTheScreen.o" "DrawRezShape.o" "DrawRezCenterShape.o" ¶
			"FontData.o" "FontSaveState.o" "FontRestoreState.o" ¶
			"DrawAString.o" "WidthAString.o" "SetFontXY.o" "DrawAChar.o" ¶
			"InstallAFont.o" "FontUseZero.o" "FontUseMask.o" "FontSetColor.o" ¶
			"CurrentPalette.o" "SetAPalette.o" "SetAPalettePtr.o" "SetAPalettePtr2.o" ¶
			"FadeTo.o" "FadeToPtr.o" "FadeToBlack.o" ¶
			"KeyArray.o" "InterceptKey.o" "MyGetch.o" "MyKbhit.o" "FlushKeys.o" ¶
			"GetAKey.o" "WaitAKey.o" "ReadJoyButtons.o" "ReadMouseButtons.o" ¶
			"ReadMouseAbs.o" "DetectMouse.o" "MousePresent.o" ¶
			"InitMemory.o" "AllocAHandle.o" "DeallocAHandle.o" "ReallocAHandle.o" ¶
			"FindAHandle.o" "LockAHandle.o" "UnlockAHandle.o" "CompactHandles.o" ¶
			"GetAHandleSize.o" "GetTotalFreeMem.o" "SetHandlePurgeFlag.o" ¶
			"AllocAPointer.o" "DeallocAPointer.o" "DumpHandles.o" "PurgeHandles.o" ¶
			"InitResource.o" "LoadAResource.o" "LoadAResource2.o" ¶
			"DetachResource.o" "DetachResource2.o" "LoadAResourceHandle.o" "LoadAResourceHandle2.o" ¶
			"ReleaseAResource.o" "ReleaseAResource2.o" "KillAResource.o" "KillAResource2.o" ¶
			"GetRandom.o" "SwapUShort.o" "SwapShort.o" "SwapLong.o" "Fatal.o" "NonFatal.o" ¶
			"SetErrBombFlag.o" "ErrorMsg.o" "LongWordToAsc.o" "LongToAsc.o" "BombFlag.o" ¶
			"LastTick.o" "WaitTick.o" "WaitTicks.o" "WaitTicksEvent.o" "WaitEvent.o" "ReadTick.o" ¶
			"ShowMCGA.o" "WaitJoyRelease.o" ¶
			"WaitJoyPress.o" "KeyModifiers.o" "SystemState.o" ¶
			"TestMShape.o" "TestMBShape.o" "KilledSong.o" "Music.o" ¶
			"Abs.o" "Exit2.o" "Atexit.o" "RezFileName.o" "Halt.o" "SaveAFile.o" 
					
#####################################
#	Default build rules
#			
#####################################
All				Ä	{Program}

.o			Ä	.c Burger.h
	{CC} {COptions} -o {TargDir}{Default}.o {DepDir}{Default}.c

.o			Ä	.s
	{ASM} {SOptions} -o {TargDir}{Default}.o {DepDir}{Default}.s


#####################################
#	Target build rules
#####################################
{Program}		ÄÄ	{Program}.make {OBJECTS}
	Duplicate -y Burger.h {3DOIncludes}Burger.h
	armlib -c Burger.lib {OBJECTS}
	armlib -o Burger.lib
	Duplicate -y Burger.lib {3DOLibs}Burger.lib
