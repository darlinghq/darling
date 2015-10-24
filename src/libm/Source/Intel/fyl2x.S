#include "machine/asm.h"
#include "abi.h"

// long double _fylgx( long double y, long double x )
ENTRY ( __fyl2x )
	fldt	FRAME_SIZE(STACKP)
	fldt	16+FRAME_SIZE( STACKP )
	fyl2x
	ret

