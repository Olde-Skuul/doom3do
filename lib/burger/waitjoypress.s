;
; Wait for a joystick to be pressed
;
; Word WaitJoyPress(void);
;

	AREA	|C$$code|,CODE,READONLY

	EXPORT WaitJoyPress
	IMPORT ReadJoyButtons

WaitJoyPress
	STR	LR,[sp,#-4]!	;Save the return address
Again	MOV	R0,#0	
	BL	ReadJoyButtons	;Read the joystick
	CMP	R0,#0	;Pressed it?
	BEQ	Again	;Wait
	LDR	PC,[sp],#4	;Exit then with R0 having the hit value

	END
