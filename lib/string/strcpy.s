;
;Copy a "C" string
;
;char * strcpy(char *Dest, char *Src);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


Dest	RN	R0	;Original pointer to the dest buffer
Src	RN	R1	;Pointer to the "C" string
Str	RN	R2	;Pointer to the dest buffer
Temp	RN	R3	;Temp value for compare

	EXPORT strcpy

strcpy	MOV	Str,Dest		;Copy first pointer into temp
Loop	LDRB	Temp,[Src],#1	;Get a byte from the first string
	STRB	Temp,[Str],#1	;Store to the second string
	CMP	Temp,#0		;End of string?
	BNE	Loop
	MOV	PC,LR		;Return with zero
	END
