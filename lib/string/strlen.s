;
;Return the length of a "C" string
;
;int strlen(char * Src);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


Src	RN	R0	;Pointer to the "C" string
OrgSrc	RN	R1	;Copy of the source pointer
Temp	RN	R2	;Temp value for compare
		
	EXPORT strlen
strlen	ADD	OrgSrc,Src,#1	;Save the pointer to the string
Loop	LDRB	Temp,[Src],#1	;Get a byte from the "C" string
	CMP	Temp,#0		;Null?
	BNE	Loop		;Loop for more
	SUB	R0,Src,OrgSrc	;Place the string length in R0
	MOV	PC,LR		;Return to caller
	END
