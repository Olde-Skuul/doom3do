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
Program			=	IntMath
CC				=	armcc
ASM				=	armasm
LINK			=	armlink

#####################################
#	Default compiler options
#####################################
COptions		= -zps1 -za1 -wn -ff -i "{3DOIncludes}"

SOptions		= -bi -i "{3DOIncludes}"

LOptions		= -aif -r -b 0x00

#####################################
#		Object files
#####################################	
					
# NOTE: Add object files here...

OBJECTS	=	"IM_FixMul.o" "IM_FixDiv.o" "IM_LoWord.o" "IM_HiWord.o" "IM_Long2Fix.o" ¶
	"IM_Multiply.o" "IM_SDivide.o" "IM_UDivide.o" "IM_Dec2Int.o"

#####################################
#	Default build rules
#			
#####################################
All				Ä	{Program}

.o			Ä	.c
	{CC} {COptions} -o {TargDir}{Default}.o {DepDir}{Default}.c

.o			Ä	.s
	{ASM} {SOptions} -o {TargDir}{Default}.o {DepDir}{Default}.s


#####################################
#	Target build rules
#####################################
{Program}		ÄÄ	{Program}.make {OBJECTS}
	Duplicate -y IntMath.h {3DOIncludes}IntMath.h
	armlib -c IntMath.lib {OBJECTS}
	armlib -o IntMath.lib
	Duplicate -y IntMath.lib {3DOLibs}IntMath.lib
