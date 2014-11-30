;
; Show a picture to the main video screen
;
;void ShowPicPtr(void *PicPtr);
;

	AREA	|C$$code|,CODE,READONLY

	EXPORT ShowPicPtr
	IMPORT memcpy
	IMPORT VideoPointer

;
; Load in the picture resource and then copy it to the main work screen
;

PicPtr	RN	R0	;Picture resource #

ShowPicPtr
	MOV	R1,PicPtr	;Source pointer
	MOV	R2,#128000	;Length of picture data
	LDR	R0,VideoP	;Get pointer to video
	LDR	R0,[R0]	;Get true pointer
	ADD	R0,R0,#20*640	;Offset from the top 20 lines
	B	memcpy	;Copy the memory

VideoP	DCD	VideoPointer	;Get the memory pointer
	END
