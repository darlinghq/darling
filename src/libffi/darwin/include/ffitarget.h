#ifdef __arm__
#include "ffitarget_armv7.h"
#endif

#ifdef __arm64__
#include "ffitarget_arm64.h"
#endif

#if defined(__i386__) || defined(__x86_64__)
#include "ffitarget_x86.h"
#endif
