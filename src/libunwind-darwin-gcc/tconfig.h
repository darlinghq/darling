#define ATTRIBUTE_UNUSED
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mcore.h"
#define SHARED


/*
#define _Unwind_GetTextRelBase __darwin__Unwind_GetTextRelBase
#define _Unwind_GetDataRelBase __darwin__Unwind_GetDataRelBase
#define _Unwind_GetRegionStart __darwin__Unwind_GetRegionStart
#define _Unwind_GetGR __darwin__Unwind_GetGR
#define _Unwind_GetCFA __darwin__Unwind_GetCFA
#define _Unwind_SetGR __darwin__Unwind_SetGR
#define _Unwind_GetIP __darwin__Unwind_GetIP
#define _Unwind_GetIPInfo __darwin__Unwind_GetIPInfo
#define _Unwind_SetIP __darwin__Unwind_SetIP
#define _Unwind_GetLanguageSpecificData __darwin__Unwind_GetLanguageSpecificData
#define _Unwind_FindEnclosingFunction __darwin__Unwind_FindEnclosingFunction
#define _Unwind_Find_FDE __darwin__Unwind_Find_FDE
*/

#define _Unwind_RaiseException __darwin__Unwind_RaiseException
/* #define _Unwind_ForcedUnwind __darwin__Unwind_ForcedUnwind */
#define _Unwind_Resume __darwin__Unwind_Resume
#define _Unwind_Resume_or_Rethrow __darwin__Unwind_Resume_or_Rethrow
#define _Unwind_DeleteException __darwin__Unwind_DeleteException
/* #define _Unwind_Backtrace __darwin__Unwind_Backtrace */
/* #define _Unwind_Find_FDE __darwin__Unwind_Find_FDE */

#define gcc_assert assert
#define gcc_unreachable() __builtin_unreachable()
