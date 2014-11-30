;
; DLZSS decompressor (ARM-6 version)
; Written by Rebecca Ann Heineman
; Copyright 1995 by LogicWare
; So there.
;

	 AREA |C$$code|, CODE, READONLY

;
; void DLZSS(unsigned char *Dst, unsigned char *Src, unsigned long Length);
;
; Decompress data in the LZSS format
;

Dst	RN	R0	;Pointer to the Dest buffer
Src	RN	R1	;Pointer to the packed data
Length	RN	R2	;Bytes to decompress
FFFiller	RN	R3	;#&FFFFF000 Constant
ZFlags	RN	R4	;Bit bucket
PackPtr	RN	R14	;Packed data pointer
Temp	RN	R12	;Temp var

	EXPORT DLZSS	;My global

;
; Any data to decompress?
;

DLZSS	CMP	Length,#0	;No data to decompress?
	MOVEQ	PC,LR	;Exit then!

	STMDB sp!,{R4,LR}	;Save all registers I'll nuke!

	LDRB	ZFlags,[Src],#1	;Fetch a byte
	MOV	FFFiller,#&FF000000	;Init with an 8 bit number
	ORR	ZFlags,ZFlags,#&100	;Reset the count (Bit test)
	MOV	FFFiller,FFFiller,ASR #12 ;Make it #&FFFFF000
	MOVS	ZFlags,ZFlags,LSR #1	;Shove a bit in carry
	BCC	Decomp	;Multibyte

;
; Get a single raw byte
;

Single	LDRB	Temp,[Src],#1	;Copy a byte of data
	SUBS	Length,Length,#1	;Count down the length
	STRB	Temp,[Dst],#1	;Save to dest
	LDMEQIA sp!,{R4,PC}	;Exit now?

;
; Loop here, Dest always has index to pack buffer
; Get a byte in flags to know if the next 8 samples are either
; words or bytes
;

A1	CMP	ZFlags,#1	;Only the high bit left?
	LDREQB ZFlags,[Src],#1	;Fetch a byte
	ORREQ	ZFlags,ZFlags,#&100	;Reset the count (Bit test)
	MOVS	ZFlags,ZFlags,LSR #1	;Shove a bit in carry

;
; Raw byte or pack array?
;

	BCS	Single	;If the bit is clear then decompress

;
; Decompress from the ring buffer
;

Decomp	LDRB	PackPtr,[Src],#1	;Get the low word to offset
	LDRB	Temp,[Src],#1	;Get the high word
	ORR	PackPtr,PackPtr,Temp,LSL #8 ;Merge the bytes
	MOV	Temp,Temp,LSR #4	;Get the upper 4 bits
	ORR	PackPtr,PackPtr,FFFiller	;Ora with &FFFFF000
	ADD	Temp,Temp,#3	;Get true byte count
	SUBS	Length,Length,Temp	;Remove the run from the length
	ADD	PackPtr,PackPtr,Dst	;Add this offset to the pointer

	ADDLT	Temp,Temp,Length	;Use the ORIGINAL length in temp
	MOVLTS Length,#0	;Zap the length (And the zero flag)

	RSB	Temp,Temp,#18	;Reverse the count
	ADD	PC,PC,Temp,LSL #3	;Jump into the inline code
	MOV	R0,R0	;Nop to fix the PC

;
; I can unpack a maximum of 18:1 so I have 18 bytes moves here
;

C18	LDRB	Temp,[PackPtr],#1	;Copy the decompress buffer
	STRB	Temp,[Dst],#1
C17	LDRB	Temp,[PackPtr],#1	;17 bytes to go...
	STRB	Temp,[Dst],#1
C16	LDRB	Temp,[PackPtr],#1	;16 bytes to go...
	STRB	Temp,[Dst],#1
C15	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C14	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C13	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C12	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C11	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C10	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C09	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C08	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C07	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C06	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C05	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C04	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C03	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C02	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
C01	LDRB	Temp,[PackPtr],#1
	STRB	Temp,[Dst],#1
	BNE	A1	;Loop for more?
	LDMIA sp!,{R4,PC}	;Exit now?

	END
