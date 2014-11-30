#include "Burger.h"

/**********************************

	Draw a shape using a resource number
	
**********************************/

void DrawRezShape(Word x,Word y,Word RezNum)
{
	DrawShape(x,y,LoadAResource(RezNum));
	ReleaseAResource(RezNum);
}
