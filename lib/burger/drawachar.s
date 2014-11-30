;
; Draw a char to the video screen
;
; void DrawAChar(Word Char)
;

	AREA	|C$$code|,CODE,READONLY

Char	RN	R0	;Char to print
Font	RN	R1	;Pointer to font data
ScreenPtr RN	R2	;Pointer to the screen
Width	RN	R3	;Width of the font in bytes
Height	RN	R4	;Height of the font in lines
Screenad2 RN	R5	;Running screen pointer
Width2	RN	R6
Invisible RN	R7
FontOr	RN	R8
Temp	RN	R12
Temp2	RN	R14

	EXPORT DrawAChar
	IMPORT FontX
	IMPORT FontY
	IMPORT FontFirst
	IMPORT FontLast
	IMPORT FontWidths
	IMPORT FontPointer
	IMPORT FontInvisible
	IMPORT FontHeight
	IMPORT FontOrMask
	IMPORT VideoPointer
	IMPORT YTable

DrawAChar
	STMDB	sp!,{R4-R8,LR}
	LDR	Temp,FontFirstP	;Get the first valid char #
	LDR	Temp,[Temp]
	SUB	Char,Char,Temp	;Offset the char from the first
	LDR	Temp,FontLastP
	LDR	Temp,[Temp]	;Get the last valid char
	CMP	Char,Temp
	LDMGEIA sp!,{R4-R8,PC}	;Exit if I can't draw

	LDR	Font,FontWidthsP	;Get the pointer to the width table
	LDR	Font,[Font]
	LDRB	Height,[Font,Char]	;Get the width of this font (Keep in height)
	SUB	Width,Height,#1	;-1 for BPL addressing
	MOV	Width,Width,LSR #1	;Width in bytes

	LDR	Font,FontPtrP
	LDR	Font,[Font]	;Get the pointer to the font

	MOV	Char,Char,LSL #1	;Word index
	LDRB	Temp,[Font,Char]	;Get the low byte
	ADD	Char,Char,#1
	LDRB	Temp2,[Font,Char]	;Get the high byte
	ORR	Temp,Temp,Temp2,LSL #8	;Mesh together
	ADD	Font,Font,Temp	;Get the pointer to the true shape

	LDR	Temp,FontXP	;Get the X coord
	LDR	Screenad2,[Temp]	;Save in temp
	ADD	Height,Height,Screenad2	;Add in pixel width
	STR	Height,[Temp]	;Save X coord in FontX

	LDR	Temp,FontYP	;Get the FontY pointer
	LDR	Height,VideoP	;Get the pointer to the video screen
	LDR	Temp2,YTableP	;Get the pointer to the YTable
	LDR	Temp,[Temp]	;Get the Y coord
	LDR	Height,[Height]	;Get the base video pointer

	TST	Temp,#1	;Test the low bit of the Y coord
	LDR	Temp,[Temp2,Temp,LSL #2]	;Get the base Y offset
	ADD	Screenad2,Height,Screenad2,LSL #2 ;Add the X offset (Longs)
	ADD	Screenad2,Temp,Screenad2	;Add in the Y offset
	ADDNE	Screenad2,Screenad2,#2	;Adjust to the high/low word

	LDR	Height,FontHeightP	;Get the height
	LDR	Height,[Height]
	LDR	Invisible,FontInvisibleP	;Get the invisible color
	LDR	Invisible,[Invisible]
	LDR	FontOr,FontOrMaskP	;Get pointer to the table
	B	LoopEnt

;
; Draw the font
;

Loop2	TST	Screenad2,#2	;Even line?
	ORREQ	Screenad2,Screenad2,#2	;Adjust one line down
	BICNE	Screenad2,Screenad2,#2	;Clear the line
	ADDNE	Screenad2,Screenad2,#1280 ;Adjust for odd to even...
LoopEnt	MOV	ScreenPtr,Screenad2	;Reset the screen address
	MOV	Width2,Width
Loop1	LDRB	Char,[Font],#1	;Get a font byte
	MOV	Temp2,Char,LSR #4	;Mask the high nibble
	TEQ	Temp2,Invisible	;Masked color?
	BEQ	SkipIt	;No good?
	LDR	Temp,[FontOr,Temp2,LSL #2]	;Fetch value
	STRB	Temp,[ScreenPtr,#1]	;Save to screen (As short)
	MOV	Temp,Temp,LSR #8	;Fetch another byte
	STRB	Temp,[ScreenPtr]	;Save second

SkipIt	AND	Temp2,Char,#&F	;Mask the high nibble
	TEQ	Temp2,Invisible
	BEQ	SkipIt2
	LDR	Temp,[FontOr,Temp2,LSL #2]
	STRB	Temp,[ScreenPtr,#5]
	MOV	Temp,Temp,LSR #8
	STRB	Temp,[ScreenPtr,#4]
SkipIt2	SUBS	Width2,Width2,#1	;Adjust the width
	ADD	ScreenPtr,ScreenPtr,#8	;Adjust the dest address
	BPL	Loop1
	SUBS	Height,Height,#1	;The height is done?
	BNE	Loop2
	LDMIA	sp!,{R4-R8,PC}

FontXP	DCD	FontX
FontYP	DCD	FontY
FontHeightP DCD FontHeight
FontWidthsP DCD FontWidths
FontFirstP DCD	FontFirst
FontLastP DCD	FontLast
FontOrMaskP DCD FontOrMask
FontPtrP DCD	FontPointer
FontInvisibleP DCD FontInvisible
VideoP	DCD	VideoPointer
YTableP	DCD	YTable

	END
