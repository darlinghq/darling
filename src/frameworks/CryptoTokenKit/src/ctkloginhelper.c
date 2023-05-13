#include <ctkloginhelper.h>

CFDictionaryRef TKCopyAvailableTokensInfo(uid_t uid, CFDictionaryRef hints) {
	return NULL;
};

int TKPerformLogin(uid_t uid, CFStringRef token_id, CFDataRef pubkey_hash, CFStringRef pin, CFStringRef kerberos_principal, CFErrorRef* error) {
	return -1;
};

OSStatus TKAddSecureToken(CFStringRef username, CFStringRef pwd, CFStringRef tokenID, CFDataRef wrapPubKeyHash) {
	return -1;
}