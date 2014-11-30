;
;Convert a "C" string to lower case.
;
;char * strlwr(char * Src);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


Src	RN	R0	;Pointer to the "C" string
Str	RN	R1	;Copy of the source pointer
Temp	RN	R2	;Temp value for compare
		
	EXPORT strlwr
strlwr	MOV	Str,Src		;Save the pointer to the string in scratch
Loop	LDRB	Temp,[Str],#1	;Get a byte from the "C" string
	CMP	Temp,#0		;Null?
	MOVEQ	PC,LR		;Exit if at the end
	CMP	Temp,#'A'		;Upper case?
	BCC	Loop		;Nope
	CMP	Temp,#'Z'+1		;Still upper case?
	BCS	Loop
	ORR	Temp,Temp,#&20	;Convert to lower case
	STRB	Temp,[Str,#-1]	;Save in the string
	B	Loop		;Jump back
	END
