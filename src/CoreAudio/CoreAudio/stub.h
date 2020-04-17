#ifndef CA_STUB_H
#define CA_STUB_H
#include <stdio.h>

#ifndef STUB
#	define STUB() fprintf(stderr, "CoreAudio STUB: %s called\n", __FUNCTION__)
#endif

#endif

