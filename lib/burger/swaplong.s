;
; Swap a long value from Little endian to Big endian (And back again)
; I use the rule A^B = C, A^A = 0, 0^B = B, A^A^B = B;
;
;LongWord SwapLong(LongWord val)
;

	AREA	|C$$code|,CODE,READONLY
|x$codeseg|

	EXPORT SwapLong
	EXPORT SwapULong

Val	RN	R0
Temp	RN	R1

SwapULong		;Input bytes are 1,2,3,4
SwapLong
	EOR	Temp,Val,Val,ROR #16	;Temp = 1^3,2^4,1^3,2^4
	BIC	Temp,Temp,#&FF0000	;Temp = 1^3,0,1^3,2^4 (Not used)
	MOV	Val,Val,ROR #8	;Val = 4,1,2,3
	EOR	Val,Val,Temp,LSR #8	;Val = 4^0=4,1^1^3=3,2^0=2,3^1^3=1
	MOV	PC,LR	;Exit

	END
