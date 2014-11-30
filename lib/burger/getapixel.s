;
; Return the value of a pixel on the current video screen
;
; Word GetAPixel(Word x,Word y);
;

	AREA |C$$code|, CODE, READONLY
|x$codeseg|

	EXPORT GetAPixel
	IMPORT VideoPointer
	IMPORT YTable

x	RN	r0	;X coord and Index to the screen
y	RN	r1	;Y coord
Temp	RN	r2	;Scratchpad
Screenad	RN	r3	;Pointer to screen

GetAPixel
	TST	y,#1	;Set the flag for even/odd transfer
	LDR	Temp,YTableP	;Get the table pointer
	LDR	Screenad,VideoPointerP	;Get the true base address
	LDR	Temp,[Temp,y,LSL #2]	;Get the base offset for Y
	LDR	Screenad,[Screenad]	;Load the screen address pointer
	ADD	x,Temp,x,LSL #2	;Add in the x factor
	LDR	Temp,[Screenad,x]	;Fetch from the screen
	MOVNE	Temp,Temp,LSL #16	;If odd scan line, use low word
         MOV	r0,Temp,LSR #16	;Clear the high word and return it
	MOV	pc,lr	;Exit the routine

VideoPointerP
	DCD	VideoPointer
YTableP
	DCD	YTable

	END
