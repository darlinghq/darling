// Guest test: RemoteServiceDiscovery reports no remote devices.
// Build against RemoteServiceDiscovery.framework and run inside Darling; exits non-zero on failure.
#include <RemoteServiceDiscovery/RemoteServiceDiscovery.h>
#include <stdio.h>

int main(void)
{
	int failures = 0;
	for (int type = 0; type < 8; type++) {
		if (remote_device_copy_unique_of_type(type) != NULL) {
			fprintf(stderr, "FAIL: a remote device of type %d was reported\n", type);
			failures++;
		}
	}
	if (remote_device_copy_property(NULL, "UniqueDeviceID") != NULL) {
		fprintf(stderr, "FAIL: a property was returned without a device\n");
		failures++;
	}
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}
