#ifndef APPKIT_DEFINES_H
#define APPKIT_DEFINES_H

#define NS_BLOCKS_AVAILABLE 1 /* Blocks are required for Darling */

typedef void* CGImageRef;
typedef long long NSAlignmentOptions; /* This should be in Foundation */

#ifndef BEGIN_EXTERN_C
#	ifdef __cplusplus
#		define BEGIN_EXTERN_C extern "C" {
#		define END_EXTERN_C }
#	else
#		define BEGIN_EXTERN_C
#		define END_EXTERN_C
#	endif
#endif
#ifndef EXTERN_C
#	ifdef __cplusplus
#		define EXTERN_C extern "C"
#	else
#		define EXTERN_C extern
#	endif
#endif

#ifdef DARLING_BUILD
#	define DEFINE_CONSTANT(name) NSString* name = @ #name
#endif

#endif

