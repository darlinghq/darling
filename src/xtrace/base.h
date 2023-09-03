#ifndef _XTRACE_BASE_H_
#define _XTRACE_BASE_H_

// common utilities for xtrace code

#define XTRACE_INLINE static __attribute__((always_inline))
#define XTRACE_HIDDEN __attribute__((visibility("hidden")))

#ifdef __cplusplus
	#define XTRACE_DECLARATIONS_C_BEGIN extern "C" {
	#define XTRACE_CPP __cplusplus
#else
	#define XTRACE_DECLARATIONS_C_BEGIN
#endif

#ifdef __cplusplus
	#define XTRACE_DECLARATIONS_C_END }
#else
	#define XTRACE_DECLARATIONS_C_END
#endif

#endif // _XTRACE_BASE_H_
