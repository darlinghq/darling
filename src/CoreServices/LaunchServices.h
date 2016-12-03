#ifndef LAUNCHSERVICES_H
#define LAUNCHSERVICES_H
#include "MacTypes.h"
#include "FileManager.h"
#include "Processes.h"
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFURL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef OptionBits LSInitializeFlags;
typedef OptionBits LSLaunchFlags;
typedef void AppleEvent;

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

struct LSApplicationParameters
{
	CFIndex version;
	LSLaunchFlags flags;
	const FSRef * application;
	void * asyncLaunchRefCon;
	CFDictionaryRef environment;
	CFArrayRef argv;
	AppleEvent * initialEvent;
};

OSStatus LSInit(LSInitializeFlags flags);
OSStatus LSTerm(void);

OSStatus LSOpenApplication(const LSApplicationParameters *appParams, ProcessSerialNumber *outPSN);
OSStatus LSOpenCFURLRef(CFURLRef inURL, CFURLRef *outLaunchedURL);
OSStatus LSOpenFSRef(const FSRef *inRef, FSRef *outLaunchedRef);

OSStatus LSGetExtensionInfo(UniCharCount inNameLen, const UniChar* inNameBuffer, UniCharCount *outExtStartIndex);
OSStatus LSSetExtensionHiddenForRef(const FSRef *inRef, Boolean hide);
OSStatus LSSetExtensionHiddenForURL(CFURLRef inURL, Boolean hide);

// TODO: many other functions

#ifdef __cplusplus
}
#endif

#endif

