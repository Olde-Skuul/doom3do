/**********************************

	Return the absolute value of an int
	(Somehow, 3DO forgot to include this one)

**********************************/

int abs(int val)
{
	if (val<0) {		/* Negative? */
		return -val;	/* Return the absolute value */
	}
	return val;			/* Return the value untouched */
}
