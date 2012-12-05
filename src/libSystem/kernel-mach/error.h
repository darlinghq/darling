#ifndef MACH_ERROR_H
#define MACH_ERROR_H
#include <mach/kern_return.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* mach_error_string(kern_return_t code);
void mach_error(const char* str, kern_return_t code);
const char* mach_error_type(kern_return_t code);

#ifdef __cplusplus
}
#endif

#endif

