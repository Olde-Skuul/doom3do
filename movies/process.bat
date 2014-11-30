sftostream -cs 4096 -i ADILogo.aiff -o ADILogo.Audio
movietostream AdiLogo.moov -o AdiLogo.FILM -b 20480
weaver -o AdiLogo.cine < AdiLogoweavescript

sftostream -cs 4096 -i EALogo.aiff -o EALogo.Audio
movietostream EALogo.moov -o EALogo.FILM -b 20480
weaver -o EALogo.cine < EALogoweavescript


dumpstream -stats -i EALogo.cine
"EALogo.cine" contains 5% filler
		block size = 20480 bytes
		average filler block size = 1042 bytes


"ADiLogo.cine" contains 4% filler
		block size = 20480 bytes
		average filler block size = 1008 bytes



