;
; Swap a short value from Little endian to Big endian (And back again)
;
;short SwapShort(short val)

	AREA	|C$$code|,CODE,READONLY
|x$codeseg|

	EXPORT SwapShort

Val	RN	R0
			;1 2 R0
SwapShort MOV	R0,R0,LSL #16	;Kill off the upper 16 bits
	MOV	R2,R0,LSR #24	;X 1 R2 (Get byte #1)
	MOV	R0,R0,LSL #8	;Shift off the upper byte
	ORR	R0,R2,R0,ASR #16	;2 1 R0 (Get byte #2) (Signed)
	MOV	PC,LR

	END
