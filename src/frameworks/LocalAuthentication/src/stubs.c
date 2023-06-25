#include <stddef.h>
#include <LocalAuthentication/LAPrivateDefines.h>

bool LAEvaluatePolicy(CFTypeRef context, int policy, CFMutableDictionaryRef options, CFErrorRef *error) {
	return false;
};

bool LAVerifySEP(uid_t pw_uid, CFErrorRef *error) {
	return false;
};
