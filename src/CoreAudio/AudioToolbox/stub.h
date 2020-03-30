#ifndef AT_STUB_H
#define AT_STUB_H
#include <stdio.h>

#ifndef STUB
#	define STUB() fprintf(stderr, "AudioToolbox STUB: %s called\n", __FUNCTION__)
#endif

#define TRACE()
#define TRACE1(...)
#define TRACE2(...)

#endif

