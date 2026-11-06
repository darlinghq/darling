/*
 * arm64_complex_helpers.c
 *
 * Private (hidden-visibility) copies of `__muldc3` and `__mulsc3`, the
 * complex-multiplication helpers that clang emits libcalls to when it
 * encounters `double _Complex` / `float _Complex` multiplication.
 *
 * Background
 * ----------
 * These symbols normally live in compiler-rt's builtins library. In
 * Darling's build graph compiler-rt links *against* libsystem_m (for
 * sqrt, copysign, etc.), so making libsystem_m depend on compiler-rt
 * for `__muldc3`/`__mulsc3` would close the loop. Linking compiler-rt
 * publicly into libm also pollutes the system_m ABI with runtime-only
 * helper symbols.
 *
 * The fix is to keep the implementations physically inside libm but
 * private:
 *   - identical semantics to compiler-rt's `muldc3.c` / `mulsc3.c`
 *     (C99 Annex G special-case handling for NaN/Inf)
 *   - hidden visibility, so they are statically resolvable by libm's
 *     internal complex-multiply call sites but not exported as part
 *     of libsystem_m's public symbol list
 *   - compiler-rt continues to build and export its own non-hidden
 *     copies for everyone else; the two copies do not collide
 *
 * If/when libm itself stops emitting `__muldc3`/`__mulsc3` libcalls
 * (e.g. via -fno-emit-complex-builtin-libcalls or a future clang
 * pragma) this whole TU can be removed.
 */

#if defined(__aarch64__) || defined(__arm64__)

#include <math.h>

#pragma GCC visibility push(hidden)

double _Complex
__muldc3(double __a, double __b, double __c, double __d)
{
	double __ac = __a * __c;
	double __bd = __b * __d;
	double __ad = __a * __d;
	double __bc = __b * __c;
	double _Complex z;
	__real__ z = __ac - __bd;
	__imag__ z = __ad + __bc;
	if (__builtin_isnan(__real__ z) && __builtin_isnan(__imag__ z)) {
		int __recalc = 0;
		if (__builtin_isinf(__a) || __builtin_isinf(__b)) {
			__a = __builtin_copysign(__builtin_isinf(__a) ? 1.0 : 0.0, __a);
			__b = __builtin_copysign(__builtin_isinf(__b) ? 1.0 : 0.0, __b);
			if (__builtin_isnan(__c)) __c = __builtin_copysign(0.0, __c);
			if (__builtin_isnan(__d)) __d = __builtin_copysign(0.0, __d);
			__recalc = 1;
		}
		if (__builtin_isinf(__c) || __builtin_isinf(__d)) {
			__c = __builtin_copysign(__builtin_isinf(__c) ? 1.0 : 0.0, __c);
			__d = __builtin_copysign(__builtin_isinf(__d) ? 1.0 : 0.0, __d);
			if (__builtin_isnan(__a)) __a = __builtin_copysign(0.0, __a);
			if (__builtin_isnan(__b)) __b = __builtin_copysign(0.0, __b);
			__recalc = 1;
		}
		if (!__recalc && (__builtin_isinf(__ac) || __builtin_isinf(__bd) ||
		                  __builtin_isinf(__ad) || __builtin_isinf(__bc))) {
			if (__builtin_isnan(__a)) __a = __builtin_copysign(0.0, __a);
			if (__builtin_isnan(__b)) __b = __builtin_copysign(0.0, __b);
			if (__builtin_isnan(__c)) __c = __builtin_copysign(0.0, __c);
			if (__builtin_isnan(__d)) __d = __builtin_copysign(0.0, __d);
			__recalc = 1;
		}
		if (__recalc) {
			__real__ z = __builtin_huge_val()  * (__a * __c - __b * __d);
			__imag__ z = __builtin_huge_val()  * (__a * __d + __b * __c);
		}
	}
	return z;
}

float _Complex
__mulsc3(float __a, float __b, float __c, float __d)
{
	float __ac = __a * __c;
	float __bd = __b * __d;
	float __ad = __a * __d;
	float __bc = __b * __c;
	float _Complex z;
	__real__ z = __ac - __bd;
	__imag__ z = __ad + __bc;
	if (__builtin_isnan(__real__ z) && __builtin_isnan(__imag__ z)) {
		int __recalc = 0;
		if (__builtin_isinf(__a) || __builtin_isinf(__b)) {
			__a = __builtin_copysignf(__builtin_isinf(__a) ? 1.0f : 0.0f, __a);
			__b = __builtin_copysignf(__builtin_isinf(__b) ? 1.0f : 0.0f, __b);
			if (__builtin_isnan(__c)) __c = __builtin_copysignf(0.0f, __c);
			if (__builtin_isnan(__d)) __d = __builtin_copysignf(0.0f, __d);
			__recalc = 1;
		}
		if (__builtin_isinf(__c) || __builtin_isinf(__d)) {
			__c = __builtin_copysignf(__builtin_isinf(__c) ? 1.0f : 0.0f, __c);
			__d = __builtin_copysignf(__builtin_isinf(__d) ? 1.0f : 0.0f, __d);
			if (__builtin_isnan(__a)) __a = __builtin_copysignf(0.0f, __a);
			if (__builtin_isnan(__b)) __b = __builtin_copysignf(0.0f, __b);
			__recalc = 1;
		}
		if (!__recalc && (__builtin_isinf(__ac) || __builtin_isinf(__bd) ||
		                  __builtin_isinf(__ad) || __builtin_isinf(__bc))) {
			if (__builtin_isnan(__a)) __a = __builtin_copysignf(0.0f, __a);
			if (__builtin_isnan(__b)) __b = __builtin_copysignf(0.0f, __b);
			if (__builtin_isnan(__c)) __c = __builtin_copysignf(0.0f, __c);
			if (__builtin_isnan(__d)) __d = __builtin_copysignf(0.0f, __d);
			__recalc = 1;
		}
		if (__recalc) {
			__real__ z = __builtin_huge_valf() * (__a * __c - __b * __d);
			__imag__ z = __builtin_huge_valf() * (__a * __d + __b * __c);
		}
	}
	return z;
}

#pragma GCC visibility pop

#endif /* __aarch64__ || __arm64__ */
