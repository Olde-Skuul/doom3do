##########################################################################
#   File:       Doom.make
#
#	Contains:	make file for Doom for the 3DO
#
#	History:
#	8/4/95		Burger	Began scripting
##########################################################################

#####################################
#		Symbol definitions
#####################################
Program			=	Doom

ObjectDir		=	:ObjectCode:
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
LIBS			=	"{3DOLibs}Burger.lib" ¶
					"{3DOLibs}Lib3DO.lib"	¶
					"{3DOLibs}filesystem.lib"	¶
					"{3DOLibs}graphics.lib"	¶
					"{3DOLibs}audio.lib"		¶
					"{3DOLibs}music.lib"		¶
					"{3DOLibs}operamath.lib"	¶
					"{3DOLibs}input.lib"		¶
					"{3DOLibs}String.lib" ¶
					"{3DOLibs}clib.lib"		¶
					"{3DOLibs}swi.lib" ¶
					"{3DOLibs}IntMath.lib"

# NOTE: Add object files here...
OBJECTS			=	"{ObjectDir}AMMain.c.o" ¶
					"{ObjectDir}Base.c.o" ¶
					"{ObjectDir}Ceilng.c.o" ¶
					"{ObjectDir}Change.c.o" ¶
					"{ObjectDir}Data.c.o" ¶
					"{ObjectDir}DMain.c.o" ¶
					"{ObjectDir}Doors.c.o" ¶
					"{ObjectDir}Enemy.c.o" ¶
					"{ObjectDir}Floor.c.o" ¶
					"{ObjectDir}FMain.c.o" ¶
					"{ObjectDir}Game.c.o" ¶
					"{ObjectDir}Info.c.o" ¶
					"{ObjectDir}InMain.c.o" ¶
					"{ObjectDir}Inter.c.o" ¶
					"{ObjectDir}Lights.c.o" ¶
					"{ObjectDir}Map.c.o" ¶
					"{ObjectDir}MapUtl.c.o" ¶
					"{ObjectDir}MObj.c.o" ¶
					"{ObjectDir}Move.c.o" ¶
					"{ObjectDir}MMain.c.o" ¶
					"{ObjectDir}OMain.c.o" ¶
					"{ObjectDir}Plats.c.o" ¶
					"{ObjectDir}PSpr.c.o" ¶
					"{ObjectDir}RData.c.o" ¶
					"{ObjectDir}RMain.c.o" ¶
					"{ObjectDir}StMain.c.o" ¶
					"{ObjectDir}Sound.c.o" ¶
					"{ObjectDir}Tables.c.o" ¶
					"{ObjectDir}Tick.c.o" ¶
					"{ObjectDir}Shoot.c.o" ¶
					"{ObjectDir}Sight.c.o" ¶
					"{ObjectDir}Slide.c.o" ¶
					"{ObjectDir}Spec.c.o" ¶
					"{ObjectDir}Switch.c.o" ¶
					"{ObjectDir}Setup.c.o" ¶
					"{ObjectDir}Telept.c.o" ¶
					"{ObjectDir}User.c.o" ¶
					"{ObjectDir}Phase1.c.o" ¶
					"{ObjectDir}Phase2.c.o" ¶
					"{ObjectDir}Phase6.c.o" ¶
					"{ObjectDir}Phase7.c.o" ¶
					"{ObjectDir}Phase8.c.o" ¶
					"{ObjectDir}BlitAsm.s.o" ¶
					"{ObjectDir}ThreeDO.c.o"
					

#####################################
#	Default build rules
#####################################
All				Ä	{Program}

{ObjectDir}		Ä	:

.c.o			Ä	.c doom.h
	{CC} {COptions} -o {TargDir}{Default}.c.o {DepDir}{Default}.c

.s.o			Ä	.s
	{ASM} {SOptions} -o {TargDir}{Default}.s.o {DepDir}{Default}.s


#####################################
#	Target build rules
#####################################
{Program}		ÄÄ	{Program}.make {OBJECTS}
	{LINK}	{LOptions}					¶
			-o {Program}				¶
			"{3DOLibs}cstartup.o"		¶
			{OBJECTS}					¶
			{LIBS}
	modbin {Program} -stack 10000
	SetFile {Program} -c 'EaDJ' -t 'PROJ'
	stripaif {Program} -o {Program}
	duplicate -y {Program} :Game3DO:remote:
