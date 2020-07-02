project(darling-libsystem_m)

cmake_minimum_required(VERSION 2.4.0)
enable_language(C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2 -msse3 -w -ggdb -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__DARWIN_UNIX03 -fPIC -w -DBUILDING_FOR_CARBONCORE_LEGACY -funroll-loops -msse3") # -DBUILDING_FOR_CARBONCORE_LEGACY")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -DBUILDING_FOR_CARBONCORE_LEGACY")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

if (TARGET_ARM64)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLONG_LONG_MAX=0 -DLONG_LONG_MIN=0 -D__SOFTFP__")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Source/Intel)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../kernel-include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libc/darwin)
add_definitions(-Dmovsxw=movswl -DPRIVATE)

set(libm_sources
	Source/abs.c
	Source/sincos.c
	Source/exp10.c
	#Source/nan.c
	#Source/nanl.c
	#Source/version_info.c

	#Source/musl/__cos.c
	#Source/musl/__cosdf.c
	#Source/musl/exp_data.c
	#Source/musl/exp2f_data.c
	#Source/musl/log_data.c
	#Source/musl/log2_data.c
	#Source/musl/logf_data.c
	#Source/musl/__math_divzero.c
	#Source/musl/__math_divzerof.c
	#Source/musl/__math_invalid.c
	#Source/musl/__math_invalidf.c
	#Source/musl/__math_oflow.c
	#Source/musl/__math_oflowf.c
	#Source/musl/__math_uflow.c
	#Source/musl/__math_uflowf.c
	#Source/musl/__math_xflowf.c
	#Source/musl/__math_xflow.c
	#Source/musl/__rem_pio2.c
	#Source/musl/__rem_pio2_large.c
	#Source/musl/__rem_pio2f.c
	#Source/musl/__sin.c
	#Source/musl/__sindf.c
	#Source/musl/__tan.c
	#Source/musl/__tandf.c
	#Source/musl/asinhf.c
	#Source/musl/atan.c
	#Source/musl/atan2.c
	#Source/musl/atan2f.c
	#Source/musl/atanf.c
	#Source/musl/ceil.c
	#Source/musl/copysign.c
	#Source/musl/cos.c
	#Source/musl/cosf.c
	#Source/musl/expf.c
	#Source/musl/expm1f.c
	#Source/musl/fabs.c
	#Source/musl/floor.c
	#Source/musl/frexp.c
	#Source/musl/hypot.c
	#Source/musl/hypotf.c
	#Source/musl/ldexp.c
	#Source/musl/llrintf.c
	#Source/musl/llrintl.c
	#Source/musl/llroundf.c
	#Source/musl/llroundl.c
	#Source/musl/log.c
	#Source/musl/log1pf.c
	#Source/musl/log2.c
	#Source/musl/logbl.c
	#Source/musl/logf.c
	#Source/musl/pow.c
	#Source/musl/pow_data.c
	#Source/musl/scalbn.c
	#Source/musl/scalbnf.c
	#Source/musl/scalbnl.c
	#Source/musl/sin.c
	#Source/musl/sinf.c
	#Source/musl/tan.c
	#Source/musl/tanf.c
)

if (TARGET_x86_64 OR TARGET_i386)
	set(libm_sources ${libm_sources}
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
		Source/Intel/xmm_erfgamma.c
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
		#Source/Intel/e_remquol.S
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
		Source/Intel/s_cos.S
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

elseif (TARGET_ARM64)
	set(libm_sources ${libm_sources}
		Source/nanl.c

		Source/musl/__cos.c
		Source/musl/__cosdf.c
		Source/musl/exp_data.c
		Source/musl/exp2f_data.c
		Source/musl/log_data.c
		Source/musl/log2_data.c
		Source/musl/logf_data.c
		Source/musl/__math_divzero.c
		Source/musl/__math_divzerof.c
		Source/musl/__math_invalid.c
		Source/musl/__math_invalidf.c
		Source/musl/__math_oflow.c
		Source/musl/__math_oflowf.c
		Source/musl/__math_uflow.c
		Source/musl/__math_uflowf.c
		Source/musl/__math_xflowf.c
		Source/musl/__math_xflow.c
		Source/musl/__rem_pio2.c
		Source/musl/__rem_pio2_large.c
		Source/musl/__rem_pio2f.c
		Source/musl/__sin.c
		Source/musl/__sindf.c
		Source/musl/__tan.c
		Source/musl/__tandf.c
		Source/musl/asinhf.c
		Source/musl/atan.c
		Source/musl/atan2.c
		Source/musl/atan2f.c
		Source/musl/atanf.c
		Source/musl/ceil.c
		Source/musl/copysign.c
		Source/musl/cos.c
		Source/musl/cosf.c
		Source/musl/expf.c
		Source/musl/expm1f.c
		Source/musl/fabs.c
		Source/musl/floor.c
		Source/musl/frexp.c
		Source/musl/hypot.c
		Source/musl/hypotf.c
		Source/musl/ldexp.c
		Source/musl/llrintf.c
		Source/musl/llrintl.c
		Source/musl/llroundf.c
		Source/musl/llroundl.c
		Source/musl/log.c
		Source/musl/log1pf.c
		Source/musl/log2.c
		Source/musl/logbl.c
		Source/musl/logf.c
		Source/musl/pow.c
		Source/musl/pow_data.c
		Source/musl/scalbn.c
		Source/musl/scalbnf.c
		Source/musl/scalbnl.c
		Source/musl/sin.c
		Source/musl/sinf.c
		Source/musl/tan.c
		Source/musl/tanf.c

		Source/ARM/acos_freeBSD.c
		Source/ARM/acosh_freeBSD.c
		Source/ARM/asin_freeBSD.c
		Source/ARM/asinh_freeBSD.c
		Source/ARM/atanh_freeBSD.c
		Source/ARM/cbrt.c
		Source/ARM/complex.c
		Source/ARM/containsNothing.c
		Source/ARM/copysignf.c
		Source/ARM/cosh_freeBSD.c
		Source/ARM/e_rem_pio2_freeBSD.c
		Source/ARM/erf.c
		Source/ARM/erfc.c
		Source/ARM/erfcf.c
		Source/ARM/erff.c
		#Source/ARM/exp2.c
		Source/ARM/exp2_freeBSD.c
		Source/ARM/exp_freeBSD.c
		Source/ARM/expm1_freeBSD.c
		Source/ARM/fdim.c
		Source/ARM/fdimf.c
		Source/ARM/fegetfltrounds.c
		Source/ARM/fenv.c
		Source/ARM/fmaf.c
		Source/ARM/fma_freeBSD.c
		Source/ARM/fmod.c
		Source/ARM/fmodf.c
		Source/ARM/fpclassify.c
		Source/ARM/fpclassifyf.c
		Source/ARM/ilogb.c
		Source/ARM/ilogbf.c
		Source/ARM/j0y0_freeBSD.c
		Source/ARM/j1y1_freeBSD.c
		Source/ARM/jnyn_freeBSD.c
		Source/ARM/k_rem_pio2_freeBSD.c
		Source/ARM/k_tan_freeBSD.c
		Source/ARM/lgamma.c
		Source/ARM/lgammaf.c
		Source/ARM/llrint.c
		Source/ARM/llround.c
		Source/ARM/log.c
		Source/ARM/logb.c
		Source/ARM/logbf.c
		Source/ARM/lrint.c
		Source/ARM/math_errhandling.c
		Source/ARM/modf_IncrediblySkanky.c
		Source/ARM/nan.c
		Source/ARM/nearbyint.c
		Source/ARM/nextafter.c
		Source/ARM/nextafterf.c
		Source/ARM/nexttoward.c
		Source/ARM/nexttowardf.c
		Source/ARM/remainder.c
		Source/ARM/remainderf.c
		Source/ARM/remquo.c
		Source/ARM/remquof.c
		Source/ARM/rint.c
		Source/ARM/scalb.c
		Source/ARM/scalbln.c
		Source/ARM/scalblnf.c
		Source/ARM/signgam.c
		Source/ARM/sinh_freeBSD.c
		Source/ARM/sqrt.c
		Source/ARM/sqrtf.c
		Source/ARM/tanh_freeBSD.c
		Source/ARM/tgamma.c
		Source/ARM/tgammaf.c
		#Source/ARM/version_info.c
	)
endif(TARGET_x86_64 OR TARGET_i386)

# Gross hack to rename symbols in $fenv_access_off variants
set(CMAKE_AR "${CMAKE_CURRENT_SOURCE_DIR}/rename_wrapper")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (TARGET_x86_64 OR TARGET_i386)
	add_darling_object_library(system_m_extra Source/Intel/xmm_misc.c Source/Intel/xmm_log.c Source/Intel/fmaxfminfdim.S Source/Intel/nextafter.S Source/Intel/xmm_remainder.c Source/Intel/e_remquol.S)
	set_target_properties(system_m_extra PROPERTIES COMPILE_FLAGS "-UBUILDING_FOR_CARBONCORE_LEGACY -O0 -ggdb")
else (TARGET_x86_64 OR TARGET_i386)
	add_darling_object_library(system_m_extra Source/empty.c)
endif (TARGET_x86_64 OR TARGET_i386)
	make_fat(system_m_extra)

if (TARGET_x86_64 OR TARGET_i386)
	set(system_m_link_flags
		"-Wl,-alias_list,${CMAKE_CURRENT_SOURCE_DIR}/Exports/libm_Intel.a.alias -Wl,-alias_list,${CMAKE_CURRENT_SOURCE_DIR}/Exports/libmathCommonIntel.alias"
	)
else (TARGET_x86_64 OR TARGET_i386)
	set(system_m_link_flags "")
endif (TARGET_x86_64 OR TARGET_i386)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_m.dylib")
add_circular(system_m FAT
	SOURCES
		${libm_sources}
	OBJECTS
		$<TARGET_OBJECTS:system_m_extra>
	SIBLINGS
		system_c
		system_dyld
		compiler_rt
	LINK_FLAGS
		${system_m_link_flags}
)
#set_property(TARGET system_m APPEND_STRING PROPERTY LINK_FLAGS
#	"-Wl,-alias_list,${CMAKE_CURRENT_SOURCE_DIR}/Exports/libm_Intel.a.alias -Wl,-alias_list,${CMAKE_CURRENT_SOURCE_DIR}/Exports/libmathCommonIntel.alias")
#target_link_libraries(system_m)
#make_fat(system_m)

install(TARGETS system_m DESTINATION libexec/darling/usr/lib/system)

set(CMAKE_AR "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/x86_64-apple-darwin11-ar")
set(CMAKE_RANLIB "${CMAKE_BINARY_DIR}/src/external/cctools-port/cctools/ar/x86_64-apple-darwin11-ranlib")
add_darling_static_library(system_m_static SOURCES ${libm_sources} $<TARGET_OBJECTS:system_m_extra>)
make_fat(system_m_static)
