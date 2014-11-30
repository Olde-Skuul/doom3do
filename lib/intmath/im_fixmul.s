;
; Multiply two 16.16 bit fixed point numbers and return a 16.16 fixed
; point result. Also pin the number to maximum 0x7FFFFFFF or -0x7FFFFFFF.
; Round up the result if needed.
;
; Fixed IM_FixMul(Fixed a,Fixed b);
;

	AREA	|C$$code|, CODE, READONLY
|x$codeseg|

Input1	RN	R0	;First fixed point value
Input2	RN	R1	;Second fixed point value
Temp	RN	R2	;Temp result
High1	RN	R3	;High word for Input1
High2	RN	R12	;High word for Input2
Sign	RN	R14	;Temp to hold the sign

	EXPORT IMFixMul

IMFixMul
	STR	lr,[sp,#-4]!	;Save the link register

;
; This is needed for all 4 cases
;

	EOR	Sign,Input2,Input1	;Get the sign of the result
	CMP	Input1,#0	;Get the absolute values of the inputs
	RSBMI	Input1,Input1,#0
	CMP	Input2,#0	;Input #2
	RSBMI	Input2,Input2,#0

;
; What type of multiply am I going to use (16X16, 16X32, 32X32)
;

	CMP	Input1,#&10000	;Is input1 16 bits?
	BCC	Test1616	;Check for 16 by 16 mul
	CMP	Input2,#&10000	;Are both Input1 and Input2 16 bits?
	BCC	Mul3216	;Perform 32,16 mul

;
; Do it the hard way with a full 32 by 32 bit mul
; Note, this is the ONLY case in which I can generate an overflow
; condition. So if I add greater than 48 bits, I will barf.
;

Mul3232			;Perform 32,32 mul
	MOV	High1,Input1,LSR #16	;Isolate the 16 bit pieces
	MOV	High2,Input2,LSR #16
	BIC	Input1,Input1,High1,LSL #16
	BIC	Input2,Input2,High2,LSL #16
	MUL	Temp,Input2,Input1	;Mul 0.2 * 0.1
	MUL	Input2,High1,Input2	;Mul 1.0 * 0.2
	MUL	Input1,High2,Input1	;Mul 2.0 * 0.1
	MUL	High2,High1,High2	;Mul 1.0 * 2.0

			;Also add in the rounding
	MOVS	Temp,Temp,LSR #16	;Convert the offset 32 bits into
	ADC	Temp,Temp,High2,LSL #16	;one unified fixed point number

	CMP	High2,#&8000	;Too high already?
	ADDCCS Input1,Input1,Input2	;Add the two fixed point values
	ADDCCS Input1,Input1,Temp	;Add in the unified fixed point value
	CMPCC Input1,#&80000000	;>=$80000000?
	MOVCS	R0,#&7FFFFFFF	;Maximum
	CMP	Sign,#0	;64 bit negation
	RSBMI R0,R0,#0	;Negate low word
	LDR	pc,[sp],#4	;Pop and exit

;
; Input1 is 32 bits, and Input2 is 16 bits
;

Mul3216
	MOV	High1,Input1,LSR #16	;Isolate the 16 bit chunks
	BIC	Input1,Input1,High1,LSL #16
	MUL	Temp,Input1,Input2	;Mul 0.1 * 0.2 (Low 32 bits)
	MUL	Input2,High1,Input2	;Mul 1.0 * 0.2
	MOVS	Temp,Temp,LSR #16	;Convert to fixed (Carry = rounding)
	ADC	R0,Input2,Temp	;Input2.Temp = 48 bit result
	CMP	Sign,#0	;Negate the answer?
	RSBMI	R0,R0,#0	;Negate the result
	LDR	pc,[sp],#4	;Pop and exit

;
; Input1 is 16 bits, and Input2 is 32 bits
;

Test1616	CMP	Input2,#&10000	;Are both values 16 bit?
	BCC	Mul1616	;Use the FAST routine
Mul1632
	MOV	High2,Input2,LSR #16	;Isolate the 16 bit pieces
	BIC	Input2,Input2,High2,LSL #16
	MUL	Temp,Input2,Input1	;Mul 0.2 * 0.1 (Low 32 bits)
	MUL	Input1,High2,Input1	;Mul 2.0 * 0.1
	MOVS	Temp,Temp,LSR #16	;Shift off fraction (Place rounding in Carry)
	ADC	R0,Input1,Temp	;Input2.Temp = 48 bit result
	CMP	Sign,#0	;Negate the answer?
	RSBMI	R0,R0,#0	;Negate the result
	LDR	pc,[sp],#4	;Pop and exit

;
; Really easy, both Input1 and Input2 are 16 bits
;

Mul1616	MUL	R0,Input2,Input1	;Perform the mul
	MOVS	R0,R0,LSR #16	;Convert result to fixed point
	ADC	R0,R0,#0	;Round up if needed
	CMP	Sign,#0	;Signed?
	RSBMI	R0,R0,#0	;Negate the result
	LDR	pc,[sp],#4	;Exit

	END
