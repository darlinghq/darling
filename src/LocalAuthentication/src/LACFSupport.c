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
