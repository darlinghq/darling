#ifndef MACH_MISC_H
#define MACH_MISC_H
#include <mach/kern_return.h>

extern "C"
{

	char* mach_error_string(kern_return_t rv);

}

#endif
