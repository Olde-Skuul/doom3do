#####################################
#		Symbol definitions
#####################################
Program			=	IDLogo
ObjectDir		=	:Objects:
CC				=	armcc
ASM				=	armasm
LINK			=	armlink

DESTDIR			=	BurgerDrive:Doom3DO:Game3DO:remote:

#####################################
#	Default compiler options
#####################################

COptions		= -bi -zps1 -za1 -ff -wn -i "{3DOIncludes}"

SOptions		= -bi -g -i "{3DOIncludes}"

LOptions		= -aif -r -b 0x00

#####################################
#		Object files
#####################################
LIBS			=	#"{3DOLibs}Subscriber.lib"	¶
					"{3DOLibs}Lib3DO.lib"		¶
					"{3DOLibs}operamath.lib"	¶
					"{3DOLibs}graphics.lib"	¶
					"{3DOLibs}audio.lib"		¶
					"{3DOLibs}filesystem.lib"		¶
					"{3DOLibs}input.lib"		¶
					#"{3DOLibs}DS.lib"	¶
					#"{3DOLibs}DataAcq.lib"	¶
					"{3DOLibs}clib.lib"		¶
					"{3DOLibs}swi.lib"

# NOTE: Add object files here...
OBJECTS			=	:Objects:idlogo.c.o

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
{Program}		Ä	 {OBJECTS}
	{LINK}	{LOptions}				¶
			-o {Program}		¶
			"{3DOLibs}cstartup.o"	¶
			{OBJECTS}				¶
			{LIBS}
	SetFile {Program} -c 'EaDJ' -t 'PROJ'
	modbin {Program} -stack 4000
	duplicate -y {Program} '{DESTDIR}{Program}'
