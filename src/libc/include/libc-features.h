#if defined(__x86_64__)
#	include "x86-64/libc-features.h"
#elif defined(__i386__)
#	include "i386/libc-features.h"
#else
#	error Missing include
#endif

