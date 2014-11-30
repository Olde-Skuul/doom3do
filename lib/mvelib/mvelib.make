##########################################################################
#   File:       MveLib.make
#
#	Contains:	make file for the Video library
#
#	History:
#	2/22/94		Burger	Began scripting
##########################################################################

#####################################
#		Symbol definitions
#####################################
Program			=	MveLib
CC				=	armcc
ASM				=	armasm
LINK			=	armlink

#####################################
#	Default compiler options
#####################################
COptions		= -zps1 -za1 -w -ff -i "{3DOIncludes}"

SOptions		= -bi -i "{3DOIncludes}"

LOptions		= -aif -r -b 0x00

#####################################
#		Object files
#####################################	
					
# NOTE: Add object files here...

OBJECTS			=	"nfAdvance.o" "nfHiColorDecomp.o"
					
#####################################
#	Default build rules
#####################################
All				Ä	{Program}

.c.o			Ä	.c
	{CC} {COptions} -o {TargDir}{Default}.c.o {DepDir}{Default}.c

.o				Ä	.s
	{ASM} {SOptions} -o {TargDir}{Default}.o {DepDir}{Default}.s


#####################################
#	Target build rules
#####################################
{Program}		ÄÄ	{Program}.make {OBJECTS}
	Duplicate -y MveLib.h {3DOIncludes}MveLib.h
	armlib -c MveLib.lib {OBJECTS}
	armlib -o MveLib.lib
	Duplicate -y MveLib.lib {3DOLibs}MveLib.lib
