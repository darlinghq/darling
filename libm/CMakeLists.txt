project(darling-libsystem_m)

cmake_minimum_required(VERSION 2.4.0)
enable_language(C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -msse3 -w")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DARWIN_UNIX03 -fPIC -w") # -DBUILDING_FOR_CARBONCORE_LEGACY")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source/Intel)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../kernel-include)
add_definitions(-Dliteral8=text -Dliteral4=text -Dliteral16=text -Dmovsxw=movswl -DPRIVATE)

set(libm_sources
	Source/abs.c
	#Source/nan.c
	#Source/nanl.c
	#Source/version_info.c
	Source/Intel/acos.c
	Source/Intel/asin.c
	Source/Intel/atan.c
	Source/Intel/complex.c
	Source/Intel/containsNothingIntel.c
	Source/Intel/e_acosh.c
	Source/Intel/e_atanh.c
	Source/Intel/e_cosh.c
	Source/Intel/e_j0.c
	Source/Intel/e_j1.c
	Source/Intel/e_jn.c
	Source/Intel/e_sinh.c
	Source/Intel/expf_logf_powf.c
	Source/Intel/fenv.c
	Source/Intel/fmaf.c
	Source/Intel/nan.c
	Source/Intel/nanl.c
	Source/Intel/rndtol.c
	Source/Intel/s_asinh.c
	Source/Intel/sincostan.c
	Source/Intel/s_matherr.c
	Source/Intel/s_tanh.c
	Source/Intel/xmm_arcsincostan.c
	#Source/Intel/xmm_erfgamma.c
	Source/Intel/xmm_exp.c
	Source/Intel/xmm_fma.c
	Source/Intel/xmm_log.c
	Source/Intel/xmm_misc.c
	Source/Intel/xmm_nextafter.c
	Source/Intel/xmm_power.c
	Source/Intel/xmm_remainder.c
	Source/Intel/xmm_sqrt.c
	
	Source/Intel/acosf.S
	Source/Intel/acoshf.S
	Source/Intel/asinf.S
	Source/Intel/asinhf.S
	Source/Intel/atan2f.S
	Source/Intel/atanf.S
	Source/Intel/atanhf.S
	Source/Intel/cbrtf.S
	Source/Intel/ceilf.S
	Source/Intel/ceill.S
	Source/Intel/ceil.S
	Source/Intel/copysign.S
	Source/Intel/coshf.S
	Source/Intel/cosh.S
	Source/Intel/e_acos.S
	Source/Intel/e_asin.S
	Source/Intel/e_atan2.S
	Source/Intel/e_cbrtl.S
	Source/Intel/e_exp.S
	Source/Intel/e_log10.S
	Source/Intel/e_log.S
	Source/Intel/e_remainder.S
	Source/Intel/e_remquol.S
	Source/Intel/e_sqrt.S
	Source/Intel/exp2f.S
	Source/Intel/exp2.S
	Source/Intel/expf.S
	Source/Intel/expl.S
	Source/Intel/expm1f.S
	Source/Intel/expm1.S
	Source/Intel/exp.S
	Source/Intel/floorf.S
	Source/Intel/floorl.S
	Source/Intel/floor.S
	Source/Intel/fmaxfminfdim.S
	Source/Intel/fmod.S
	Source/Intel/frexp.S
	Source/Intel/fyl2x.S
	Source/Intel/hypotf.S
	Source/Intel/hypotl.S
	Source/Intel/hypot.S
	Source/Intel/ilogb.S
	Source/Intel/log10f.S
	Source/Intel/log2f.S
	Source/Intel/log2.S
	Source/Intel/logf.S
	#Source/Intel/log.S
	Source/Intel/lrintl.S
	Source/Intel/lroundf.S
	Source/Intel/lroundl.S
	Source/Intel/lround.S
	Source/Intel/modff.S
	Source/Intel/modfl.S
	Source/Intel/modf.S
	Source/Intel/nearbyintf.S
	Source/Intel/nearbyintl.S
	Source/Intel/nearbyint.S
	Source/Intel/nextafterf.S
	Source/Intel/nextafterl.S
	Source/Intel/nextafter.S
	#Source/Intel/powf.S
	Source/Intel/rintf.S
	Source/Intel/rintl.S
	Source/Intel/rint.S
	Source/Intel/roundf.S
	Source/Intel/roundl.S
	Source/Intel/round.S
	Source/Intel/s_atan.S
	Source/Intel/scalbnf.S
	Source/Intel/scalbnl.S
	Source/Intel/scalbn.S
	Source/Intel/s_cosisin.S
	#Source/Intel/s_cos.S
	#Source/Intel/s_ilogb.S
	Source/Intel/sinfcosf.S
	Source/Intel/sinhf.S
	Source/Intel/sinh.S
	Source/Intel/s_log1p.S
	#Source/Intel/s_logb.S
	Source/Intel/s_rint.S
	#Source/Intel/s_significand.S
	Source/Intel/s_sin.S
	Source/Intel/s_tan.S
	Source/Intel/tanf.S
	Source/Intel/tanhf.S
	Source/Intel/tanh.S
	Source/Intel/truncf.S
	Source/Intel/truncl.S
	Source/Intel/trunc.S

)

add_library(system_m SHARED ${libm_sources})
