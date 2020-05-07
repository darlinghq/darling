/*
 *  cc_config.h
 *  corecrypto
 *
 *  Created on 11/16/2010
 *
 *  Copyright (c) 2010,2011,2012,2013,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CC_CONFIG_H_
#define _CORECRYPTO_CC_CONFIG_H_

/* A word about configuration macros:

    Conditional configuration macros specific to corecrypto should be named CORECRYPTO_xxx
    or CCxx_yyy and be defined to be either 0 or 1 in this file. You can add an
    #ifndef #error construct at the end of this file to make sure it's always defined.

    They should always be tested using the #if directive, never the #ifdef directive.

    No other conditional macros shall ever be used (except in this file)

    Configuration Macros that are defined outside of corecrypto (eg: KERNEL, DEBUG, ...)
    shall only be used in this file to define CCxxx macros.

    External macros should be assumed to be either undefined, defined with no value,
    or defined as true or false. We shall strive to build with -Wundef whenever possible,
    so the following construct should be used to test external macros in this file:

         #if defined(DEBUG) && (DEBUG)
         #define CORECRYPTO_DEBUG 1
         #else
         #define CORECRYPTO_DEBUG 0
         #endif


    It is acceptable to define a conditional CC_xxxx macro in an implementation file,
    to be used only in this file.

    The current code is not guaranteed to follow those rules, but should be fixed to.

    Corecrypto requires GNU and C99 compatibility.
    Typically enabled by passing --gnu --c99 to the compiler (eg. armcc)

*/

//Do not set this macros to 1, unless you are developing/testing for Linux under macOS
#define CORECRYPTO_SIMULATE_POSIX_ENVIRONMENT    0

//Do not set these macros to 1, unless you are developing/testing for Windows under macOS
#define CORECRYPTO_SIMULATE_WINDOWS_ENVIRONMENT 0
#define CORECRYPTO_HACK_FOR_WINDOWS_DEVELOPMENT 0 //to be removed after <rdar://problem/27304763> port corecrypto to Windows

#if (defined(DEBUG) && (DEBUG)) || defined(_DEBUG) //MSVC defines _DEBUG
/* CC_DEBUG is already used in CommonCrypto */
 #define CORECRYPTO_DEBUG 1
#else
 #define CORECRYPTO_DEBUG 0
#endif

// This macro can be used to enable prints when a condition in the macro "cc_require"
// is false. This is especially useful to confirm that negative testing fails
// at the intended location
#define CORECRYPTO_DEBUG_ENABLE_CC_REQUIRE_PRINTS 0

#if defined(KERNEL) && (KERNEL)
 #define CC_KERNEL 1 // KEXT, XNU repo or kernel components such as AppleKeyStore
#else
 #define CC_KERNEL 0
#endif

#if defined(__linux__) || CORECRYPTO_SIMULATE_POSIX_ENVIRONMENT
 #define CC_LINUX 1
#else
 #define CC_LINUX 0
#endif

#if defined(USE_L4) && (USE_L4)
 #define CC_USE_L4 1
#else
 #define CC_USE_L4 0
#endif

#if defined(RTKIT) && (RTKIT)
 #define CC_RTKIT 1
#else
 #define CC_RTKIT 0
#endif

#if defined(RTKITROM) && (RTKITROM)
#define CC_RTKITROM 1
#else
#define CC_RTKITROM 0
#endif

#if defined(USE_SEPROM) && (USE_SEPROM)
 #define CC_USE_SEPROM 1
#else
 #define CC_USE_SEPROM 0
#endif

#if defined(USE_S3) && (USE_S3)
 #define CC_USE_S3 1
#else
 #define CC_USE_S3 0
#endif

#if (defined(ICE_FEATURES_ENABLED)) || (defined(MAVERICK) && (MAVERICK))
 #define CC_BASEBAND 1
#else
 #define CC_BASEBAND 0
#endif

#if defined(EFI) && (EFI)
 #define CC_EFI 1
#else
 #define CC_EFI 0
#endif

#if defined(IBOOT) && (IBOOT)
 #define CC_IBOOT 1
#else
 #define CC_IBOOT 0
#endif

// Defined by the XNU build scripts
// Applies to code embedded in XNU but NOT to the kext
#if defined(XNU_KERNEL_PRIVATE)
 #define CC_XNU_KERNEL_PRIVATE 1
#else
 #define CC_XNU_KERNEL_PRIVATE 0
#endif

// handle unaligned data, if the cpu cannot. Currently for gladman AES and the C version of the SHA256
#define CC_HANDLE_UNALIGNED_DATA CC_BASEBAND

// BaseBand configuration
#if CC_BASEBAND

// -- ENDIANESS
#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
 #if defined(ENDIAN_LITTLE) || (defined(__arm__) && !defined(__BIG_ENDIAN))
  #define __LITTLE_ENDIAN__
 #elif !defined(ENDIAN_BIG) && !defined(__BIG_ENDIAN)
  #error Baseband endianess not defined.
 #endif
 #define AESOPT_ENDIAN_NO_FILE
#endif

// -- Architecture
 #define CCN_UNIT_SIZE  4 // 32 bits

// -- External function
 #define assert ASSERT   // sanity

// -- Warnings
// Ignore irrelevant warnings after verification
// #1254-D: arithmetic on pointer to void or function type
// #186-D: pointless comparison of unsigned integer with zero
// #546-D: transfer of control bypasses initialization of
 #ifdef __arm__
  #pragma diag_suppress 186, 1254,546
 #elif defined(__GNUC__)
// warning: pointer of type 'void *' used in arithmetic
  #pragma GCC diagnostic ignored "-Wpointer-arith"
 #endif // __arm__
#define CC_SMALL_CODE 1

#endif // CC_BASEBAND

#if CC_RTKIT || CC_RTKITROM
#define CC_SMALL_CODE 1
#endif


#ifndef CC_SMALL_CODE
#define CC_SMALL_CODE 0
#endif

//CC_XNU_KERNEL_AVAILABLE indicates the availibity of XNU kernel functions,
//like what we have on OSX, iOS, tvOS, Watch OS
#if defined(__APPLE__) && defined(__MACH__)
 #define CC_XNU_KERNEL_AVAILABLE 1
#else
 #define CC_XNU_KERNEL_AVAILABLE 0
#endif

//arm arch64 definition for gcc
#if defined(__GNUC__) && defined(__aarch64__) && !defined(__arm64__)
    #define __arm64__
#endif

#if !defined(CCN_UNIT_SIZE)
 #if defined(__arm64__) || defined(__x86_64__)  || defined(_WIN64)
  #define CCN_UNIT_SIZE  8
 #elif defined(__arm__) || defined(__i386__) || defined(_WIN32)
  #define CCN_UNIT_SIZE  4
 #else
  #error undefined architecture
 #endif
#endif /* !defined(CCN_UNIT_SIZE) */


//this allows corecrypto Windows development using xcode
#if defined(CORECRYPTO_SIMULATE_WINDOWS_ENVIRONMENT)
 #if CORECRYPTO_SIMULATE_WINDOWS_ENVIRONMENT && CC_XNU_KERNEL_AVAILABLE && CORECRYPTO_DEBUG
  #define CC_USE_ASM 0
  #define CC_USE_HEAP_FOR_WORKSPACE 1
   #if (CCN_UNIT_SIZE==8)
    #define CCN_UINT128_SUPPORT_FOR_64BIT_ARCH 0
   #else
    #define CCN_UINT128_SUPPORT_FOR_64BIT_ARCH 1
   #endif
 #endif
#endif

#if !defined(CCN_UINT128_SUPPORT_FOR_64BIT_ARCH)
 #if defined(_WIN64) && defined(_WIN32) && (CCN_UNIT_SIZE==8)
  #define CCN_UINT128_SUPPORT_FOR_64BIT_ARCH 0
 #elif defined(_WIN32)
  #define CCN_UINT128_SUPPORT_FOR_64BIT_ARCH 1//should not be a problem
 #else
  #define CCN_UINT128_SUPPORT_FOR_64BIT_ARCH 1
 #endif
#endif

#if defined(_MSC_VER)
    #if defined(__clang__)
        #define CC_ALIGNED(x) __attribute__ ((aligned(x))) //clang compiler
    #else
        #define CC_ALIGNED(x) __declspec(align(x)) //MS complier
    #endif
#else
    #if __clang__ || CCN_UNIT_SIZE==8
        #define CC_ALIGNED(x) __attribute__ ((aligned(x)))
    #else
        #define CC_ALIGNED(x) __attribute__ ((aligned((x)>8?8:(x))))
    #endif
#endif

#if defined(__arm__)
//this is copied from <arm/arch.h>, because <arm/arch.h> is not available on SEPROM environment
#if defined (__ARM_ARCH_7A__) || defined (__ARM_ARCH_7S__) || defined (__ARM_ARCH_7F__) || defined (__ARM_ARCH_7K__) || defined(__ARM_ARCH_7EM__)
  #define _ARM_ARCH_7
 #endif

 #if defined(__ARM_ARCH_6M__) || defined(__TARGET_ARCH_6S_M) || defined (__armv6m__)
  #define _ARM_ARCH_6M
 #endif
#endif

#if defined(__arm64__) || defined(__arm__)
 #define CCN_IOS				   1
 #define CCN_OSX				   0
#elif defined(__x86_64__) || defined(__i386__)
 #define CCN_IOS				   0
 #define CCN_OSX				   1
#endif

#if CC_USE_S3
/* For corecrypto kext, CC_STATIC should be undefined */
 #define CC_STATIC              1
#endif

#if !defined(CC_USE_HEAP_FOR_WORKSPACE)
 #if CC_USE_S3 || CC_USE_SEPROM || CC_RTKITROM
  #define CC_USE_HEAP_FOR_WORKSPACE 0
 #else
  #define CC_USE_HEAP_FOR_WORKSPACE 1
 #endif
#endif

/* memset_s is only available in few target */
#if CC_USE_SEPROM || defined(__CC_ARM) \
    || defined(__hexagon__) || CC_EFI
 #define CC_HAS_MEMSET_S 0
#else
 #define CC_HAS_MEMSET_S 1
#endif

// Include target conditionals if available.
#if defined(__has_include)     /* portability */
#if __has_include(<TargetConditionals.h>)
#include <TargetConditionals.h>
#endif /* __has_include(<TargetConditionals.h>) */
#endif /* defined(__has_include) */

// Disable RSA Keygen on iBridge
#if defined(TARGET_OS_BRIDGE) && TARGET_OS_BRIDGE && CC_KERNEL
#define CC_DISABLE_RSAKEYGEN 1 /* for iBridge */
#else
#define CC_DISABLE_RSAKEYGEN 0 /* default */
#endif

// see rdar://problem/26636018
#if (CCN_UNIT_SIZE == 8) && !( defined(_MSC_VER) && defined(__clang__))
#define CCEC25519_CURVE25519_64BIT 1
#else
#define CCEC25519_CURVE25519_64BIT 0
#endif

//- functions implemented in assembly ------------------------------------------
//this the list of corecrypto clients that use assembly and the clang compiler
#if !(CC_XNU_KERNEL_AVAILABLE || CC_KERNEL || CC_USE_L4 || CC_IBOOT || CC_RTKIT || CC_RTKITROM || CC_USE_SEPROM || CC_USE_S3) && !defined(_WIN32) && CORECRYPTO_DEBUG
 #warning "You are using the default corecrypto configuration, assembly optimizations may not be available for your platform"
#endif

// Enable assembler in Linux if CC_LINUX_ASM is defined
#if CC_LINUX && defined(CC_LINUX_ASM) && CC_LINUX_ASM
#define CC_USE_ASM 1
#endif

// Use this macro to strictly disable assembly regardless of cpu/os/compiler/etc.
// Our assembly code is not gcc compatible. Clang defines the __GNUC__ macro as well.
#if !defined(CC_USE_ASM)
 #if defined(_WIN32) || CC_EFI || CC_BASEBAND || CC_XNU_KERNEL_PRIVATE || (defined(__GNUC__) && !defined(__clang__)) || defined(__ANDROID_API__) || CC_LINUX
  #define CC_USE_ASM 0
 #else
  #define CC_USE_ASM 1
 #endif
#endif

//-(1) ARM V7
#if defined(_ARM_ARCH_7) && __clang__ && CC_USE_ASM
 #define CCN_DEDICATED_SQR      CC_SMALL_CODE
 #define CCN_MUL_KARATSUBA      0 // no performance improvement
 #define CCN_ADD_ASM            1
 #define CCN_SUB_ASM            1
 #define CCN_MUL_ASM            0
 #define CCN_ADDMUL1_ASM        1
 #define CCN_MUL1_ASM           1
 #define CCN_CMP_ASM            1
 #define CCN_ADD1_ASM           1
 #define CCN_SUB1_ASM           1
 #define CCN_N_ASM              1
 #define CCN_SET_ASM            1
 #define CCN_SHIFT_RIGHT_ASM    1
 #if defined(__ARM_NEON__) 
 #define CCN_SHIFT_LEFT_ASM     1
 #else
 #define CCN_SHIFT_LEFT_ASM     0
 #endif
 #define CCN_MOD_224_ASM        1
 #define CCN_MULMOD_256_ASM     1
 #define CCAES_ARM_ASM          1
 #define CCAES_INTEL_ASM        0
 #if CC_KERNEL || CC_USE_L4 || CC_IBOOT || CC_RTKIT || CC_RTKITROM || CC_USE_SEPROM || CC_USE_S3
  #define CCAES_MUX             0
 #else
  #define CCAES_MUX             1
 #endif
 #define CCN_USE_BUILTIN_CLZ    1
 #define CCSHA1_VNG_INTEL       0
 #define CCSHA2_VNG_INTEL       0

 #if defined(__ARM_NEON__) || CC_KERNEL
  #define CCSHA1_VNG_ARM        1
  #define CCSHA2_VNG_ARM        1
 #else /* !defined(__ARM_NEON__) */
  #define CCSHA1_VNG_ARM        0
  #define CCSHA2_VNG_ARM        0
 #endif /* !defined(__ARM_NEON__) */
 #define CCSHA256_ARMV6M_ASM 0

 #define CC_ACCELERATECRYPTO    1

//-(2) ARM 64
#elif defined(__arm64__) && __clang__ && CC_USE_ASM
 #define CCN_DEDICATED_SQR      CC_SMALL_CODE
 #define CCN_MUL_KARATSUBA      1 // 4*n CCN_UNIT extra memory required.
 #define CCN_ADD_ASM            1
 #define CCN_SUB_ASM            1
 #define CCN_MUL_ASM            1
 #define CCN_ADDMUL1_ASM        0
 #define CCN_MUL1_ASM           0
 #define CCN_CMP_ASM            1
 #define CCN_ADD1_ASM           0
 #define CCN_SUB1_ASM           0
 #define CCN_N_ASM              1
 #define CCN_SET_ASM            0
 #define CCN_SHIFT_RIGHT_ASM    1
 #define CCN_SHIFT_LEFT_ASM     1
 #define CCN_MOD_224_ASM        0
 #define CCN_MULMOD_256_ASM     1
 #define CCAES_ARM_ASM          1
 #define CCAES_INTEL_ASM        0
 #define CCAES_MUX              0        // On 64bit SoC, asm is much faster than HW
 #define CCN_USE_BUILTIN_CLZ    1
 #define CCSHA1_VNG_INTEL       0
 #define CCSHA2_VNG_INTEL       0
 #define CCSHA1_VNG_ARM         1
 #define CCSHA2_VNG_ARM         1
 #define CCSHA256_ARMV6M_ASM    0

 #define CC_ACCELERATECRYPTO    1

//-(3) Intel 32/64
#elif (defined(__x86_64__) || defined(__i386__)) && __clang__ && CC_USE_ASM
 #define CCN_DEDICATED_SQR      1
 #define CCN_MUL_KARATSUBA      1 // 4*n CCN_UNIT extra memory required.
 /* These assembly routines only work for a single CCN_UNIT_SIZE. */
 #if (defined(__x86_64__) && CCN_UNIT_SIZE == 8) || (defined(__i386__) && CCN_UNIT_SIZE == 4)
  #define CCN_ADD_ASM            1
  #define CCN_SUB_ASM            1
  #define CCN_MUL_ASM            1
 #else
  #define CCN_ADD_ASM            0
  #define CCN_SUB_ASM            0
  #define CCN_MUL_ASM            0
 #endif

 #if (defined(__x86_64__) && CCN_UNIT_SIZE == 8)
  #define CCN_CMP_ASM            1
  #define CCN_N_ASM              1
  #define CCN_SHIFT_RIGHT_ASM    1
  #define CCN_SHIFT_LEFT_ASM     1
 #else
  #define CCN_CMP_ASM            0
  #define CCN_N_ASM              0
  #define CCN_SHIFT_RIGHT_ASM    0
  #define CCN_SHIFT_LEFT_ASM     0
 #endif

 #define CCN_MOD_224_ASM        0
 #define CCN_MULMOD_256_ASM     0
 #define CCN_ADDMUL1_ASM        0
 #define CCN_MUL1_ASM           0
 #define CCN_ADD1_ASM           0
 #define CCN_SUB1_ASM           0
 #define CCN_SET_ASM            0
 #define CCAES_ARM_ASM          0
 #define CCAES_INTEL_ASM        1
 #define CCAES_MUX              0
 #define CCN_USE_BUILTIN_CLZ    0
 #define CCSHA1_VNG_INTEL       1
 #define CCSHA2_VNG_INTEL       1
 #define CCSHA1_VNG_ARM         0
 #define CCSHA2_VNG_ARM         0
 #define CCSHA256_ARMV6M_ASM    0

 #define CC_ACCELERATECRYPTO    1

//-(4) disable assembly
#else
 #if CCN_UINT128_SUPPORT_FOR_64BIT_ARCH
  #define CCN_DEDICATED_SQR     1
 #else
  #define CCN_DEDICATED_SQR     0 //when assembly is off and 128-bit integers are not supported, dedicated square is off. This is the case on Windows
 #endif
 #define CCN_MUL_KARATSUBA      1 // 4*n CCN_UNIT extra memory required.
 #define CCN_ADD_ASM            0
 #define CCN_SUB_ASM            0
 #define CCN_MUL_ASM            0
 #define CCN_ADDMUL1_ASM        0
 #define CCN_MUL1_ASM           0
 #define CCN_CMP_ASM            0
 #define CCN_ADD1_ASM           0
 #define CCN_SUB1_ASM           0
 #define CCN_N_ASM              0
 #define CCN_SET_ASM            0
 #define CCN_SHIFT_RIGHT_ASM    0
 #define CCN_SHIFT_LEFT_ASM     0
 #define CCN_MOD_224_ASM        0
 #define CCN_MULMOD_256_ASM     0
 #define CCAES_ARM_ASM          0
 #define CCAES_INTEL_ASM        0
 #define CCAES_MUX              0
 #define CCN_USE_BUILTIN_CLZ    0
 #define CCSHA1_VNG_INTEL       0
 #define CCSHA2_VNG_INTEL       0
 #define CCSHA1_VNG_ARM         0
 #define CCSHA2_VNG_ARM         0
 #define CCSHA256_ARMV6M_ASM    0

 #define CC_ACCELERATECRYPTO    0

#endif

#define CC_INLINE static inline

#ifdef __GNUC__
 #define CC_NORETURN __attribute__((__noreturn__))
 #define CC_NOTHROW __attribute__((__nothrow__))
 #define CC_NONNULL(N) __attribute__((__nonnull__ N))
 #define CC_NONNULL4 CC_NONNULL((4))
 #define CC_NONNULL_ALL __attribute__((__nonnull__))
 #define CC_SENTINEL __attribute__((__sentinel__))
 // Only apply the `CC_CONST` attribute to functions with no side-effects where the output is a strict function of pass by value input vars with no exterior side-effects.
 // Specifically, do not apply CC_CONST if the function has any arguments that are pointers (directly, or indirectly)
 #define CC_CONST __attribute__((__const__))
 #define CC_PURE __attribute__((__pure__))
 #define CC_WARN_RESULT __attribute__((__warn_unused_result__))
 #define CC_MALLOC_CLEAR __attribute__((__malloc__))
 #define CC_UNUSED __attribute__((unused))
#else /* !__GNUC__ */
/*! @parseOnly */
 #define CC_UNUSED
/*! @parseOnly */
 #define CC_NONNULL(N)
/*! @parseOnly */
 #define CC_NONNULL4
/*! @parseOnly */
 #define CC_NORETURN
/*! @parseOnly */
 #define CC_NOTHROW
/*! @parseOnly */
 #define CC_NONNULL_ALL
/*! @parseOnly */
 #define CC_SENTINEL
/*! @parseOnly */
 #define CC_CONST
/*! @parseOnly */
 #define CC_PURE
/*! @parseOnly */
 #define CC_WARN_RESULT
/*! @parseOnly */
 #define CC_MALLOC_CLEAR
#endif /* !__GNUC__ */


// Bridge differences between MachO and ELF compiler/assemblers. */
#if CC_USE_ASM
#if CC_LINUX
#define CC_ASM_SECTION_CONST .rodata
#define CC_ASM_PRIVATE_EXTERN .hidden
#define CC_C_LABEL(_sym) _sym
#else /* !CC_LINUX */
#define CC_ASM_SECTION_CONST .const
#define CC_ASM_PRIVATE_EXTERN .private_extern
#define CC_C_LABEL(_sym) _##_sym
#endif /* !CC_LINUX */
#endif /* CC_USE_ASM */


// Enable FIPSPOST function tracing only when supported. */
#ifdef CORECRYPTO_POST_TRACE
#define CC_FIPSPOST_TRACE 1
#else
#define CC_FIPSPOST_TRACE 0
#endif

#endif /* _CORECRYPTO_CC_CONFIG_H_ */
