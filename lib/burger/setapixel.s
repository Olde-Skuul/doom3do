;
; Return the value of a pixel on the current video screen
;
; Word GetAPixel(Word x,Word y);
;

	AREA |C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT SetAPixel
	IMPORT VideoPointer
	IMPORT YTable

x	RN	r0	;X coord and Index to the screen
y	RN	r1	;Y coord
Color	RN	r2
Temp	RN	r3	;Scratchpad
Screenad	RN	r12	;Pointer to screen

SetAPixel
	TST	y,#1	;Set the flag for even/odd transfer
	LDR	Temp,YTableP	;Get the table pointer
	LDR	Screenad,VideoPointerP	;Get the true base address
	LDR	Temp,[Temp,y,LSL #2]	;Get the base offset for Y
	LDR	Screenad,[Screenad]	;Load the screen address pointer
	ADD	x,Temp,x,LSL #2	;Add in the x factor
	MOV	Temp,Color,LSR #8	;Isolate the high byte
	ADD	Screenad,Screenad,x	;Precalc the offset
	ADDNE	Screenad,Screenad,#2	;Offset to odd scan line
	STRB	Temp,[Screenad]	;Store the high byte
	STRB	Color,[Screenad,#1]	;Store the low byte
	MOV	pc,lr	;Exit the routine

VideoPointerP
	DCD	VideoPointer
YTableP
	DCD	YTable

	END
