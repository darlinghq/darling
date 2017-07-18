#ifndef CORESERVICESPRIV_H_
#define CORESERVICESPRIV_H_

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURLPriv.h>

CF_EXTERN_C_BEGIN

Boolean _CSCheckFix(CFStringRef str);

// Assumed
#define kLSDownloadRiskCategoryKey CFSTR("LSDownloadRiskCategory")

#define kLSRiskCategorySafe "LSRiskCategorySafe"
#define kLSRiskCategoryNeutral "LSRiskCategoryNeutral"
#define kLSRiskCategoryUnknown "LSRiskCategoryUnknown"
#define kLSRiskCategoryMayContainUnsafeExecutable "LSRiskCategoryMayContainUnsafeExecutable"

// Also assumed
#define QTN_FLAG_ASSESSMENT_OK 1

CF_EXTERN_C_END

#endif
