#ifndef _XTRACELIB_H_
#define _XTRACELIB_H_
#include <stdint.h>

struct calldef
{
	const char* name;
	void (*print_args)(char*, void* args[]);
	void (*print_retval)(char*, uintptr_t rv);
};

#ifdef __cplusplus
extern "C" {
#endif

void handle_generic_entry(const struct calldef* defs, const char* type, int nr, void* args[]);
void handle_generic_exit(const struct calldef* defs, const char* type, int nr, uintptr_t retval);

#ifdef __cplusplus
}
#endif

#endif

