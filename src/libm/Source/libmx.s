/*	This file causes certain symbols to be defined in the ppc branch of
	libmx.dylib, libmx_debug.dylib, and libmx_profile.debug but nothing in
	other branches.  See Documentation/libmxNotes.doc for additional
	information.

	Additionally, a linker extension is used that causes the symbols to be
	visible only when linking for Mac OS X version 10.3.  This is the only
	version in which the symbols must be resolved from libmx.  In later
	versions, the symbols were available in libSystem.  In earlier versions,
	the symbols were not available in any dynamic library.
*/


#if defined __ppc__

	#define	Define(x)	$ld$add$os10.3$##x:  .globl $ld$add$os10.3$##x

	Define(_acosf)
	Define(_acoshf)
	Define(_asinf)
	Define(_asinhf)
	Define(_atan2f)
	Define(_atanf)
	Define(_atanhf)
	Define(_cabsf)
	Define(_cacos)
	Define(_cacosf)
	Define(_cacosh)
	Define(_cacoshf)
	Define(_carg)
	Define(_cargf)
	Define(_casin)
	Define(_casinf)
	Define(_casinh)
	Define(_casinhf)
	Define(_catan)
	Define(_catanf)
	Define(_catanh)
	Define(_catanhf)
	Define(_cbrtf)
	Define(_ccos)
	Define(_ccosf)
	Define(_ccosh)
	Define(_ccoshf)
	Define(_cexp)
	Define(_cexpf)
	Define(_clog)
	Define(_clogf)
	Define(_cosf)
	Define(_coshf)
	Define(_cpow)
	Define(_cpowf)
	Define(_cproj)
	Define(_cprojf)
	Define(_csin)
	Define(_csinf)
	Define(_csinh)
	Define(_csinhf)
	Define(_csqrt)
	Define(_csqrtf)
	Define(_ctan)
	Define(_ctanf)
	Define(_ctanh)
	Define(_ctanhf)
	Define(_erfcf)
	Define(_erff)
	Define(_exp2f)
	Define(_expf)
	Define(_expm1f)
	Define(_hypotf)
	Define(_lgammaf)
	Define(_log10f)
	Define(_log1pf)
	Define(_log2f)
	Define(_logf)
	Define(_powf)
	Define(_sinf)
	Define(_sinhf)
	Define(_sqrtf)
	Define(_tanf)
	Define(_tanhf)
	Define(_tgammaf)

#endif defined __ppc__
