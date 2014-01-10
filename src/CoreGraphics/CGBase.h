#ifndef CGBASE_H_
#define CGBASE_H_
#include <limits.h>

#ifndef CGFLOAT_DEFINED
#	ifdef __x86_64__
#		define CGFLOAT_IS_DOUBLE 1
#		define CGFLOAT_MIN DBL_MIN
#		define CGFLOAT_MAX DBL_MAX
typedef double CGFloat;
#	else
#		define CGFLOAT_IS_DOUBLE 0
#		define CGFLOAT_MIN FLT_MIN
#		define CGFLOAT_MAX FLT_MAX
typedef float CGFloat;
#	endif
#endif

#endif

