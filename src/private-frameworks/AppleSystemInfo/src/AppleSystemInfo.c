#include <AppleSystemInfo/AppleSystemInfo.h>
#include <mach/host_info.h>

CFStringRef ASI_CopyComputerModelName(int arg0) {
	return CFSTR("UNIMPLEMENTED");
}

int ASI_NumberOfCPUs(void) {
	// stub
	return 1;
};
