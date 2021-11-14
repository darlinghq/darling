#ifndef _XTRACELIB_H_
#define _XTRACELIB_H_
#include <stdint.h>
#include <darling/emulation/simple.h>

struct calldef
{
	const char* name;
	void (*print_args)(int nr, void* args[]);
	void (*print_retval)(int nr, uintptr_t rv);
};

#ifdef __cplusplus
extern "C" {
#endif

void handle_generic_entry(const struct calldef* defs, const char* type, int nr, void* args[]);
void handle_generic_exit(const struct calldef* defs, const char* type, uintptr_t retval, int force_split);

extern int xtrace_no_color;
void xtrace_set_gray_color(void);
void xtrace_reset_color(void);

void xtrace_start_line(int indent);

void xtrace_log(const char* format, ...) __attribute__((format(printf, 1, 2)));
void xtrace_log_v(const char* format, va_list args) __attribute__((format(printf, 1, 0)));
void xtrace_error(const char* format, ...) __attribute__((format(printf, 1, 2)));
void xtrace_error_v(const char* format, va_list args) __attribute__((format(printf, 1, 0)));

void xtrace_abort(const char* message) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif

