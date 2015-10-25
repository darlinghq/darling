/*	This is an undeclared and undocumented routine used by strtof_l in libc.a.
	It appears to be identical to the C standard fegetround except that it
	uses different values for the modes.

	This routine should be deprecated.  libc.a should use the standard routine.
*/

#ifdef ARMLIBM_FENV_SUPPORT

#include "fenv.h"
#include <float.h>

int __fegetfltrounds(void);

int __fegetfltrounds(void)
{
	switch (fegetround())
	{
		case FE_TONEAREST:	return  1;
		case FE_TOWARDZERO:	return  0;
		case FE_UPWARD:		return  2;
		case FE_DOWNWARD:	return  3;
		default:			return -1;
	}
}

#endif
