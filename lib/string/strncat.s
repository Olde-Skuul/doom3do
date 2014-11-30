;
;Concatanate two "C" strings together with a maximum length
;
;char * strncat(char *Dest, char * Src,size_t n);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


Dest	RN	R0	;Pointer to the dest "C" string
Src	RN	R1	;String to concatinate with
MaxLen	RN	R2 	;Maximum buffer size
Str	RN	R3	;Running pointer
Temp	RN	R12	;Temp value for compare
		
	EXPORT strncat
strncat	CMP	MaxLen,#0		;No data to transfer?
	MOVEQ	PC,LR		;Leave now!

	MOV	Str,Dest		;Save the pointer to the string in scratch
Loop1	LDRB	Temp,[Str],#1	;Get a byte from the dest string
	CMP	Temp,#0
	BNE	Loop1		;Wait for a null
	SUB	Str,Str,#1		;Remove the postindex

Loop2	LDRB	Temp,[Src],#1	;Get a byte from the "C" string
	STRB	Temp,[Str],#1	;Copy it over
	CMP	Temp,#0		;Null?
	MOVEQ	PC,LR		;Get lost then
	SUBS	MaxLen,MaxLen,#1	;Remove one from the max...
	BNE	Loop2		;Still more space left?
	STRB	MaxLen,[Str]		;Write the final null
	MOV	PC,LR		;Bye now!
	END
