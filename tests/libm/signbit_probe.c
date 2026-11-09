// Copyright (C) 2026 Lubos Dolezel
//
// Compiled (not run) by signbit_header.cmake for each Darwin ABI. The long double
// helper behind signbit() must read the sign of whatever format long double has
// on the target: 80-bit x87 on i386/x86_64, a plain double on arm64.
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

int probe_negative(void) { return __inline_signbit(-1.0L) != 0; }
int probe_positive(void) { return __inline_signbit(1.0L) != 0; }
int probe_negative_zero(void) { return __inline_signbit(-0.0L) != 0; }
int probe_positive_zero(void) { return __inline_signbit(0.0L) != 0; }
int probe_negative_inf(void) { return __inline_signbit(-__builtin_infl()) != 0; }

#ifdef __cplusplus
}
#endif
