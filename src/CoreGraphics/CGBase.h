#ifndef CGBASE_H_
#define CGBASE_H_
#include <limits.h>

#ifndef CGFLOAT_DEFINED
#	ifdef __x86_64__
#		define CGFLOAT_IS_DOUBLE 1
#		define CGFLOAT_MIN DBL_MIN
#		define CGFLOAT_MAX DBL_MAX
#		ifdef DARLING_BUILD
#			define CGFLOAT_EPSILON DBL_EPSILON
#		endif
typedef double CGFloat;
#	else
#		define CGFLOAT_IS_DOUBLE 0
#		define CGFLOAT_MIN FLT_MIN
#		define CGFLOAT_MAX FLT_MAX
#		ifdef DARLING_BUILD
#			define CGFLOAT_EPSILON FLT_EPSILON
#		endif
typedef float CGFloat;
#	endif
#endif

#ifndef BEGIN_EXTERN_C
#   ifdef __cplusplus
#       define BEGIN_EXTERN_C extern "C" {
#       define END_EXTERN_C }
#   else
#       define BEGIN_EXTERN_C
#       define END_EXTERN_C
#   endif
#endif


#endif

