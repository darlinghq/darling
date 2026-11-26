/* SPDX-License-Identifier: GPL-3.0-or-later */
// Number formatting from the Text Utilities (NumberFormatting.h in Apple's CarbonCore).
#ifndef _CarbonCore_NumberFormatting_H_
#define _CarbonCore_NumberFormatting_H_
#include <CoreServices/MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !__LP64__
void NumToString(long theNum, Str255 theString);
void StringToNum(ConstStr255Param theString, long* theNum);
#endif

#ifdef __cplusplus
}
#endif

#endif
