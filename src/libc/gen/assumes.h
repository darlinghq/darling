#ifndef __OSX_ASSUMES_H__
#define __OSX_ASSUMES_H__

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <Availability.h>
#include <TargetConditionals.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <asl.h>

#if __GNUC__
#define osx_fastpath(x)	((typeof(x))__builtin_expect((long)(x), ~0l))
#define osx_slowpath(x)	((typeof(x))__builtin_expect((long)(x), 0l))
#define osx_constant(x) __builtin_constant_p((x))
#define osx_hardware_trap() __builtin_trap()

#define __OSX_COMPILETIME_ASSERT__(e) ({ \
	char __compile_time_assert__[(e) ? 1 : -1];	\
	(void)__compile_time_assert__; \
})
#else
#define osx_fastpath(x)	(x)
#define osx_slowpath(x)	(x)
#define osx_constant(x) ((long)0)
#define osx_hardware_trap() abort()

#define __OSX_COMPILETIME_ASSERT__(e) (e)
#endif /* __GNUC__ */

typedef bool (*osx_redirect_t)(const char *);

/* The asl_message argument is an aslmsg that, when given to asl_log(), will
 * direct the message to the MessageTracer diagnostic messages store rather than
 * the default system log store.
 */
typedef bool (*osx_log_callout_t)(aslmsg asl_message, void *ctx, const char *);

#if TARGET_OS_IPHONE
#define osx_set_crash_message(arg) /* no-op */
#else
#include <CrashReporterClient.h>
#define osx_set_crash_message(arg) _crc_make_setter(message, (arg))
#endif

#define osx_assumes(e) ({ \
	typeof(e) _e = osx_fastpath(e); /* Force evaluation of 'e' */ \
	if (!_e) { \
		if (osx_constant(e)) { \
			__OSX_COMPILETIME_ASSERT__(e); \
		} \
		_osx_assumes_log((uint64_t)(uintptr_t)_e); \
		_osx_avoid_tail_call(); \
	} \
	_e; \
})

#define osx_assumes_zero(e) ({ \
	typeof(e) _e = osx_slowpath(e); \
	if (_e) { \
		if (osx_constant(e)) { \
			__OSX_COMPILETIME_ASSERT__(!e); \
		} \
		_osx_assumes_log((uint64_t)(uintptr_t)_e); \
		_osx_avoid_tail_call(); \
	} \
	_e; \
})

/* This variant is for use with old-style POSIX APIs that return -1 on failure
 * and set errno. If the return code is -1, the value logged will be as though
 * osx_assumes_zero(errno) was used. It encapsulates the following pattern:
 *
 * int tubes[2];
 * if (pipe(tubes) == -1) {
 *     (void)osx_assumes_zero(errno);
 * }
 */
#define posix_assumes_zero(e) ({ \
	typeof(e) _e = osx_slowpath(e); \
	if (_e == (typeof(e))-1) { \
		_osx_assumes_log((uint64_t)(uintptr_t)errno); \
		_osx_avoid_tail_call(); \
	} \
	_e; \
})

#define osx_assert(e) ({ \
	typeof(e) _e = osx_fastpath(e); \
	if (!_e) { \
		if (osx_constant(e)) { \
			__OSX_COMPILETIME_ASSERT__(e); \
		} \
\
		char *message = _osx_assert_log((uint64_t)(uintptr_t)_e); \
		osx_set_crash_message(message); \
		osx_hardware_trap(); \
		free(message); \
	} \
})

#define osx_assert_zero(e) ({ \
	typeof(e) _e = osx_slowpath(e); \
	if (_e) { \
		if (osx_constant(e)) { \
			__OSX_COMPILETIME_ASSERT__(!e); \
		} \
\
		char *message = _osx_assert_log((uint64_t)(uintptr_t)_e); \
		osx_set_crash_message(message); \
		osx_hardware_trap(); \
		free(message); \
	} \
})

#define posix_assert_zero(e) ({ \
	typeof(e) _e = osx_slowpath(e); \
	if (_e == (typeof(e))-1) { \
		char *message = _osx_assert_log((uint64_t)(uintptr_t)errno); \
		osx_set_crash_message(message); \
		osx_hardware_trap(); \
		free(message); \
	} \
})

/* These are for defining your own assumes()-like wrapper calls so that you can
 * log additional information, such as the about-PID, sender, etc. They're
 * generally not useful for direct inclusion in your code.
 */
#define osx_assumes_ctx(f, ctx, e) ({ \
	typeof(e) _e = osx_fastpath(e); \
	if (!_e) { \
		if (osx_constant(e)) { \
			__OSX_COMPILETIME_ASSERT__(e); \
		} \
		_osx_assumes_log_ctx(f, ctx, (uintptr_t)_e); \
		_osx_avoid_tail_call(); \
	} \
	_e; \
})

#define osx_assumes_zero_ctx(f, ctx, e) ({ \
	typeof(e) _e = osx_slowpath(e); \
	if (_e) { \
		if (osx_constant(e)) { \
			__OSX_COMPILETIME_ASSERT__(!e); \
		} \
		_osx_assumes_log_ctx((f), (ctx), (uintptr_t)_e); \
		_osx_avoid_tail_call(); \
	} \
	_e; \
})

#define posix_assumes_zero_ctx(f, ctx, e) ({ \
	typeof(e) _e = osx_slowpath(e); \
	if (_e == (typeof(e))-1) { \
		_osx_assumes_log_ctx((f), (ctx), (uintptr_t)errno); \
		_osx_avoid_tail_call(); \
	} \
	_e; \
})

#define osx_assert_ctx(f, ctx, e) ({ \
	typeof(e) _e = osx_fastpath(e); \
	if (!_e) { \
		if (osx_constant(e)) { \
			__OSX_COMPILETIME_ASSERT__(e); \
		} \
\
		char *message = _osx_assert_log_ctx((f), (ctx), (uint64_t)(uintptr_t)_e); \
		osx_set_crash_message(message); \
		osx_hardware_trap(); \
		free(message); \
	} \
})

#define osx_assert_zero_ctx(f, ctx, e) ({ \
	typeof(e) _e = osx_slowpath(e); \
	if (_e) { \
		if (osx_constant(e)) { \
			__OSX_COMPILETIME_ASSERT__(!e); \
		} \
\
		char *message = _osx_assert_log_ctx((f), (ctx), (uint64_t)(uintptr_t)_e); \
		osx_set_crash_message(message); \
		osx_hardware_trap(); \
		free(message); \
	} \
})

#define posix_assert_zero_ctx(f, ctx, e) ({ \
	typeof(e) _e = osx_slowpath(e); \
	if (_e == (typeof(e))-1) { \
		char *message = _osx_assert_log_ctx((f), (ctx), (uint64_t)(uintptr_t)errno); \
		osx_set_crash_message(message); \
		osx_hardware_trap(); \
		free(message); \
	} \
})

__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_4_3)
extern void
_osx_assumes_log(uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
extern char *
_osx_assert_log(uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
extern void
_osx_assumes_log_ctx(osx_log_callout_t callout, void *ctx, uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
extern char *
_osx_assert_log_ctx(osx_log_callout_t callout, void *ctx, uint64_t code);

__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0)
extern void
_osx_avoid_tail_call(void);

__END_DECLS

#endif /* __OSX_ASSUMES_H__ */
