;
;Compare two "C" strings for equality but convert all case to lower case
;so that case doesn't matter
;
;int stricmp(char *Str1, char *Str2);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

__S	EQU	1	; whitespace
__P	EQU	2	; punctuation
__B	EQU	4	; blank
__L	EQU	8	; lower case letter
__U	EQU	16	; upper case letter
__N	EQU	32	; (decimal) digit
__C	EQU	64 	; control chars
__X	EQU	128	; A-F and a-f


OrgStr1	RN	R0	;Pointer to the first "C" string (Destroyed)
Str1	RN	R2	;Pointer to the first "C" string (Temp)
Str2	RN	R1	;Pointer to the second "C" string
Temp1	RN	R3	;Temp value for compare
Temp2	RN	R0	;Temp value for compare (Return value)
Field	RN	R12	;Temp bit field for test
CType	RN	LR	;Pointer to the bit field
SP	RN	R13	;Stack pointer
		
	EXPORT stricmp
	IMPORT __ctype

stricmp	STMDB	SP!,{LR}	;Push my temps onto the stack

	MOV	Str1,OrgStr1	;Copy first pointer into temp
	LDR	CType,CtypePtr	;Get a pointer to CType

Loop	LDRB	Temp1,[Str1],#1	;Get a byte from the first string
	LDRB	Field,[CType,Temp1]	;Get the bit mask
	TST	Field,#__U	;Upper case?
	ORRNE	Temp1,Temp1,#&20	;Convert to lower case

	LDRB	Temp2,[Str2],#1	;Get a byte from the second
	LDRB	Field,[CType,Temp2]	;Get the bit mask
	TST	Field,#__U	;Upper case?
	ORRNE	Temp2,Temp2,#&20	;Convert to lower case

	SUBS	Temp2,Temp1,Temp2	;Subtract for a compare
	LDMNEIA SP!,{PC}	;Exit with result (If not equal!)
	MOVS	Temp2,Temp1	;End of the string? (Clear R0!)
	BNE	Loop	;Nope, keep going
	LDMIA	SP!,{PC}	;Return with zero then

CtypePtr	DCD	__ctype		;Pointer to the CType array

	END
