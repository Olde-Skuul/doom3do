;
;Reverse the letters in a "C" string
;
;char * strrev(char *Dest);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


Dest	RN	R0	;Original pointer to the dest buffer
EndStr	RN	R1	;Pointer to the end of the string
Str	RN	R2	;Temp pointer
Temp1	RN	R3	;Temp value for swap and end string check
Temp2	RN	R12	;Temp value for swap

	EXPORT strrev

strrev	LDRB	Temp1,[Dest]		;Is this a null string?
	CMP	Temp1,#0
	MOVEQ	PC,LR		;Bug out!
	MOV	Str,Dest		;Save the pointer
	ADD	EndStr,Dest,#1	;End of the string pointer

EndLoop	LDRB	Temp1,[EndStr],#1	;Scan to the end of the string
	CMP	Temp1,#0
	BNE	EndLoop

Loop	LDRB	Temp1,[Str]		;Get the first byte of the string
	LDRB	Temp2,[EndStr,#-1]	;Get the last
	STRB	Temp2,[Str],#1	;Swap 'em
	STRB	Temp1,[EndStr,#-1]!
	CMP	Str,EndStr		;Did the pointers cross?
	BCC	Loop		;Nope, keep on going...
	MOV	PC,LR		;Return with zero
	END
