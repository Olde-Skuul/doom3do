;
; Load and show a picture to the main video screen
;
;void ShowPic(Word PicNum);
;

	AREA	|C$$code|,CODE,READONLY

	EXPORT ShowPic
	IMPORT LoadAResource
	IMPORT ReleaseAResource
	IMPORT ShowPicPtr

;
; Load in the picture resource and then copy it to the main work screen
;

PicNum	RN	R0	;Picture resource #

ShowPic
	STMDB	sp!,{R0,LR}	;Save the return address and registers
	BL	LoadAResource	;Load in the picture resource
	BL	ShowPicPtr	;Pass the pointer to the display code
	LDR	R0,[sp],#4	;Get the resource #
	BL	ReleaseAResource	;Release the memory
	LDR	PC,[sp],#4	;Exit...
	END
