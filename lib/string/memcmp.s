;
;Compare two buffers for equality
;
;int memcmp(char *Str1, char *Str2,size_t n);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


OrgStr1	RN	R0	;Pointer to the first "C" string
Str1	RN	R3	;Pointer to the first "C" string (Used)
Str2	RN	R1	;Pointer to the second "C" string
Len	RN	R2	;Number of bytes to match
Temp1	RN	R12	;Temp value for compare
Temp2	RN	R0	;Temp value for compare
		
	EXPORT memcmp

memcmp	CMP	Len,#0		;No memory to check?
	MOVLE	R0,#0		;Return a zero
	MOVLE PC,LR		;Leave!
	MOV	Str1,OrgStr1		;Copy first pointer into temp
Loop	LDRB	Temp1,[Str1],#1	;Get a byte from the first string
	LDRB	Temp2,[Str2],#1	;Compare to the second string
	SUBS	Temp2,Temp1,Temp2	;Subtract for a compare
	MOVNE	PC,LR		;Exit with result
	SUBS	Len,Len,#1		;End of the string?
	BNE	Loop		;Nope, keep going
	MOV	PC,LR		;Return with zero

	END
