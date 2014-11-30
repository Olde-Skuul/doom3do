;
;Compare two "C" strings for equality
;
;int strcmp(char *Str1, char *Str2);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


OrgStr1	RN	R0	;Pointer to the first "C" string
Str1	RN	R2	;Pointer to the first "C" string (Used)
Str2	RN	R1	;Pointer to the second "C" string
Temp1	RN	R3	;Temp value for compare
Temp2	RN	R0	;Temp value for compare
		
	EXPORT strcmp

strcmp	MOV	Str1,OrgStr1		;Copy first pointer into temp
Loop	LDRB	Temp1,[Str1],#1	;Get a byte from the first string
	LDRB	Temp2,[Str2],#1	;Compare to the second string
	SUBS	Temp2,Temp1,Temp2	;Subtract for a compare
	MOVNE	PC,LR		;Exit with result
	MOVS	Temp1,Temp1		;End of the string?
	BNE	Loop		;Nope, keep going
	MOV	PC,LR		;Return with zero

	END
