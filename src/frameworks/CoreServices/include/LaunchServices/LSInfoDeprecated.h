#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 2)

typedef CF_OPTIONS(OptionBits, LSRequestedInfo) {
  kLSRequestExtension = 0x00000001,
  kLSRequestTypeCreator = 0x00000002,
  kLSRequestBasicFlagsOnly = 0x00000004,
  kLSRequestAppTypeFlags = 0x00000008,
  kLSRequestAllFlags = 0x00000010,
  kLSRequestIconAndKind = 0x00000020,
  kLSRequestExtensionFlagsOnly = 0x00000040,
  kLSRequestAllInfo = (UInt32)0xFFFFFFFF
};



typedef CF_OPTIONS(OptionBits, LSItemInfoFlags) {
  kLSItemInfoIsPlainFile = 0x00000001,
  kLSItemInfoIsPackage = 0x00000002,
  kLSItemInfoIsApplication = 0x00000004,
  kLSItemInfoIsContainer = 0x00000008,
  kLSItemInfoIsAliasFile = 0x00000010,
  kLSItemInfoIsSymlink = 0x00000020,
  kLSItemInfoIsInvisible = 0x00000040,
  kLSItemInfoIsNativeApp = 0x00000080,
  kLSItemInfoIsClassicApp = 0x00000100,
  kLSItemInfoAppPrefersNative = 0x00000200,
  kLSItemInfoAppPrefersClassic = 0x00000400,
  kLSItemInfoAppIsScriptable = 0x00000800,
  kLSItemInfoIsVolume = 0x00001000,
  kLSItemInfoExtensionIsHidden = 0x00100000
};

typedef struct LSItemInfoRecord {
  LSItemInfoFlags flags;
  OSType filetype;
  OSType creator;
  CFStringRef extension;
} LSItemInfoRecord;

extern OSStatus
LSCopyItemInfoForURL(
  CFURLRef inURL,
  LSRequestedInfo inWhichInfo,
  LSItemInfoRecord * outItemInfo);

extern OSStatus
LSCopyItemInfoForRef(
  const FSRef * inItemRef,
  LSRequestedInfo inWhichInfo,
  LSItemInfoRecord * outItemInfo);

extern OSStatus
LSGetExtensionInfo(
  UniCharCount inNameLen,
  const UniChar inNameBuffer[],
  UniCharCount * outExtStartIndex);

extern OSStatus
LSCopyDisplayNameForRef(
  const FSRef * inRef,
  CFStringRef * outDisplayName);

extern OSStatus
LSCopyDisplayNameForURL(
  CFURLRef inURL,
  CFStringRef * outDisplayName);

extern OSStatus
LSSetExtensionHiddenForRef(
  const FSRef * inRef,
  Boolean inHide);

extern OSStatus
LSSetExtensionHiddenForURL(
  CFURLRef inURL,
  Boolean inHide);

extern OSStatus
LSCopyKindStringForRef(
  const FSRef * inFSRef,
  CFStringRef * outKindString);

extern OSStatus
LSCopyKindStringForURL(
  CFURLRef inURL,
  CFStringRef * outKindString);

extern OSStatus
LSCopyKindStringForTypeInfo(
  OSType inType,
  OSType inCreator,
  CFStringRef inExtension,
  CFStringRef * outKindString);

extern OSStatus
LSCopyKindStringForMIMEType(
  CFStringRef inMIMEType,
  CFStringRef * outKindString);

extern OSStatus
LSGetApplicationForItem(
  const FSRef * inItemRef,
  LSRolesMask inRoleMask,
  FSRef * outAppRef,
  CFURLRef * outAppURL);

extern OSStatus
LSGetApplicationForInfo(
  OSType inType,
  OSType inCreator,
  CFStringRef inExtension,
  LSRolesMask inRoleMask,
  FSRef * outAppRef,
  CFURLRef * outAppURL);

extern OSStatus
LSCopyApplicationForMIMEType(
  CFStringRef inMIMEType,
  LSRolesMask inRoleMask,
  CFURLRef * outAppURL);

extern OSStatus
LSGetApplicationForURL(
  CFURLRef inURL,
  LSRolesMask inRoleMask,
  FSRef * outAppRef,
  CFURLRef * outAppURL);

extern OSStatus
LSFindApplicationForInfo(
  OSType inCreator,
  CFStringRef inBundleID,
  CFStringRef inName,
  FSRef * outAppRef,
  CFURLRef * outAppURL);

extern OSStatus
LSCanRefAcceptItem(
  const FSRef * inItemFSRef,
  const FSRef * inTargetRef,
  LSRolesMask inRoleMask,
  LSAcceptanceFlags inFlags,
  Boolean * outAcceptsItem);

extern OSStatus
LSRegisterFSRef(
  const FSRef * inRef,
  Boolean inUpdate);

extern const CFStringRef kLSItemContentType;
extern const CFStringRef kLSItemFileType;
extern const CFStringRef kLSItemFileCreator;
extern const CFStringRef kLSItemExtension;
extern const CFStringRef kLSItemDisplayName;
extern const CFStringRef kLSItemDisplayKind;
extern const CFStringRef kLSItemRoleHandlerDisplayName;
extern const CFStringRef kLSItemIsInvisible;
extern const CFStringRef kLSItemExtensionIsHidden;
extern const CFStringRef kLSItemQuarantineProperties;

extern OSStatus
LSCopyItemAttribute(
  const FSRef * inItem,
  LSRolesMask inRoles,
  CFStringRef inAttributeName,
  CFTypeRef * outValue);

extern OSStatus
LSCopyItemAttributes(
  const FSRef * inItem,
  LSRolesMask inRoles,
  CFArrayRef inAttributeNames,
  CFDictionaryRef * outValues);

extern OSStatus
LSSetItemAttribute(
  const FSRef * inItem,
  LSRolesMask inRoles,
  CFStringRef inAttributeName,
  CFTypeRef inValue);

typedef CF_OPTIONS(OptionBits, LSHandlerOptions) {
  kLSHandlerOptionsDefault = 0,
  kLSHandlerOptionsIgnoreCreator = 1
};

extern LSHandlerOptions
LSGetHandlerOptionsForContentType(CFStringRef inContentType);

extern OSStatus
LSSetHandlerOptionsForContentType(
  CFStringRef inContentType,
  LSHandlerOptions inOptions);



#pragma pack(pop)

#ifdef __cplusplus
}
#endif

