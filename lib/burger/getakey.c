#include <burger.h>        

/********************************

    Check if a key is pending and return zero if not, else the
    ASCII code in A

********************************/

Word GetAKey(void)
{
    if (!MyKbhit()) {    /* Key pending? */
        return 0;
    }
    return MyGetch();   /* Return the key */
}
