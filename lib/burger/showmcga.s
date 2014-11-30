;
; Convert a 320 X 200 MCGA screen to 3DO
; VideoPtr = a pointer to the 3DO video memory
; SourcePtr = a pointer to a 320X200 MCGA screen
; ColorPtr = a pointer to the palette
;
;void ShowMCGA(unsigned char *VideoPtr, unsigned char * SourcePtr, unsigned char *ColorPtr);

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

Screenad	RN	R0	;Pointer to the 3DO screen memory
WorkPtr	RN	R1	;Pointer to the MCGA screen
Colors	RN	R2	;Pointer to the 3DO version of the palette
Temp	RN	R3	;Upper line cache
Temp2	RN	R4	;Lower line cache
Temp3	RN	R5	;3DO long word pixels
Height	RN	R12	;Temp height
Width	RN	LR	;Temp width

	EXPORT ShowMCGA

ShowMCGA	STMDB	sp!,{R4-R5,LR}	;Save the return address
	MOV	Height,#100	;Init the height count
Loop2	MOV	Width,#320	;Init the width count
Loop1	LDRB	Temp,[WorkPtr],#1	;Get 4 upper pixels from the table
	LDR	Temp2,[Colors,Temp,LSL #2] ;Convert to the color
	LDRB	Temp,[WorkPtr,#319]	;Get the pixel directly below
	LDR	Temp3,[Colors,Temp,LSL #2] ;Convert to the color
	ORR	Temp2,Temp3,Temp2,LSL #16 ;Blend together
	SUBS	Width,Width,#1	;At the edge?
	STR	Temp2,[Screenad],#4	;Store the result
	BNE	Loop1	;Loop for more
	SUBS	Height,Height,#1	;All lines done?
	ADD	WorkPtr,WorkPtr,#320	;Adjust the lower line (Skip 1)
	BNE	Loop2	;Loop for more
	LDMIA sp!,{R4-R5,PC}
	END
