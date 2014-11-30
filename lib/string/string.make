##########################################################################
#   File:       Burger.make
#
#	Contains:	make file for the String.h library
#
#	History:
#	6/10/93		Burger	Began scripting
##########################################################################

#####################################
#		Symbol definitions
#####################################
Program			=	String
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

OBJECTS			=	"memchr.o" "memcmp.o" "memmove.o" "memset.o" ¶
					"strcat.o" "strchr.o" "strcmp.o" "strcpy.o" "strcspn.o" "stricmp.o" ¶
					"strlen.o" "strlwr.o" "strncat.o" "strncmp.o" "strncpy.o" ¶
					"strrev.o" "strset.o" "strtoul.o" "strtol.o" "strupr.o" ¶
					
					
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
	armlib -c String.lib {OBJECTS}
	armlib -o String.lib
	Duplicate -y String.lib {3DOLibs}String.lib
	Duplicate -y String.h {3DOIncludes}String.h
