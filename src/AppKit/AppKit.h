#ifndef APPKIT_H_
#define APPKIT_H_

#ifndef BEGIN_EXTERN_C
#	ifdef __cplusplus
#		define BEGIN_EXTERN_C extern "C" {
#		define END_EXTERN_C }
#	else
#		define BEGIN_EXTERN_C
#		define END_EXTERN_C
#	endif
#endif

#ifdef DARLING_BUILD
#	define DEFINE_CONSTANT(name) NSString* name = @ #name
#endif

#include "NSApplication.h"

#endif
