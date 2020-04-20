#ifndef __LSOPEN__
#define __LSOPEN__

#include <CoreFoundation/CoreFoundation.h>
#include <LaunchServices/LSInfo.h>
#include <CoreServices/FileManager.h>
#include <AE/AE.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 2)
typedef OptionBits LSLaunchFlags;

enum {
  kLSLaunchDefaults = 0x00000001,
  kLSLaunchAndPrint = 0x00000002,
  kLSLaunchAndDisplayErrors = 0x00000040,
  kLSLaunchDontAddToRecents = 0x00000100,
  kLSLaunchDontSwitch = 0x00000200,
  kLSLaunchAsync = 0x00010000,
  kLSLaunchNewInstance = 0x00080000,
  kLSLaunchAndHide = 0x00100000,
  kLSLaunchAndHideOthers = 0x00200000,
};

typedef struct LSLaunchURLSpec {
  _Nullable CFURLRef appURL;
  _Nullable CFArrayRef itemURLs;
  const AEDesc * _Nullable passThruParams;
  LSLaunchFlags launchFlags;
  void * _Nullable asyncRefCon;
} LSLaunchURLSpec;

extern OSStatus
LSOpenCFURLRef(
  CFURLRef inURL,
  _Nullable CFURLRef *_Nullable outLaunchedURL);

extern OSStatus
LSOpenFromURLSpec(
  const LSLaunchURLSpec * inLaunchSpec,
  _Nullable CFURLRef *_Nullable outLaunchedURL);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#include <LaunchServices/LSOpenDeprecated.h>

#endif
