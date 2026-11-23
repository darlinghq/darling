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

/* Context in a user session, used by authd for sheet based authentication. */
CF_EXPORT
CFTypeRef LACreateNewContextWithACMContextInSession(uid_t uid, CFDataRef acm_context, CFErrorRef *error);

/* Credential held by a context (for example the extractable passcode), NULL if it holds none. */
CF_EXPORT
CFDataRef LACopyCredential(CFTypeRef context, int credentialType, CFErrorRef *error);

/* Result of a previous policy evaluation of a context, NULL if there is none. */
CF_EXPORT
CFDictionaryRef LACopyResultOfPolicyEvaluation(CFTypeRef context, int policy, CFDictionaryRef options, CFErrorRef *error);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif
