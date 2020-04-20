#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 2)

enum {
  kLSLaunchInhibitBGOnly= 0x00000080,
  kLSLaunchNoParams = 0x00000800,
  kLSLaunchStartClassic = 0x00020000,
  kLSLaunchInClassic = 0x00040000,
  kLSLaunchHasUntrustedContents = 0x00400000,
};


typedef struct LSLaunchFSRefSpec {
  const FSRef * appRef;
  ItemCount numDocs;
  const FSRef * itemRefs;
  const AEDesc * passThruParams;

  LSLaunchFlags launchFlags;
  void * asyncRefCon;
} LSLaunchFSRefSpec;

extern OSStatus
LSOpenFSRef(
  const FSRef * inRef,
  FSRef * outLaunchedRef);

extern OSStatus
LSOpenFromRefSpec(
  const LSLaunchFSRefSpec * inLaunchSpec,
  FSRef * outLaunchedRef);

typedef struct LSApplicationParameters {
  CFIndex version;
  LSLaunchFlags flags;
  const FSRef * application;
  void * asyncLaunchRefCon;
  CFDictionaryRef environment;

  CFArrayRef argv;

  AppleEvent * initialEvent;
} LSApplicationParameters;

extern OSStatus
LSOpenApplication(
  const LSApplicationParameters * appParams,
  ProcessSerialNumber * outPSN);

extern OSStatus
LSOpenItemsWithRole(
  const FSRef * inItems,
  CFIndex inItemCount,
  LSRolesMask inRole,
  const AEKeyDesc * inAEParam,
  const LSApplicationParameters * inAppParams,
  ProcessSerialNumber * outPSNs,
  CFIndex inMaxPSNCount);

extern OSStatus
LSOpenURLsWithRole(
  CFArrayRef inURLs,
  LSRolesMask inRole,
  const AEKeyDesc * inAEParam,
  const LSApplicationParameters * inAppParams,
  ProcessSerialNumber * outPSNs,
  CFIndex inMaxPSNCount);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

