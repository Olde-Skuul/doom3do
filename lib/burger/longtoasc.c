#include "Burger.h"

/****************************************

	Converts a signed long number into an ASCII string

****************************************/

void longToAscii(long Input,Byte *AsciiPtr)
{
	if (Input<0) {
		Input = -Input;
		AsciiPtr[0] = '-';
		++AsciiPtr;
	}
	LongWordToAscii(Input,AsciiPtr);
}
