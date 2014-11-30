;
; Wait until the joystick is released
;
; void WaitJoyRelease(void)
;

	AREA	|C$$code|,CODE,READONLY
|x$codeseg|

	EXPORT WaitJoyRelease
	IMPORT ReadJoyButtons

SP	RN	sp

WaitJoyRelease STR LR,[sp,#-4]!	;Save the return address
Again	MOV	R0,#0		;Joypad #0	
	BL	ReadJoyButtons	;Read the joystick
	CMP	R0,#0	;Released it?
	BNE	Again
	LDR	PC,[sp],#4	;Exit then
	END
