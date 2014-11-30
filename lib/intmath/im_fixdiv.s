;
; Divide two 16.16 bit fixed point numbers and return a 16.16 fixed
; point result. Also pin the number to maximum 0x7FFFFFFF or -0x7FFFFFFF.
; Round up the result if needed. Also handles divide by zero.
;
; Fixed IM_FixDiv(Fixed Numerator,Fixed Denominator);
;

	AREA	|C$$code|,CODE,READONLY
|x$codeseg|

Numerator RN	R0	;First fixed point value
Denominator RN	R1	;Second fixed point value
Temp	RN	R2	;Temp result (Modulo)
Answer	RN	R3	;Value created
Sign	RN	R12	;Flag for sign

	EXPORT IMFixDiv

IMFixDiv
	EOR	Sign,Denominator,Numerator ;Xor the two values together to
			;determine the results sign
	CMP	Denominator,#0	;Get the absolute value
	RSBMI	Denominator,Denominator,#0
	CMP	Numerator,#0	;Get absolute values of inputs
	RSBMI	Numerator,Numerator,#0

	RSBS	Temp,Denominator,Numerator,LSR #15 ;Detect overflow right now
	BCS	Overflow	;Too high, abort now

	MOV	Answer,#0	;Init the result

	CMP	Numerator,Denominator	;Very small?
	BCC	DivFraction	;Go straight into fractional division
	RSBS	Temp,Denominator,Numerator,LSR #8	;8 bits of precision?
	BCC	Div8Bits	;Skip the first 7 bits

;
; Perform a standard divide on the lowest 15 bits
; NOTE : I don't do LSR #31-15 since the largest unsigned number
; I can have is 0x7FFF and the high bit is clear.
; Temp is only used for a discard since I have to store the result somewhere.
;

	RSBS	Temp,Denominator,Numerator,LSR #14	;Test modulo
	SBCCS	Numerator,Numerator,Denominator,LSL #14	;Perform the subtration
	ADC	Answer,Answer,Answer		;Add to the answer

	RSBS	Temp,Denominator,Numerator,LSR #13	;Repeat for all 31 bits
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

;
; Speed up by entering here for 8 bit div
;

Div8Bits	RSBS	Temp,Denominator,Numerator,LSR #7
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

;
; At this point I have performed 15 bits of divide, now I insert
; 16 zeros to get the fractional part of the divide.
; Currently Answer contains the INTEGER part of the result.
; NOTE : I do not have to detect overflow on the ADD instruction
; since the largest denominator I can have is 0x7FFFFFFF so if bit #31
; is set, it WILL be subtracted on the next pass. So I can never have
; bit #31 set when I perform the ADD.
;

DivFraction
	RSBS	Numerator,Denominator,Numerator,LSL #1	;Shift and sub
	ADDCC	Numerator,Denominator,Numerator	;Undo the subtraction if no good
	ADC	Answer,Answer,Answer	;Insert a bit into the answer

	RSBS	Numerator,Denominator,Numerator,LSL #1	;1
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;2
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;3
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;4
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;5
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;6
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;7
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;8
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;9
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;10
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;11
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;12
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;13
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;14
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer
	RSBS	Numerator,Denominator,Numerator,LSL #1	;15
	ADDCC	Numerator,Denominator,Numerator
	ADC	Answer,Answer,Answer

;
; Perform a phony test to detect if I should round up the result
;

	RSBS	Numerator,Denominator,Numerator,LSL #1	;Final test
	ADC	Answer,Answer,#0	;Round up the result if needed

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
