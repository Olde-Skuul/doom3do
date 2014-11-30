;
;Fill a "C" string with a constant char
;
;char * strset(char *Dest, int c);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


Dest	RN	R0	;Original pointer to the dest buffer
Char	RN	R1	;Pointer to the "C" string
Str	RN	R2	;Temp pointer
Temp	RN	R3	;Temp value for compare

	EXPORT strset

strset	LDRB	Temp,[Dest]		;Get a byte from the first string
	CMP	Temp,#0		;Null string?
	MOVEQ	PC,LR		;Exit now!
	MOV	Str,Dest		;Save the pointer
Loop	STRB	Char,[Str],#1	;Store the fill char
	LDRB	Temp,[Str]		;Get a byte from the first string
	CMP	Temp,#0		;At the end?
	BNE	Loop		;Keep going...
	MOV	PC,LR		;Return with zero
	END
