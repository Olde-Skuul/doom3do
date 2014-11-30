;
;Convert a "C" String into an unsigned long value
;
;unsigned long Strtoul(char * Src,char ** LastPtr,int Base);

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

ERANGE	EQU	34	; Error for the range

Src	RN	R0	;Pointer to the "C" String to convert
LastPtr	RN	R1	;Copy of the source pointer
Base	RN	R2	;Temp value for compare
Str	RN	R3	;Work pointer
CType	RN	R4	;Pointer to CType table
Temp	RN	R5	;Temp ascii char
Field	RN	R12	;CType bit field
Flags	RN	LR	;Shall I negate the answer? (Yes = TRUE)
Value	RN	R0	;Running total
SP	RN	R13	;Stack pointer

NegateFlag	EQU	1
FoundFlag	EQU	2


	EXPORT strtoul
	IMPORT __ctype
	IMPORT errno

strtoul	STMDB	SP!,{R4-R5,LR}	;Push my temps onto the stack
	MOV	Str,Src		;Save the pointer to the String in scratch
	CMP	LastPtr,#0		;if LastPtr in non-null then
	STRNE Str,[LastPtr]	;initialize it to Str

	LDR	CType,CtypePtr	;Get the pointer to the CType record
	MOV	Flags,#0		;Assume no negation or chars converted
	MOV	Value,#0		;No value to return yet

;
; Skip any leading whitespace
;

NoPtr1	LDRB	Temp,[Str],#1	;Skip the white space
	LDRB	Field,[CType,Temp]
	TST	Field,#__S
	BNE	NoPtr1

;
; Deduce the Base
;

	CMP	Temp,#'+'		;Is there a leading plus?
	BEQ	Conv		;Don't set the Str back then!
	CMP	Temp,#'-'		;Negative?
	ORREQ	Flags,Flags,#NegateFlag	;Set to negate the answer
	SUBNE	Str,Str,#1		;Nope, undo the auto increment!

Conv	CMP	Base,#0		;if the Base is zero then
	BNE	ChkHex
	MOV	Base,#10		;assume Base 10
	LDRB	Temp,[Str]		;if the first char is 0 then
	CMP	Temp,#'0'
	BNE	Loop0
	MOV	Base,#8		;assume Base 8
	LDRB	Temp,[Str,#1]!	;if the second char is 'X' or 'x' then
	AND	Temp,Temp,#&DF
	CMP	Temp,#'X'
	BNE	Loop		;Go to the conversion loop
	MOV	Base,#16		;Base 16
	B	LoopX		;Fetch the next char

ChkHex	LDRB	Temp,[Str]		;if the first two chars are 0x or 0X then
	CMP	Temp,#'0'
	BNE	Loop0
	LDRB	Temp,[Str,#1]!	;Accept the zero anyways...
	AND	Temp,Temp,#&DF	;Mask for lowercase X
	CMP	Temp,#'X'		;X?
	BNE	Loop
	CMP	Base,#16		;make sure the Base is 16
	BNE	returnERANGE


;
; Convert the number
;

LoopX	LDRB	Temp,[Str,#1]!	;get a (possible) digit
Loop	CMP	Temp,#'0'		;branch if it is not a digit
Loop0	BCC	EndConv
	CMP	Temp,#'9'+1		;branch if it is a numeric digit
	ANDCC	Temp,Temp,#&0F	;convert digit to value (0-9)
	BCC	GotAZ		;Process...
	AND	Temp,Temp,#&DF	;convert lowercase to uppercase
	CMP	Temp,#'A'		;branch if it is not a digit
	BCC	EndConv
	CMP	Temp,#'Z'+1		;branch if it is not a digit
	BCS	EndConv
	SUB	Temp,Temp,#'A'-10	;convert "alpha" digit to value
GotAZ	CMP	Temp,Base		;branch if the digit is too big
	BCS	EndConv		;Oops! Out of range!
	ORR	Flags,Flags,#FoundFlag	;note that we have found a number
	MOVS	Field,Value,LSR #16	;Save the highest 8 bits in temp
	BIC	Value,Value,Field,ASL #16	;Clear out the highest word for precision
	MLA	Value,Base,Value,Temp	;Mul by base and add the digit
	BEQ	LoopX			;Field == 0?
	MUL	Field,Base,Field		;Only mul if there's something here!
	CMP	Field,#&10000		;Overflow?
	BCS	TooHigh			;Yikes!
	ADDS	Value,Value,Field,LSL #16	;Blend the numbers
	BCC	LoopX		;Fetch the next value if not
TooHigh	MOV	Value,#-1		;Set to maximum signed value!

;
; flag an error
;

returnERANGE MOV	Temp,#ERANGE		;errno = ERANGE
	LDR	CType,ErrPtr		;Get pointer to errno
	STR	Temp,[CType]		;Save the error code
	LDMIA SP!,{R4-R5,PC}	;Return to caller with answer

;
; End the conversion and return the result
;

EndConv	TST	Flags,#FoundFlag	;If no digits were found, flag the error
	BEQ	returnERANGE
	CMP	LastPtr,#0		;if LastPtr in non-null then
	STRNE	Str,[LastPtr]	;write the pointer back
	TST	Flags,#NegateFlag	;Should I negate the answer?
	RSBNE	Value,Value,#0	;Negate the answer
	LDMIA SP!,{R4-R5,PC}	;Return to caller with answer

ErrPtr	DCD	errno		;Pointer to the error variable
CtypePtr	DCD	__ctype		;Pointer to the CType array

	END
