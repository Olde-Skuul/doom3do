;
; Swap a short value from Little endian to Big endian (And back again)
;
;unsigned short SwapUShort(unsigned short val)

	AREA	|C$$code|,CODE,READONLY
|x$codeseg|

	EXPORT SwapUShort

Val	RN	R0	;X X 1 2

SwapUShort MOV	R0,R0,ROR #8	;R0 = 2 X X 1
	AND	R2,R0,#&FF	;R2 = 0 0 0 1
	AND	R0,R0,#&FF000000	;R0 = 2 0 0 0
	ORR	R0,R2,R0,LSR #16	;R0 = F F 2 1 (Get byte #2)
	MOV	PC,LR

	END
