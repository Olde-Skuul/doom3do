##########################################################################
#   File:       OOTW.make
#
#	Contains:	make file for Out Of This World for the 3DO
#
#	History:
#	1/18/93		Burger	Began scripting
##########################################################################

#####################################
#		Symbol definitions
#####################################
Program			=	TEST
DebugFlag		=	1
ObjectDir		=	:
CC				=	armcc
ASM				=	armasm
LINK			=	armlink

#####################################
#	Default compiler options
#####################################
COptions		= -zps1 -za1 -wn -ff -i "{3DOIncludes}"

SOptions		= -bi -g -i "{3DOIncludes}"

LOptions		= -aif -r -b 0x00

#####################################
#		Object files
#####################################
LIBS			=	"{3DOLibs}Burger.lib"		¶
					"{3DOLibs}Lib3DO.lib"		¶
					"{3DOLibs}filesystem.lib"		¶
					"{3DOLibs}graphics.lib"	¶
					"{3DOLibs}audio.lib"		¶
					"{3DOLibs}music.lib"		¶
					"{3DOLibs}operamath.lib"	¶
					"{3DOLibs}input.lib"		¶
					"{3DOLibs}String.lib" ¶
					"{3DOLibs}clib.lib"		¶
					"{3DOLibs}swi.lib"

# NOTE: Add object files here...
OBJECTS			=	"{ObjectDir}Test.c.o"


#####################################
#	Default build rules
#####################################
All				Ä	{Program}

{ObjectDir}		Ä	:

.c.o			Ä	.c
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
	modbin Test -stack 10000
	SetFile {Program} -c 'EaDJ' -t 'PROJ'
	duplicate -y Test {3DOFolder}3DO_os:1p4:remote:
