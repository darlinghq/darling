#include <CoreServices/CoreServicesPriv.h>
#include <mach/machine.h>

// TODO: Security.framework expects this symbol to be available in
// /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/CarbonCore

Boolean _CSCheckFix(CFStringRef str)
{
	// Checks whether given issue number is fixed in this release or something
	return 0;
}

// No fix is ever reported as applying (see _CSCheckFix()), so there is nothing
// left to turn off. The call returns nothing and has no other effect.
// Test: coreservices_spi.c
void _CSCheckFixDisable(void)
{
}

// Returns the CPU type of the running process, as in <mach/machine.h>.
int _CSGetProcessorArchitecture(void)
{
#if defined(__x86_64__)
	return CPU_TYPE_X86_64;
#elif defined(__i386__)
	return CPU_TYPE_I386;
#elif defined(__arm64__) || defined(__aarch64__)
	return CPU_TYPE_ARM64;
#else
#error Unsupported architecture
#endif
}
