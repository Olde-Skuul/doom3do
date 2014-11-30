;
;Halt execution of the program so the debugger can look at the code
;
;void Halt(void);
;

	AREA	|C$$code|,CODE,READONLY

	EXPORT Halt
Halt	SWI	&101	;Invoke the 3DO Debugger
	MOV	PC,LR	;Return to caller
			;(Allow the debugger to skip)
	END
