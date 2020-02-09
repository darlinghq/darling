#ifndef _XTRACELIB_H_
#define _XTRACELIB_H_
#include <stdint.h>

struct calldef
{
	const char* name;
	void (*print_args)(char*, int nr, void* args[]);
	void (*print_retval)(char*, int nr, uintptr_t rv);
};

#ifdef __cplusplus
extern "C" {
#endif

void handle_generic_entry(const struct calldef* defs, const char* type, int nr, void* args[]);
void handle_generic_exit(const struct calldef* defs, const char* type, int nr, uintptr_t retval, int force_split);

extern int xtrace_no_color;
void xtrace_set_gray_color(void);
void xtrace_reset_color(void);

void xtrace_start_line(int indent);

#ifdef __cplusplus
}
#endif

#endif

