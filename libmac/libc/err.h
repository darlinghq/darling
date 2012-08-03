#ifndef LIBC_ERR_H
#define LIBC_ERR_H
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void __darwin_err(int eval, const char *fmt, ...);

void
__darwin_err_set_exit(void (*exitf)(int));

#ifdef __BLOCKS__
void
__darwin_err_set_exit_b(void (^exitb)(int));
#endif

void
__darwin_err_set_file(void *vfp);

void
__darwin_errc(int eval, int code, const char *fmt, ...);

void
__darwin_errx(int eval, const char *fmt, ...);

void
__darwin_warn(const char *fmt, ...);

void
__darwin_warnc(int code, const char *fmt, ...);

void
__darwin_warnx(const char *fmt, ...);

void
__darwin_verr(int eval, const char *fmt, va_list args);

void
__darwin_verrc(int eval, int code, const char *fmt, va_list args);

void
__darwin_verrx(int eval, const char *fmt, va_list args);

void
__darwin_vwarn(const char *fmt, va_list args);

void
__darwin_vwarnc(int code, const char *fmt, va_list args);

void
__darwin_vwarnx(const char *fmt, va_list args);

#ifdef __cplusplus
}
#endif

#endif

