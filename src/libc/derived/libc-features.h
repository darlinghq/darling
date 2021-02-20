#if defined(__i386__)
	#include "i386/libc-features.h"
#elif defined(__x86_64__)
	#include "x86_64/libc-features.h"
#elif defined(__ppc__)
	#include "ppc/libc-features.h"
#elif defined(__arm64__)
	#include "arm64/libc-features.h"
#elif defined(__arm__)
	#include "arm/libc-features.h"
#else
	#error "Unsupported architecture!"
#endif
