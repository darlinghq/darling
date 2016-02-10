#include "ktrace.h"

void
runtime_ktrace1(runtime_ktrace_code_t code)
{
	void *ra = __builtin_extract_return_addr(__builtin_return_address(1));

	/* This syscall returns EINVAL when the trace isn't enabled. */
	if (launchd_apple_internal) {
		syscall(180, code, 0, 0, 0, (long)ra);
	}
}

void
runtime_ktrace0(runtime_ktrace_code_t code)
{
	void *ra = __builtin_extract_return_addr(__builtin_return_address(0));

	/* This syscall returns EINVAL when the trace isn't enabled. */
	if (launchd_apple_internal) {
		syscall(180, code, 0, 0, 0, (long)ra);
	}
}

void
runtime_ktrace(runtime_ktrace_code_t code, long a, long b, long c)
{
	void *ra = __builtin_extract_return_addr(__builtin_return_address(0));

	/* This syscall returns EINVAL when the trace isn't enabled. */
	if (launchd_apple_internal) {
		syscall(180, code, a, b, c, (long)ra);
	}
}
