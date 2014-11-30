;
;Copy a "C" string but only up to MaxLen in length
;
;char * strncpy(char *Dest, char *Src,size_t n);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


Dest	RN	R0	;Original pointer to the dest buffer
Src	RN	R1	;Pointer to the "C" string
MaxLen	RN	R2	;Maximum number of bytes to move
Str	RN	R3	;Pointer to the dest buffer
Temp	RN	R12	;Temp value for compare

	EXPORT strncpy

strncpy	CMP	MaxLen,#0		;No memory to move at all?
	MOVEQ	PC,LR		;Leave now
	MOV	Str,Dest		;Copy first pointer into temp
Loop	LDRB	Temp,[Src],#1	;Get a byte from the first string
	STRB	Temp,[Str],#1	;Store to the second string
	SUBS	MaxLen,MaxLen,#1	;No more to move?
	MOVEQ	PC,LR		;Exit then...
	CMP	Temp,#0		;End of string?
	BNE	Loop		;Nope, keep going

PadLoop	STRB	Temp,[Str],#1	;Fill the rest with zero's
	SUBS	MaxLen,MaxLen,#1	;More left?
	BNE	PadLoop		;Keep going!
	MOV	PC,LR		;Return
	END
