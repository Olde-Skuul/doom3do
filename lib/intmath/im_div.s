;
; Divide two 32 bit numbers and return a 32
; point result. Also pin the number to maximum 0x7FFFFFFF or -0x7FFFFFFF.
; Round up the result if needed. Also handles divide by zero.
;
; long IM_Div(long Numerator,long Denominator);
;

	AREA	|C$$code|,CODE,READONLY
|x$codeseg|

Numerator RN	R0	;First fixed point value
Denominator RN	R1	;Second fixed point value
Temp	RN	R2	;Temp result (Modulo)
Answer	RN	R3	;Value created
Sign	RN	R12	;Flag for sign

	EXPORT IMDiv

IMDiv
	EOR	Sign,Denominator,Numerator ;Xor the two values together to
			;determine the results sign
	CMP	Denominator,#0	;Get the absolute value
	RSBMI	Denominator,Denominator,#0
	CMP	Numerator,#0	;Get absolute values of inputs
	RSBMI	Numerator,Numerator,#0

	RSBS	Temp,Denominator,Numerator,LSR #31 ;Detect overflow right now
	BCS	Overflow	;Too high, abort now

	MOV	Answer,#0	;Init the result

;
; Perform a standard divide
; Temp is only used for a discard since I have to store the result somewhere.
;

	RSBS	Temp,Denominator,Numerator,LSR #31	;Test modulo
	SBCCS	Numerator,Numerator,Denominator,LSL #31	;Perform the subtration
	ADC	Answer,Answer,Answer		;Add to the answer

	RSBS	Temp,Denominator,Numerator,LSR #30	;Repeat for all 31 bits
	SBCCS	Numerator,Numerator,Denominator,LSL #30
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #29
	SBCCS	Numerator,Numerator,Denominator,LSL #29
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #28
	SBCCS	Numerator,Numerator,Denominator,LSL #28
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #27
	SBCCS	Numerator,Numerator,Denominator,LSL #27
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #26
	SBCCS	Numerator,Numerator,Denominator,LSL #26
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #25
	SBCCS	Numerator,Numerator,Denominator,LSL #25
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #24
	SBCCS	Numerator,Numerator,Denominator,LSL #24
	ADC	Answer,Answer,Answer
	
	RSBS	Temp,Denominator,Numerator,LSR #23
	SBCCS	Numerator,Numerator,Denominator,LSL #23
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #22
	SBCCS	Numerator,Numerator,Denominator,LSL #22
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #21
	SBCCS	Numerator,Numerator,Denominator,LSL #21
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #20
	SBCCS	Numerator,Numerator,Denominator,LSL #20
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #19
	SBCCS	Numerator,Numerator,Denominator,LSL #19
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #18
	SBCCS	Numerator,Numerator,Denominator,LSL #18
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #17
	SBCCS	Numerator,Numerator,Denominator,LSL #17
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #16
	SBCCS	Numerator,Numerator,Denominator,LSL #16
	ADC	Answer,Answer,Answer
	
	RSBS	Temp,Denominator,Numerator,LSR #15
	SBCCS	Numerator,Numerator,Denominator,LSL #15
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #14
	SBCCS	Numerator,Numerator,Denominator,LSL #14
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #13
	SBCCS	Numerator,Numerator,Denominator,LSL #13
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #12
	SBCCS	Numerator,Numerator,Denominator,LSL #12
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #11
	SBCCS	Numerator,Numerator,Denominator,LSL #11
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #10
	SBCCS	Numerator,Numerator,Denominator,LSL #10
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #9
	SBCCS	Numerator,Numerator,Denominator,LSL #9
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #8
	SBCCS	Numerator,Numerator,Denominator,LSL #8
	ADC	Answer,Answer,Answer

	RSBS	Temp,Denominator,Numerator,LSR #7
	SBCCS	Numerator,Numerator,Denominator,LSL #7
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #6
	SBCCS	Numerator,Numerator,Denominator,LSL #6
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #5
	SBCCS	Numerator,Numerator,Denominator,LSL #5
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #4
	SBCCS	Numerator,Numerator,Denominator,LSL #4
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #3
	SBCCS	Numerator,Numerator,Denominator,LSL #3
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #2
	SBCCS	Numerator,Numerator,Denominator,LSL #2
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator,LSR #1
	SBCCS	Numerator,Numerator,Denominator,LSL #1
	ADC	Answer,Answer,Answer
	RSBS	Temp,Denominator,Numerator
	SBCCS	Numerator,Numerator,Denominator
	ADC	Answer,Answer,Answer

	TST	Sign,#&80000000	;Adjust the sign on the answer
	RSBNE	Answer,Answer,#0
	MOV	R0,Answer	;Return in R0
	MOV	pc,lr

;
; I detected overflow!
;

Overflow	TST	Sign,#&80000000	;What's the answer's sign?
	MOVEQ	R0,#&7FFFFFFF	;Max positive number
	MOVNE	R0,#&80000000	;Max negative number
	MOV	pc,lr	;Exit now!

	END
