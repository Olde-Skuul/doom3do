;
;Compare two "C" strings for equality but stop at MaxLen
;
;int strncmp(char *Str1, char *Str2,size_t n);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


OrgStr1	RN	R0	;Pointer to the first "C" string
Str1	RN	R12	;Pointer to the first "C" string (Used)
Str2	RN	R1	;Pointer to the second "C" string
MaxLen	RN	R2	;Maximum number of bytes to check
Temp1	RN	R3	;Temp value for compare
Temp2	RN	R0	;Temp value for compate
		
	EXPORT strncmp

strncmp	MOV	Str1,OrgStr1		;Copy first pointer into temp
	MOVS	Temp2,MaxLen		;Place length in return value
	MOVEQ	PC,LR		;Return with zero if there is no length

Loop	LDRB	Temp1,[Str1],#1	;Get a byte from the first string
	LDRB	Temp2,[Str2],#1	;Compare to the second string
	SUBS	Temp2,Temp1,Temp2	;Subtract for a compare
	MOVNE	PC,LR		;Exit with result
	MOVS	Temp2,Temp1		;End of the string?
	MOVEQ	PC,LR		;Exit equal...
	SUBS  MaxLen,MaxLen,#1	;Remove one from the count
	BNE	Loop		;Still more to test?
	MOV	Temp2,#0		;Return zippo!
	MOV	PC,LR		;Return with zero

	END
