#ifndef LAUNCHSERVICES_H
#define LAUNCHSERVICES_H

#include <CoreServices/MacTypes.h>
#include <CoreServices/FileManager.h>
#include <CoreServices/Processes.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFURL.h>

#include <AE/AE.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef OptionBits LSInitializeFlags;
typedef OptionBits LSLaunchFlags;
typedef int LSNotificationCode;
typedef int LSNotificationID;
typedef int LSASN;
typedef CFTypeRef LSASNRef;
typedef int LSSessionID;

#define kLSNotificationInvalidID -1
#define kLSNotifyApplicationDeath 1
#define kLSUnknownErr -1

#define kLSDefaultSessionID 1

enum
{
	kLSLaunchDefaults = 0x00000001,
	kLSLaunchAndPrint = 0x00000002,
	kLSLaunchReserved2 = 0x00000004,
	kLSLaunchReserved3 = 0x00000008,
	kLSLaunchReserved4 = 0x00000010,
	kLSLaunchReserved5 = 0x00000020,
	kLSLaunchAndDisplayErrors = 0x00000040,
	kLSLaunchInhibitBGOnly = 0x00000080,
	kLSLaunchDontAddToRecents = 0x00000100,
	kLSLaunchDontSwitch = 0x00000200,
	kLSLaunchNoParams = 0x00000800,
	kLSLaunchAsync = 0x00010000,
	kLSLaunchStartClassic = 0x00020000,
	kLSLaunchInClassic = 0x00040000,
	kLSLaunchNewInstance = 0x00080000,
	kLSLaunchAndHide = 0x00100000,
	kLSLaunchAndHideOthers = 0x00200000,
	kLSLaunchHasUntrustedContents = 0x00400000
};

typedef struct LSApplicationParameters
{
	CFIndex version;
	LSLaunchFlags flags;
	const FSRef * application;
	void * asyncLaunchRefCon;
	CFDictionaryRef environment;
	CFArrayRef argv;
	AppleEvent * initialEvent;
} LSApplicationParameters;

OSStatus LSInit(LSInitializeFlags flags);
OSStatus LSTerm(void);

OSStatus LSOpenApplication(const LSApplicationParameters *appParams, ProcessSerialNumber *outPSN);
OSStatus LSOpenCFURLRef(CFURLRef inURL, CFURLRef *outLaunchedURL);
OSStatus LSOpenFSRef(const FSRef *inRef, FSRef *outLaunchedRef);

OSStatus LSGetExtensionInfo(UniCharCount inNameLen, const UniChar* inNameBuffer, UniCharCount *outExtStartIndex);
OSStatus LSSetExtensionHiddenForRef(const FSRef *inRef, Boolean hide);
OSStatus LSSetExtensionHiddenForURL(CFURLRef inURL, Boolean hide);

extern CFStringRef _kLSBundlePathKey;
extern CFStringRef _kLSDisplayNameKey;
extern CFStringRef _kLSExecutablePathKey;

CFTypeRef _LSCopyApplicationInformationItem(int /* hopefully */, CFTypeRef asn, CFStringRef what);

CFDictionaryRef _LSCopyApplicationInformation(int, CFTypeRef asn, int);

LSASNRef _LSASNCreateWithPid(CFAllocatorRef allocator, pid_t pid);

extern CFStringRef LSUserApplicationType, LSSystemApplicationType, LSInternalApplicationType, LSPlugInKitType, LSVPNPluginType;

//OSStatus _LSOpenURLsWithRole(CFArrayRef inURLs, LSRolesMask inRole, void *unknown);

// TODO: many other functions

#ifdef __cplusplus
}
#endif

#endif

