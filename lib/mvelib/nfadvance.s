;
;Swap the page pointers for the video
;
;void nfAdvance(void);
;

	AREA	|C$$code|,CODE,READONLY
|x$codeseg|

	EXPORT nfAdvance
	IMPORT nf_buf_cur
	IMPORT nf_buf_prv

nfAdvance

Src	RN	R0	;Source register
Dest	RN	R1	;Dest register
Temp	RN	R2	;Temp value

	LDR	Src,SrcAdr	;Get pointer to source address
	LDR	Dest,DestAdr	;Get pointer to dest address
	LDR	Temp,[Src]	;Fetch first word
	SWP	Temp,Temp,[Dest]	;Swap the words
	STR	Temp,[Src]	;Save the final word
	MOV	PC,LR	;Exit

SrcAdr	DCD	nf_buf_cur	;Current pointer
DestAdr	DCD	nf_buf_prv	;Previous pointer

	END
