;
;Fill a region of memory with a constant byte
;This is more complicated due to long word alignment for ARM 6
;
;char * memset(void * Dest,int FillByte,size_t Len);
;

	AREA	|C$$code|,CODE,READONLY
|x$codeseg|


Dest	RN	R0	;Pointer to the Source data
FillByte	RN	R1	;Byte to fill with
Len	RN	R2	;Bytes to move
Temp1	RN	R3	;Temp register for destination pointer
Temp2	RN	R12	;Temp length word
Temp3	RN	LR
SP	RN	R13	;Stack pointer

	EXPORT memset2
		
memset2	CMP	Len,#0		;No memory to fill at all?
	MOVLE	PC,LR		;Less than or equal to zero?

	STMDB	SP!,{Dest,LR}	;Save the dest pointer and return address
	CMP	Len,#4		;Only 1-3 bytes?
	BCC	SlowLoop		;Then just do it the BYTE way...
	ANDS	Temp1,Dest,#3	;Long word aligned already?
	BEQ	FastFill		;Do a Fast fill...

*
* Force long word alignment
*

	RSB	Temp1,Temp1,#4		;Reverse 1-3 into 3-1
	CMP	Temp1,#2			;Test for 1-3
	STRB	FillByte,[Dest],#1	;Always start 1 at least (1,2,3)
	STRGEB FillByte,[Dest],#1	;Store if 2 or 3 (2,3)
	STRGTB FillByte,[Dest],#1	;Store if 3 only (3)
	SUB	Len,Len,Temp1		;Remove the count from the total
	CMP	Len,#4			;Can't go the rest of the way?
	BCC	SlowLoop			;No way dude, finish slowly!

*
* Fill using long words
*

FastFill	AND	FillByte,FillByte,#&FF			;Make sure this is a BYTE
	ORR	FillByte,FillByte,FillByte,LSL #8
	ORR	FillByte,FillByte,FillByte,LSL #16	;Convert to a long word
	MOV	Temp1,FillByte					;Make lot's o'copies!!
	MOV	Temp2,FillByte
	MOV	Temp3,FillByte

*
* Fast block copy in 32 byte chunks
*

Loop	SUBS	Len,Len,#32				;Remove from the total
	STMCSIA Dest!,{FillByte,Temp1,Temp2,Temp3}	;Move 32 bytes
	STMCSIA Dest!,{FillByte,Temp1,Temp2,Temp3}
	BCS	Loop				;Keep going for more...

*
* Fill in all the stragglers
*

LastOnes	TST	Len,#16				;16 bytes?
	STMNEIA Dest!,{FillByte,Temp1,Temp2,Temp3}	;Move it
	TST	Len,#8				;8 bytes?
	STMNEIA Dest!,{FillByte,Temp1}	;Move it
	TST	Len,#4				;Last long word?
	STRNE	FillByte,[Dest],#4		;Move it
	ANDS	Len,Len,#3				;Remove the long word bits
	LDMEQIA SP!,{Dest,PC}			;Done now?

*
* Finish the fill (Len = 1 through 3)
*

SlowLoop	CMP	Len,#2			;Test for 1,2 or 3 bytes...
	STRB	FillByte,[Dest],#1	;Store if 1 2 or 3
	STRGEB FillByte,[Dest],#1	;Store if 2 or 3
	STRGTB FillByte,[Dest],#1	;Store if 3 only
	LDMIA SP!,{Dest,PC}		;Return to caller
	END
