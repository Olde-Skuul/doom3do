;
;Search for char in a range of memory
;
;void *memchr(void *Src, int Char,size_t n);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|


Src	RN	R0	;Pointer to the dest string
Char	RN	R1	;String to concatinate with
Len	RN	R2	;Size of the buffer to check
Temp	RN	R3	;Temp value for compare
		
	EXPORT memchr

memchr	CMP	Len,#0	;No memory to search?
	BLE	NoDice	;Exit
	AND	Char,Char,#&FF	;Make SURE it's a byte!
Loop1	LDRB	Temp,[Src],#1	;Get a byte from the source string
	CMP	Temp,Char	;A match?
	BEQ	Hit	;Leave!
	SUBS	Len,Len,#1	;No more memory to scan?
	BNE	Loop1	;Keep looking!
NoDice	MOV	Src,#0	;Return a NULL
	MOV	PC,LR	;Go back!

Hit	SUB	Src,Src,#1	;Undo the post increment
	MOV	PC,LR	;Go back!
	END
