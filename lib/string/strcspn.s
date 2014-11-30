;
; Search for the occurance of
;Reverse the letters in a "C" string
;
;char * strcspn(char *Str,char *Key);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


DestPtr	RN	R0	;Pointer to the string to search
KeyPtr	RN	R1	;Pointer to the key string
Str	RN	R2	;Running source pointer
Str2	RN	R3	;Running key pointer
Temp1	RN	R12	;Value in the first string
Temp2	RN	LR	;Temp value for compare
SP	RN	R13

	EXPORT strcspn

strcspn	STR	LR,[SP,#-4]!		;Save the return value on the stack
	MOV	Str,DestPtr		;Copy the starting pointer

TryAgain	LDRB	Temp1,[Str],#1	;End of the string?
	CMP	Temp1,#0
	BEQ	BugOut		;Bug out!
	MOV	Str2,KeyPtr		;Init the key pointer
More	LDRB	Temp2,[Str2],#1	;Get a char from the stream
	CMP	Temp2,#0		;End of the stream?
	BEQ	TryAgain		;Not a match!
	CMP	Temp2,Temp1		;Is the char in the stream?
	BNE	More		;Nope!
	SUB	R0,Str,DestPtr	;How many bytes down is it?
	SUB	R0,R0,#1		;-1 for auto increment
	LDR	PC,[SP],#4		;Return

BugOut	MOV	R0,#-1		;Return bogus
	LDR	PC,[SP],#4		;Exit

	END
