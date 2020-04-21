#ifndef LAUNCH_SERVICES_PRIV_H_
#define LAUNCH_SERVICES_PRIV_H_
#include <LaunchServices/LaunchServices.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef CFTypeRef LSASNRef;
typedef int LSSessionID;
typedef int LSNotificationCode;
typedef int LSNotificationID;
typedef OptionBits LSInitializeFlags;

enum
{
	kLSDefaultSessionID = -2,
};

// Used in Security
enum
{
	kLSNotificationInvalidID = -1,
	kLSNotifyApplicationDeath = 1,
};

enum
{
	/***** MASTER SERVICE COMMANDS *****/
	// IN:
	// clientversion, session
	// OUT:
	// serverversion, endpoint
	_kLSXPCHello = 0x64,

	/***** ENDPOINT COMMANDS *****/
	// OUT:
	// bool success
	// dictionary result
	_kLSXPCWeirdCheckIn = 0x1F9,
	// IN:
	// dictionary info
	// mach_port_name_t deathport
	// OUT:
	// bool success
	// dictionary result (appInfo)
	_kLSXPCCheckIn = 0x1F4,
};

extern const CFStringRef _kLSArchitecturePowerPCValue;
extern const CFStringRef _kLSArchitecturePowerPC64Value;
extern const CFStringRef _kLSArchitecturei386Value;
extern const CFStringRef _kLSArchitecturex86_64Value;
extern const CFStringRef _kLSArchitectureUnknownValue;

extern const CFStringRef _kLSArchitectureKey;

extern const CFStringRef _kLSPIDKey;
extern const CFStringRef _kLSExecutablePathKey;
extern const CFStringRef _kLSCheckInTimeKey;
extern const CFStringRef _kLSDisplayNameKey;
extern const CFStringRef _kLSArchitectureKey;
extern const CFStringRef _kLSBundlePathKey;
extern const CFStringRef _kLSApplicationTypeKey;

extern const CFStringRef _kLSASNKey;

extern const CFStringRef _kLSApplicationForegroundTypeKey;
extern const CFStringRef _kLSApplicationBackgroundOnlyTypeKey;

extern const CFStringRef _kLSExecutableFormatKey;
extern const CFStringRef _kLSExecutableFormatMachOKey;
extern const CFStringRef _kLSExecutableFormatCFMKey;

// Example call:
// _LSApplicationCheckIn(kLSDefaultSessionID, CFBundleGetInfoDictionary(CFBundleGetMainBundle()))
// Normally to be called from -[NSApplication init]
CFDictionaryRef _LSApplicationCheckIn(LSSessionID sessionID, CFDictionaryRef applicationInfo);

CFTypeRef _LSGetCurrentApplicationASN(void);
OSStatus _LSSetApplicationInformationItem(LSSessionID sessionID, LSASNRef asn, CFStringRef key, CFStringRef value, CFDictionaryRef *info);

typedef bool (^LSServerConnectionAllowedBlock) ( CFDictionaryRef optionsRef );
void _LSSetApplicationLaunchServicesServerConnectionStatus(uint64_t flags, LSServerConnectionAllowedBlock block);

CFTypeRef _LSCopyApplicationInformationItem(LSSessionID sessionID, CFTypeRef asn, CFStringRef what);

CFDictionaryRef _LSCopyApplicationInformation(LSSessionID sessionID, CFTypeRef asn, int);

LSASNRef _LSASNCreateWithPid(CFAllocatorRef allocator, pid_t pid);

OSStatus _LSLaunchApplication(CFURLRef appPath);

// Assumed
#define kLSDownloadRiskCategoryKey CFSTR("LSDownloadRiskCategory")

#define kLSRiskCategorySafe "LSRiskCategorySafe"
#define kLSRiskCategoryNeutral "LSRiskCategoryNeutral"
#define kLSRiskCategoryUnknown "LSRiskCategoryUnknown"
#define kLSRiskCategoryMayContainUnsafeExecutable "LSRiskCategoryMayContainUnsafeExecutable"

#ifdef __cplusplus
}
#endif

#endif
