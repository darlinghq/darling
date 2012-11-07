#ifndef MACH_MISC_H
#define MACH_MISC_H
#include <mach/kern_return.h>

#ifdef __cplusplus
extern "C"
{
#endif

	char* mach_error_string(kern_return_t rv);

#ifdef __cplusplus
}
#endif

#endif
