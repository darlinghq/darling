#include <LocalAuthentication/LACFSupport.h>

CFTypeRef LACreateNewContextWithACMContext(CFDataRef acm_context, CFErrorRef *error) {
	return NULL;
}

int LAEvaluateAndUpdateACL(CFTypeRef auth_handle, CFDataRef data, CFDataRef data2, CFDictionaryRef dict, CFDataRef *updated_acl, CFErrorRef *error) {
	return 0;
}

CFDataRef LACopyACMContext(CFTypeRef auth_handle, CFErrorRef *error) {
	return NULL;
}

// Darling has no coreauthd and no local authentication contexts, so there is
// never a context to continue a sheet authentication in, a credential to
// extract or an evaluation result to return.
CFTypeRef LACreateNewContextWithACMContextInSession(uid_t uid, CFDataRef acm_context, CFErrorRef *error) {
	return NULL;
}

CFDataRef LACopyCredential(CFTypeRef context, int credentialType, CFErrorRef *error) {
	return NULL;
}

CFDictionaryRef LACopyResultOfPolicyEvaluation(CFTypeRef context, int policy, CFDictionaryRef options, CFErrorRef *error) {
	return NULL;
}
