// Guest test: AOSKit reports that no iCloud account exists: no account is created, no account info
// is retrieved, and a missing transaction reports no success, result or error. Exits non-zero on failure.
#include <AOSKit/AOSKit.h>
#include <stdio.h>

int main(void)
{
	int failures = 0;
	CFTypeRef account = AOSAccountCreate(kCFAllocatorDefault, NULL);
	if (account != NULL) {
		fprintf(stderr, "FAIL: an account was created\n");
		failures++;
	}
	CFTypeRef transaction = _AOSAccountRetrieveInfo(account, NULL, NULL);
	if (transaction != NULL) {
		fprintf(stderr, "FAIL: account info was retrieved\n");
		failures++;
	}
	if (AOSTransactionSuccessful(transaction) || AOSTransactionGetResult(transaction) != NULL ||
	    AOSTransactionGetError(transaction) != NULL) {
		fprintf(stderr, "FAIL: a missing transaction reported a state\n");
		failures++;
	}
	const CFStringRef keys[] = {kAOSAppleAccountInfoKey, kAOSErrorDomain, kAOSMMeInfoKey,
				    kAOSPersonIDKey, kAOSTokensKey, kAOSURLKey};
	for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
		if (CFGetTypeID(keys[i]) != CFStringGetTypeID() || CFStringGetLength(keys[i]) == 0) {
			fprintf(stderr, "FAIL: key %zu is not a non-empty CFString\n", i);
			failures++;
		}
	}
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}
