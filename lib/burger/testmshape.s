;
; Is the shape damaged?
;
; bool TestMShape(Word x,Word y,CCB *ShapePtr);
;

	AREA	|C$$code|,CODE,READONLY

X	RN	R0	;X coord of the shape
Y	RN	R1	;Y coord of the shape
ShapePtr	RN	R2	;Pointer to the shape CCB
Screenad	RN	R3	;Pointer to the screen
Screenad2 RN	R4	;Pointer to the screen (True)
Width	RN	R5	;Width of the shape
Width2	RN	R1	;Width counter
Height	RN	R0	;Height of the shape
Temp	RN	R6	;Temp register
Shape1	RN	R12	;Source word
Shape2	RN	R14	;Dest word

	EXPORT TestMShape
	IMPORT VideoPointer

TestMShape
	STMDB	sp!,{R4-R6,LR}
	ADD	Screenad2,X,#640*20/4	;Add in the X coord (In pixels)
	MOVS	Temp,Y,LSR #1	;Clear out the lowest bit (In carry)
	MOV	Screenad2,Screenad2,LSL #2	;Convert pixels to longs
	ADDCS	Screenad2,Screenad2,#2	;Adjust to the high/low word
	ADD	Temp,Temp,Temp,LSL #2	;Mul by 5
	ADD	Screenad2,Screenad2,Temp,LSL #8 ;Mul by 256 (256*5 = 1280)

	LDR	Temp,VideoP	;Get the pointer to the video screen
	LDR	Temp,[Temp]
	ADD	Screenad2,Temp,Screenad2	;I now have the TRUE pixel address

	LDR	Height,[ShapePtr,#52]	;Get the vertical count
	LDR	Width,[ShapePtr,#56]	;Get the horizontal count
	MOV	Height,Height,LSL #16
	MOV	Width,Width,LSL #32-11	;Mask all but the low 11 bits
	MOV	Height,Height,LSR #16+6	;I have the height
	MOV	Width,Width,LSR #32-11	;I have the width
	ADD	Height,Height,#1	;Fix the Height
	ADD	Width,Width,#1	;Fix the Width
	ADD	ShapePtr,ShapePtr,#60	;Index to the raw shape

NewLine	MOV	Width2,Width	;Save the width
	MOV	Screenad,Screenad2	;Get the current screen address
WidthLoop LDRB	Shape1,[ShapePtr],#1	;Get a shape word
	LDRB	Temp,[ShapePtr],#1
	ORR	Shape1,Shape1,Temp,LSL #8 ;Convert to 16 bit value
	LDRB	Shape2,[Screenad],#1	;Get a screen word
	LDRB	Temp,[Screenad],#3
	ORR	Shape2,Shape2,Temp,LSL #8 ;Convert to 16 bit value
	CMP	Shape1,#0	;Masked byte?
	TEQNE	Shape1,Shape2	;Match then?
	BNE	OhShit	;Nope, exit with result
	SUBS	Width2,Width2,#1	;Count down...
	BNE	WidthLoop	;Hit the edge
	TST	Screenad2,#2	;Even line?
	ORREQ	Screenad2,Screenad2,#2	;Adjust one line down
	BICNE	Screenad2,Screenad2,#2	;Clear the line
	ADDNE	Screenad2,Screenad2,#1280 ;Adjust for odd to even...
	SUBS	Height,Height,#1	;Any more?
	BNE	NewLine	;Yep, try again
	LDMIA	sp!,{R4-R6,pc}	;Return zero in R0

OhShit	MOV	R0,#1	;Return true
	LDMIA	sp!,{R4-R6,pc}	;Exit

VideoP	DCD	VideoPointer
	END
