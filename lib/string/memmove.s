*
* Copy a region of memory from one place to another but take into
* account long word aligned memory.
*
* Note : This routine is convoluted and needlessly complex since the
* ARM 6 chokes on 32 bit fetches on NON-Long aligned addresses.
*
* char * memmove(void * Dest,const void * Src,size_t Len);
*

	AREA	|C$$code|,CODE,READONLY
|x$codeseg|


Dest	RN	R0	;Pointer to the dest buffer
Src	RN	R1	;Pointer to the source data
Len	RN	R2	;Bytes to move
Temp1	RN	R3	;First temp (Temp1-5 MUST BE SEQUENTIAL!!!)
Temp2	RN	R4	;Second temp
Temp3	RN	R5	;Third temp (Must be saved)
Temp4	RN	R12	;Last temp (Muse be saved)
Temp5	RN	LR	;Bit shift temp for non-aligned moves
SP	RN	R13	;My stack pointer

	EXPORT memmove

memmove    CMP	Len,#0	;Check the memory move length
	MOVLE	PC,LR	;Less than or equal to zero?
	CMP	Src,Dest	;Is the source less than the dest?
	MOVEQ	PC,LR	;If they are the same then bug out!
	STMDB	SP!,{Dest,R4,R5,LR}	;Save the return address
	BCC	MoveBack	;Go backwards!

*
* Move the memory forwards!
*

	CMP	Len,#4	;Less than 4 bytes to move?
	BCC	Last3For	;Only move those the easy way!

*
* Long align the destination pointer for long word moves
*

	ANDS	Temp1,Dest,#3	;Is the dest aligned?
	BEQ	ChkSrcFor		;Yes, it is, move in blocks now...
	RSB	Temp1,Temp1,#4	;How many bytes to move?
	CMP	Temp1,#2		;1,2 or 3?
	LDRB	Temp2,[Src],#1	;Copy if 1 2 or 3
	STRB	Temp2,[Dest],#1
	LDRGEB Temp2,[Src],#1	;Copy if 2 or 3
	STRGEB Temp2,[Dest],#1
	LDRGTB Temp2,[Src],#1	;Copy if 3
	STRGTB Temp2,[Dest],#1
	SUB	Len,Len,Temp1
	CMP	Len,#4		;Less than 4 now? (Can't be zero)
	BCC	Last3For		;Slow dude!

*
* At this time, the dest pointer is long word aligned,
* so do burst moves...
*

ChkSrcFor	ANDS	Temp1,Src,#3		;I guess it's not aligned!
	BNE	NotLongFor

*
* Fully long aligned moves
*

	SUBS	Len,Len,#32		;Move 32 bytes?
	BCC	FinishFor1		;Not enough dude!
LoopAFor	LDMIA Src!,{Temp1,Temp2,Temp3,Temp4}	;Move 16 bytes
	STMIA Dest!,{Temp1,Temp2,Temp3,Temp4}
	LDMIA Src!,{Temp1,Temp2,Temp3,Temp4}	;Move another 16 bytes
	STMIA Dest!,{Temp1,Temp2,Temp3,Temp4}
	SUBS	Len,Len,#32		;Remove another 32 bytes from the length
	BCS	LoopAFor		;Keep going?
FinishFor1	TST	Len,#16		;16 more?
	LDMNEIA Src!,{Temp1,Temp2,Temp3,Temp4}	;Move 16 bytes
	STMNEIA Dest!,{Temp1,Temp2,Temp3,Temp4}
	TST	Len,#8		;8 more?
	LDMNEIA Src!,{Temp1,Temp2}	;Move 8 bytes
	STMNEIA Dest!,{Temp1,Temp2}
	TST	Len,#4		;4 more?
	LDRNE	Temp1,[Src],#4	;Move 4 bytes
	STRNE	Temp1,[Dest],#4

*
* Move the last 1-3 bytes
*

AndLast3For ANDS	Len,Len,#3		;Mask off excess
	LDMEQIA SP!,{Dest,R4,R5,PC}	;Exit now?
Last3For	CMP	Len,#2		;Test for 1,2 or 3 bytes
	LDRB	Temp1,[Src],#1	;Always move the first
	STRB	Temp1,[Dest],#1
	LDRGEB Temp1,[Src],#1	;Move if 2 or 3
	STRGEB Temp1,[Dest],#1
	LDRGTB Temp1,[Src],#1	;Move if 3 only
	STRGTB Temp1,[Dest],#1
	LDMIA	SP!,{Dest,R4,R5,PC}	;Bug out!

*
* Non-long aligned burst moves... (At least 4 bytes to move!)
*

NotLongFor	BIC	Src,Src,#3		;Long align the source register
	LDR	Temp5,[Src],#4	;Get the extra data
	CMP	Temp1,#2		;How many bytes off?
	BGT	Off3For		;Off by 3 bytes?
	BEQ	Off2For		;Off by 2 bytes?

*
* Src = +1
*

	CMP	Len,#16		;<16 bytes?
	BCC	L1SlowFor		;Copy longs...
L1LoopFor	MOV	Temp1,Temp5,LSL #8	;Shift up 1 byte
	LDMIA	Src!,{Temp2,Temp3,Temp4,Temp5}	;Fetch the next 16 bytes
	ORR	Temp1,Temp1,Temp2,LSR #24
	MOV	Temp2,Temp2,LSL #8
	ORR	Temp2,Temp2,Temp3,LSR #24
	MOV	Temp3,Temp3,LSL #8
	ORR	Temp3,Temp3,Temp4,LSR #24
	MOV	Temp4,Temp4,LSL #8
	ORR	Temp4,Temp4,Temp5,LSR #8
	STMIA	Dest!,{Temp1,Temp2,Temp3,Temp4}	;Save the shifted 16 bytes
Off1For	SUBS	Len,Len,#16			;16 bytes processed...
	BCS	L1LoopFor
	ANDS	Len,Len,#&0F			;Undo the subtract
	LDMEQIA SP!,{Dest,R4,R5,PC}	;Bug out?
	CMP	Len,#3
	SUBCC Src,Src,#3
	BCC	AndLast3For

L1SlowFor	MOV	Temp1,Temp5,LSL #8	;Shift up 1 byte
	LDR	Temp5,[Src],#4		;Refill cache
	ORR	Temp1,Temp1,Temp5,LSR #24	;Blend
	STR	Temp1,[Dest],#4		;Save
	SUBS	Len,Len,#4			;Count down
	BCS	L1SlowFor
	SUB	Src,Src,#3		;Reset the cache address
	B	AndLast3For

*
* Src = +2
*

L2LoopFor	MOV	Temp1,Temp5,LSL #16 ;Shift up 2 bytes
	LDMIA	Src!,{Temp2,Temp3,Temp4,Temp5} ;Fetch the next 16 bytes
	ORR	Temp1,Temp1,Temp2,LSR #16
	MOV	Temp2,Temp2,LSL #16
	ORR	Temp2,Temp2,Temp3,LSR #16
	MOV	Temp3,Temp3,LSL #16
	ORR	Temp3,Temp3,Temp4,LSR #16
	MOV	Temp4,Temp4,LSL #16
	ORR	Temp4,Temp4,Temp5,LSR #16
	STMIA	Dest!,{Temp1,Temp2,Temp3,Temp4}
Off2For	SUBS	Len,Len,#16			;16 bytes processed...
	BCS	L2LoopFor
	ANDS	Len,Len,#&0F			;Undo the subtract
	LDMEQIA SP!,{Dest,R4,R5,PC}	;Bug out?
	CMP	Len,#3
	SUBCC	Src,Src,#2
	BCC	AndLast3For

L2SlowFor	MOV	Temp1,Temp5,LSL #16	;Shift up 2 bytes
	LDR	Temp5,[Src],#4		;Refill cache
	ORR	Temp1,Temp1,Temp5,LSR #16	;Blend
	STR	Temp1,[Dest],#4		;Save
	SUBS	Len,Len,#4			;Count down
	BCS	L2SlowFor
	SUB	Src,Src,#2			;Reset the source pointer
	B	AndLast3For

*
* Src = +3
*

L3LoopFor	MOV	Temp1,Temp5,LSL #24
	LDMIA Src!,{Temp2,Temp3,Temp4,Temp5}
	ORR	Temp1,Temp1,Temp2,LSR #8
	MOV	Temp2,Temp2,LSL #24
	ORR	Temp2,Temp2,Temp3,LSR #8
	MOV	Temp3,Temp3,LSL #24
	ORR	Temp3,Temp3,Temp4,LSR #8
	MOV	Temp4,Temp4,LSL #24
	ORR	Temp4,Temp4,Temp5,LSR #8
	STMIA	Dest!,{Temp1,Temp2,Temp3,Temp4}
Off3For	SUBS	Len,Len,#16			;16 bytes processed...
	BCS	L3LoopFor
	ANDS	Len,Len,#&0F			;Undo the subtract
	LDMEQIA SP!,{Dest,R4,R5,PC}	;Bug out?
	CMP	Len,#3
	SUBCC	Src,Src,#1
	BCC	AndLast3For

L3SlowFor	MOV	Temp1,Temp5,LSL #24	;Shift up 3 bytes
	LDR	Temp5,[Src],#4		;Refill cache
	ORR	Temp1,Temp1,Temp5,LSR #8	;Blend
	STR	Temp1,[Dest],#4		;Save
	SUBS	Len,Len,#4			;Count down
	BCS	L3SlowFor
	SUB	Src,Src,#1			;Reset the source pointer
	B	AndLast3For

*
* Move the memory backwards!
*

MoveBack	ADD	Src,Src,Len		;Move both pointers to the FINAL byte
	ADD	Dest,Dest,Len	;Dest too...
	CMP	Len,#4		;Move slowly?
	BCC	Last3Back

*
* Long align the dest pointer
*

	ANDS	Temp1,Dest,#3
	BEQ	ChkSrcBack		;Yes, it is, move in blocks now...
	CMP	Temp1,#2
	LDRB	Temp2,[Src,#-1]!
	STRB	Temp2,[Dest,#-1]!
	LDRGEB Temp2,[Src,#-1]!
	STRGEB Temp2,[Dest,#-1]!
	LDRGTB Temp2,[Src,#-1]!
	STRGTB Temp2,[Dest,#-1]!
	SUBS	Len,Len,Temp1	;Remove the move total
	CMP	Len,#4
	BCC	Last3Back

*
* Now move the memory BACKWARDS!
*

ChkSrcBack	ANDS	Temp1,Src,#3
	BNE	NotLongBack

*
* The memory is long aligned!
*

	SUBS	Len,Len,#32		;Move 32 bytes?
	BCC	FinishBack1
LoopABack	LDMDB Src!,{Temp1,Temp2,Temp3,Temp4}	;Move 16 bytes
	STMDB Dest!,{Temp1,Temp2,Temp3,Temp4}
	LDMDB Src!,{Temp1,Temp2,Temp3,Temp4}	;Move another 16 bytes
	STMDB Dest!,{Temp1,Temp2,Temp3,Temp4}
	SUBS	Len,Len,#32		;Remove another 32 bytes from the length
	BCS	LoopABack		;Keep going?
FinishBack1 TST	Len,#16		;16 more?
	LDMNEDB Src!,{Temp1,Temp2,Temp3,Temp4}	;Move 16 bytes
	STMNEDB Dest!,{Temp1,Temp2,Temp3,Temp4}
	TST	Len,#8		;8 more?
	LDMNEDB Src!,{Temp1,Temp2}	;Move 8 bytes
	STMNEDB Dest!,{Temp1,Temp2}
	TST	Len,#4		;4 more?
	LDRNE	Temp1,[Src,#-4]!	;Move 4 bytes
	STRNE	Temp1,[Dest,#-4]!

*
* Copy the last 3 bytes
*

AndLast3Back ANDS Len,Len,#3
	LDMEQIA SP!,{Dest,R4,R5,PC}	;Exit now?
Last3Back	CMP	Len,#2
	LDRB	Temp1,[Src,#-1]!
	STRB	Temp1,[Dest,#-1]!
	LDRGEB Temp1,[Src,#-1]!
	STRGEB Temp1,[Dest,#-1]!
	LDRGTB Temp1,[Src,#-1]!
	STRGTB Temp1,[Dest,#-1]!
	LDMIA SP!,{Dest,R4,R5,PC}	;Exit now?

*
* Non-long aligned burst moves... (At least 4 bytes to move!)
*

NotLongBack BIC	Src,Src,#3	;Long align the source pointer
	LDR	Temp5,[Src]	;Fill up the cache
	CMP	Temp1,#2	;How many bytes off?
	BCC	Off3Back	;Off by 3 bytes?
	BEQ	Off2Back	;Off by 2 bytes?

*
* Src = +1
*

	CMP	Len,#16		;<16 bytes?
	BCC	L1SlowBack		;Copy longs...
L1LoopBack	MOV	Temp1,Temp5,LSR #8
	LDMDB	Src!,{Temp2,Temp3,Temp4,Temp5}
	ORR	Temp1,Temp1,Temp2,LSL #24
	MOV	Temp2,Temp2,LSR #8
	ORR	Temp2,Temp2,Temp3,LSL #24
	MOV	Temp3,Temp3,LSR #8
	ORR	Temp3,Temp3,Temp4,LSL #24
	MOV	Temp4,Temp4,LSR #8
	ORR	Temp4,Temp4,Temp5,LSL #24
	STMDB	Dest!,{Temp1,Temp2,Temp3,Temp4}

Off1Back	SUBS	Len,Len,#16			;16 bytes processed...
	BCS	L1LoopBack
	ANDS	Len,Len,#&0F			;Undo the subtract
	LDMEQIA SP!,{Dest,R4,R5,PC}	;Bug out?
	CMP	Len,#3
	ADDCC Src,Src,#3
	BCC	AndLast3Back

L1SlowBack	MOV	Temp1,Temp5,LSR #8
	LDR	Temp5,[Src,#-4]!
	ORR	Temp1,Temp1,Temp5,LSL #24
	STR	Temp1,[Dest,#-4]!
	SUBS	Len,Len,#4
	BCS	L1SlowBack
	ADD	Src,Src,#3
	B	Last3Back

*
* Src +2
*

L2LoopBack MOV	Temp1,Temp5,LSR #16
	LDMDB Src!,{Temp2,Temp3,Temp4,Temp5}
	ORR	Temp1,Temp1,Temp2,LSL #16
	MOV	Temp2,Temp2,LSR #16
	ORR	Temp2,Temp2,Temp3,LSL #16
	MOV	Temp3,Temp3,LSR #16
	ORR	Temp3,Temp3,Temp4,LSL #16
	MOV	Temp4,Temp4,LSR #16
	ORR	Temp4,Temp4,Temp5,LSL #16
	STMDB Dest!,{Temp1,Temp2,Temp3,Temp4}
Off2Back	SUBS	Len,Len,#16
	BCS	L2LoopBack
	ANDS	Len,Len,#&0F			;Undo the subtract
	LDMEQIA SP!,{Dest,R4,R5,PC}	;Bug out?
	CMP	Len,#3
	ADDCC Src,Src,#2
	BCC	AndLast3Back
L2SlowBack	MOV	Temp1,Temp5,LSR #16
	LDR	Temp5,[Src,#-4]!
	ORR	Temp1,Temp1,Temp5,LSL #16
	STR	Temp1,[Dest,#-4]!
	SUBS	Len,Len,#4
	BCS	L2SlowBack
	ADD	Src,Src,#2
	B	Last3Back

*
* Src +3
*

L3LoopBack	MOV	Temp1,Temp5,LSR #24
	LDMDB Src!,{Temp2,Temp3,Temp4,Temp5}
	ORR	Temp1,Temp1,Temp2,LSL #8
	MOV	Temp2,Temp2,LSR #24
	ORR	Temp2,Temp2,Temp3,LSL #8
	MOV	Temp3,Temp3,LSR #24
	ORR	Temp3,Temp3,Temp4,LSL #8
	MOV	Temp4,Temp4,LSR #24
	ORR	Temp4,Temp4,Temp5,LSL #8
	STMDB	Dest!,{Temp1,Temp2,Temp3,Temp4}
Off3Back	SUBS	Len,Len,#16
	BCS	L3LoopBack
	ANDS	Len,Len,#&0F			;Undo the subtract
	LDMEQIA SP!,{Dest,R4,R5,PC}	;Bug out?
	CMP	Len,#3
	ADDCC Src,Src,#1
	BCC	AndLast3Back
L3LoopSlow MOV	Temp1,Temp2,LSR #24
	LDR	Temp2,[Src,#-4]!
	ORR	Temp1,Temp1,Temp2,LSL #8
	STR	Temp1,[Dest,#-4]!
	SUBS	Len,Len,#4
	BCS	L3LoopSlow
	ADD	Src,Src,#1
	B	Last3Back
	END
