/*
 *  cc_macros.h
 *  corecrypto
 *
 *  Created on 01/11/2012
 *
 *  Copyright (c) 2012,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CC_MACROS_H_
#define _CORECRYPTO_CC_MACROS_H_

#include <corecrypto/cc_config.h>

#ifndef __CC_DEBUG_ASSERT_COMPONENT_NAME_STRING
#define __CC_DEBUG_ASSERT_COMPONENT_NAME_STRING ""
#endif

#ifndef __CC_DEBUG_ASSERT_PRODUCTION_CODE
#define __CC_DEBUG_ASSERT_PRODUCTION_CODE !CORECRYPTO_DEBUG
#endif

#if CORECRYPTO_DEBUG_ENABLE_CC_REQUIRE_PRINTS

#if !CC_KERNEL
    #include <string.h> // for strstr
#endif // !CC_KERNEL

CC_UNUSED static char *cc_strstr(const char *file) {
#if CC_KERNEL
    (void) file;
#else
    const char cc_char []="corecrypto";
    char *p=strstr(file, cc_char);
    if (p) return (p+strlen(cc_char)+1);
#endif
    return NULL;
}

#define __CC_DEBUG_REQUIRE_MESSAGE(name, assertion, label, message, file, line, value) \
{char *___t = cc_strstr(file); cc_printf( "require: %s, %s%s:%d\n", assertion, (message!=0) ? message : "", ___t==NULL?file:___t, line);}

#endif // CORECRYPTO_DEBUG_ENABLE_CC_REQUIRE_PRINTS

#ifndef cc_require
#if (__CC_DEBUG_ASSERT_PRODUCTION_CODE) || (!CORECRYPTO_DEBUG_ENABLE_CC_REQUIRE_PRINTS)
  #if defined(_WIN32) && defined (__clang__)
    #define cc_require(assertion, exceptionLabel) \
       do { \
           if (!(assertion) ) { \
              goto exceptionLabel; \
           } \
        } while ( 0 )
  #else
    #define cc_require(assertion, exceptionLabel) \
        do { \
            if ( __builtin_expect(!(assertion), 0) ) { \
                goto exceptionLabel; \
            } \
        } while ( 0 )
 #endif
#else
    #define cc_require(assertion, exceptionLabel) \
        do { \
            if ( __builtin_expect(!(assertion), 0) ) { \
                __CC_DEBUG_REQUIRE_MESSAGE(__CC_DEBUG_ASSERT_COMPONENT_NAME_STRING, \
                    #assertion, #exceptionLabel, 0, __FILE__, __LINE__,  0); \
                goto exceptionLabel; \
            } \
        } while ( 0 )
#endif
#endif

#ifndef cc_require_action
#if __CC_DEBUG_ASSERT_PRODUCTION_CODE || (!CORECRYPTO_DEBUG_ENABLE_CC_REQUIRE_PRINTS)
  #if defined(_WIN32) && defined(__clang__)
    #define cc_require_action(assertion, exceptionLabel, action)                \
        do                                                                      \
        {                                                                       \
            if (!(assertion))                                                   \
            {                                                                   \
                {                                                               \
                    action;                                                     \
                }                                                               \
                goto exceptionLabel;                                            \
            }                                                                   \
        } while ( 0 )
  #else
    #define cc_require_action(assertion, exceptionLabel, action)                \
        do                                                                      \
        {                                                                       \
            if ( __builtin_expect(!(assertion), 0) )                            \
            {                                                                   \
                {                                                               \
                    action;                                                     \
                }                                                               \
                goto exceptionLabel;                                            \
            }                                                                   \
        } while ( 0 )
  #endif
#else
    #define cc_require_action(assertion, exceptionLabel, action)                \
        do                                                                      \
        {                                                                       \
            if ( __builtin_expect(!(assertion), 0) )                            \
            {                                                                   \
                __CC_DEBUG_REQUIRE_MESSAGE(                                      \
                    __CC_DEBUG_ASSERT_COMPONENT_NAME_STRING,                    \
                    #assertion, #exceptionLabel, 0,   __FILE__, __LINE__, 0);   \
                {                                                               \
                    action;                                                     \
                }                                                               \
                goto exceptionLabel;                                            \
            }                                                                   \
        } while ( 0 )
#endif
#endif

#endif /* _CORECRYPTO_CC_MACROS_H_ */
