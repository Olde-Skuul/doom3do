;
;Convert a "C" string to upper case.
;
;char * strupr(char * Src);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


Src	RN	R0	;Pointer to the "C" string
Str	RN	R1	;Copy of the source pointer
Temp	RN	R2	;Temp value for compare
		
	EXPORT strupr
strupr	MOV	Str,Src		;Save the pointer to the string in scratch
Loop	LDRB	Temp,[Str],#1	;Get a byte from the "C" string
	CMP	Temp,#0		;Null?
	MOVEQ	PC,LR		;Exit if at the end
	CMP	Temp,#'a'		;Lower case?
	BCC	Loop		;Nope
	CMP	Temp,#'z'+1		;Still lower case?
	BCS	Loop
	AND	Temp,Temp,#&DF	;Convert to upper case
	STRB	Temp,[Str,#-1]	;Save in the string
	B	Loop		;Jump back
	END
