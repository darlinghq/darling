#ifndef _LACFSUPPORT_H_
#define _LACFSUPPORT_H_

#include <CoreFoundation/CoreFoundation.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

CF_EXPORT
CFTypeRef LACreateNewContextWithACMContext(CFDataRef acm_context, CFErrorRef *error);

CF_EXPORT
int LAEvaluateAndUpdateACL(CFTypeRef auth_handle, CFDataRef data, CFDataRef data2, CFDictionaryRef dict, CFDataRef *updated_acl, CFErrorRef *error);

CF_EXPORT
CFDataRef LACopyACMContext(CFTypeRef auth_handle, CFErrorRef *error);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif
