#ifndef _XTRACELIB_H_
#define _XTRACELIB_H_
#include <stdint.h>
#include <darling/emulation/simple.h>

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
void handle_generic_exit(const struct calldef* defs, const char* type, uintptr_t retval, int force_split);

extern int xtrace_no_color;
extern int xtrace_kprintf;
void xtrace_set_gray_color(void);
void xtrace_reset_color(void);

void xtrace_start_line(int indent);

// the kprintf output is prefixed with "xtrace: " for easy grepping on dmesg

#define xtrace_printf(format, ...) ({ \
		if (xtrace_kprintf) { \
			__simple_kprintf("xtrace: " format, ##__VA_ARGS__); \
		} else { \
			__simple_printf(format, ##__VA_ARGS__); \
		} \
	})

#define xtrace_fprintf(fd, format, ...) ({ \
		if (xtrace_kprintf) { \
			__simple_kprintf("xtrace: " format, ##__VA_ARGS__); \
		} else { \
			__simple_fprintf(fd, format, ##__VA_ARGS__); \
		} \
	})

#ifdef __cplusplus
}
#endif

#endif

