;
;Concatanate two "C" strings together
;
;char * strcat(char *Dest, char * Src);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


Dest	RN	R0	;Pointer to the dest "C" string
Src	RN	R1	;String to concatinate with
Str	RN	R2	;Running pointer
Temp	RN	R3	;Temp value for compare
		
	EXPORT strcat
strcat	MOV	Str,Dest		;Save the pointer to the string in scratch
Loop1	LDRB	Temp,[Str],#1	;Get a byte from the dest string
	CMP	Temp,#0
	BNE	Loop1		;Wait for a null
	SUB	Str,Str,#1		;Remove the postindex
Loop2	LDRB	Temp,[Src],#1	;Get a byte from the "C" string
	STRB	Temp,[Str],#1	;Copy it over
	CMP	Temp,#0		;Null?
	BNE	Loop2		;Keep going
	MOV	PC,LR		;Exit if at the end
	END
