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
extern const CFStringRef _kLSPersistenceSuppressRelaunchAtLoginKey;

extern const CFStringRef _kLSArchitectureKey;

extern const CFStringRef _kLSPIDKey;

// Key of the options dictionary of the private open calls: a true value opens
// the item without bringing the application to the front.
extern const CFStringRef _kLSOpenOptionBackgroundLaunchKey;
extern const CFStringRef _kLSExecutablePathKey;
extern const CFStringRef _kLSCheckInTimeKey;
extern const CFStringRef _kLSDisplayNameKey;
extern const CFStringRef _kLSArchitectureKey;
extern const CFStringRef _kLSBundlePathKey;
extern const CFStringRef _kLSApplicationTypeKey;

extern const CFStringRef _kLSASNKey;

extern const CFStringRef _kLSApplicationForegroundTypeKey;
extern const CFStringRef _kLSApplicationBackgroundOnlyTypeKey;
extern const CFStringRef _kLSApplicationUIElementTypeKey;

extern const CFStringRef _kLSExecutableFormatKey;
extern const CFStringRef _kLSExecutableFormatMachOKey;
extern const CFStringRef _kLSExecutableFormatCFMKey;

// Example call:
// _LSApplicationCheckIn(kLSDefaultSessionID, CFBundleGetInfoDictionary(CFBundleGetMainBundle()))
// Normally to be called from -[NSApplication init]
CFDictionaryRef _LSApplicationCheckIn(LSSessionID sessionID, CFDictionaryRef applicationInfo);

CFTypeRef _LSGetCurrentApplicationASN(void);
// Only the application type (_kLSApplicationTypeKey) of the calling process
// can be changed; other processes and keys return unimpErr. There is no
// registry of other processes' attributes, so the information calls below
// report what can be derived from the process itself (PID, executable path,
// name) for any live process, and the application type for this process.
OSStatus _LSSetApplicationInformationItem(LSSessionID sessionID, LSASNRef asn, CFStringRef key, CFStringRef value, CFDictionaryRef *info);

// Returns the ASN of the application that currently has focus, or NULL when
// no such application exists (or has never been tracked).
LSASNRef _LSCopyFrontApplication(LSSessionID sessionID);

typedef bool (^LSServerConnectionAllowedBlock) ( CFDictionaryRef optionsRef );
void _LSSetApplicationLaunchServicesServerConnectionStatus(uint64_t flags, LSServerConnectionAllowedBlock block);

CFTypeRef _LSCopyApplicationInformationItem(LSSessionID sessionID, CFTypeRef asn, CFStringRef what);

CFDictionaryRef _LSCopyApplicationInformation(LSSessionID sessionID, CFTypeRef asn, int);

LSASNRef _LSASNCreateWithPid(CFAllocatorRef allocator, pid_t pid);

OSStatus _LSLaunchApplication(CFURLRef appPath);

// Shows a file or folder in the host desktop's file manager (the folder containing it with the item selected when
// reveal is set, otherwise the item opened). No file manager lives in the prefix, so this reaches the host through the
// desktop portal; kLSApplicationNotFoundErr when it cannot.
OSStatus _LSOpenFileOnHost(CFURLRef fileURL, Boolean reveal);

Boolean _LSASNExtractHighAndLowParts(LSASNRef asn, UInt32* outHigh, UInt32* outLow);
// ASNs of all live processes, ordered by PID.
CFArrayRef _LSCopyApplicationArray(LSSessionID sessionID);
OSStatus _LSSetWeakBindingURLForType(LSSessionID sessionID, int unknown, CFStringRef contentType, LSRolesMask roles, CFURLRef appURL);

// The URL of the icon file that shows documents of a content type, or NULL when no application declares one
// (the caller then uses a generic icon). Looks at the icon of the type's own declaration (UTTypeIconFile), then at the
// icon in CFBundleDocumentTypes of the handler application (the user's choice, else by handler rank). Follows the
// Create rule. Only the file is located; callers load it as they see fit.
CFURLRef _LSCopyIconURLForContentType(CFStringRef contentType);

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
