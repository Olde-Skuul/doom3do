;
;Search for char in a "C" string
;
;char * strchr(char *Src, int Char);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


Src	RN	R0	;Pointer to the dest "C" string
Char	RN	R1	;String to concatinate with
Temp	RN	R2	;Temp value for compare
		
	EXPORT strchr

strchr	AND	Char,Char,#&FF	;Make SURE it's a byte!
Loop1	LDRB	Temp,[Src],#1	;Get a byte from the source string
	CMP	Temp,Char		;A match?
	BEQ	Hit		;Leave!
	CMP	Temp,#0		;No dice?
	BNE	Loop1		;Keep looking!
	MOV	Src,Temp		;Return the zero
	MOV	PC,LR		;Go back!

Hit	SUB	Src,Src,#1		;Undo the post increment
	MOV	PC,LR		;Go back!
	END
