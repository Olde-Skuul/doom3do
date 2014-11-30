#if 0
// Decompress into subsection of current buffer specified
//  by x,y,w,h in units of SWIDTHxSHEIGHT (8x8).

void nfHiColorDecomp(unsigned char *comp,unsigned int x, unsigned int y,
	unsigned int w, unsigned int h)
{
    unsigned char *tbuf;
    unsigned int new_row;
    unsigned int parms_sz;
    unsigned short *bx;
    unsigned char *si;
    unsigned char *di;
    unsigned int dx;
    unsigned int si2;
    unsigned int i,j,k;
    unsigned char a,b;

    nf_new_x = x*SWIDTH*2;
    nf_new_w = w*SWIDTH*2;
    nf_new_y = y*SHEIGHT;
    nf_new_h = h*SHEIGHT;

    new_row = nf_new_row0 - nf_new_w;
    parms_sz = (w*h*nf_fqty)<<1;

    // Move to correct place in current buffer
    tbuf = nf_buf_cur;
    if (x||y) {
		tbuf += nf_new_y*nf_width + nf_new_x;
	}

	// ds:si indexes comp (to get new section data)
	// es:di indexes current screen buffer
	// dx is a frequently used constant
	// ds/fs:bx indexes section params

    si = comp;
    di = tbuf;
    dx = nf_new_line;	// width - SWIDTH

	bx = (unsigned short *) si;	 // Parms index
	si+= parms_sz;	 // Skip over flags (w*h*2)

	// Iterate over params and copy new hires data to appropriate sections.

   	for (k=0;k<h;k++) {
		for (i=0;i<w;i++) {
			if (!*bx) {
				Move16;		/* First row */
				di+=dx;
				Move16;		/* Second row */
                   di+=dx;
                   Move16;		/* Third row */
                   di+=dx;
                   Move16;		/* Fourth row */
                   di+=dx;
                   Move16;		/* Fifth row */
                   di+=dx;
                   Move16;		/* Sixth row */
                   di+=dx;
                   Move16;		/* Seventh row */
                   di+=dx;
                   Move16;		/* Eighth row */
				di-=nf_back_right;
			}
			++bx;
			di+=SWIDTH*2;
		}
		di+=new_row;
	}

	// Iterate over flags and motion source addresses from params
	//  to determine which sections to move.
	// fs:bx indexes params.
	// ds:si indexes source from buffer
	//   ds will either be segment of current or previous buffer
	//   si will be computed as +- 16K relative to di.

	bx = (unsigned short *) comp;	// Move back to start of section parms
	di = tbuf;
   	for (k=0;k<h;k++) {
		for (i=0;i<w;i++) {
			si2 = (unsigned int) SwapUShort(*bx);
			if (si2&0x8000) {
	// Move one section from previous screen to current screen.
	// Enter with ds pointing to previous screen,
	//  es:di to destination screen section,
	//  relative value of source offset in si.

ms_30:
				si2-=0xC000;	// Make si absolute
				si2+=si2;
				si2+=(di-nf_buf_cur);	/* Get the offset */
				si=nf_buf_prv+si2;	/* Make true pointer */
                       Move162;		/* First row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Second row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Third row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Fourth row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Fifth row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Sixth row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Seventh row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Eighth row */
                       di-=nf_back_right;
			} else if (si2) {
ms_10:
	// Move one section from current screen to current screen.
	// Enter with ds pointing to current screen,
	//  es:di to destination screen section,
	//  relative value of source offset in si.

				si2-=0x4000;	// Make si absolute
				si2+=si2;
				si=di+si2;
                       Move162;		/* First row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Second row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Third row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Fourth row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Fifth row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Sixth row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Seventh row */
				si+=dx;
                       di+=dx;
                       Move162;		/* Eighth row */
                       di-=nf_back_right;
			}
			++bx;
			di+=SWIDTH*2;
		}
		di+=new_row;
	}
}
#endif
