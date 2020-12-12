#ifndef _XTRACE_BASE_H_
#define _XTRACE_BASE_H_

// common utilities for xtrace code

#define XTRACE_INLINE static __attribute__((always_inline))

#ifdef __cplusplus
	#define XTRACE_DECLARATIONS_BEGIN extern "C" {
#else
	#define XTRACE_DECLARATIONS_BEGIN
#endif

#ifdef __cplusplus
	#define XTRACE_DECLARATIONS_END }
#else
	#define XTRACE_DECLARATIONS_END
#endif

#endif // _XTRACE_BASE_H_
