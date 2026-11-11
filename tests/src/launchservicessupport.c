// Guest test: _LSForceQuitApplication fails loudly instead of reporting a force quit it did not do,
// and the options key it exports is a CFString. Build against /usr/lib/libLaunchServicesSupport.dylib.
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>

extern const CFStringRef _kLSForceQuitApplicationPresentFirstForceQuitDialogKey;
OSStatus _LSForceQuitApplication();

int main(void)
{
	int failures = 0;
	if (CFGetTypeID(_kLSForceQuitApplicationPresentFirstForceQuitDialogKey) != CFStringGetTypeID()) {
		fprintf(stderr, "FAIL: options key is not a CFString\n");
		failures++;
	}
	OSStatus status = _LSForceQuitApplication();
	if (status == noErr) {
		fprintf(stderr, "FAIL: force quit reported success\n");
		failures++;
	}
	printf("%s (status %d)\n", failures ? "FAIL" : "PASS", (int)status);
	return failures != 0;
}
