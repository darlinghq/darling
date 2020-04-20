#ifndef __LSINFO__
#define __LSINFO__

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/MacTypes.h>
#include <CoreServices/FileManager.h>

#ifdef __cplusplus
extern "C" {
#endif


#pragma pack(push, 2)
enum {
  kLSAppInTrashErr = -10660,
  kLSExecutableIncorrectFormat = -10661,
  kLSAttributeNotFoundErr = -10662,
  kLSAttributeNotSettableErr = -10663,
  kLSIncompatibleApplicationVersionErr = -10664,
  kLSNoRosettaEnvironmentErr = -10665,
  kLSGarbageCollectionUnsupportedErr = -10666,
  kLSUnknownErr = -10810,
  kLSNotAnApplicationErr = -10811,
  kLSNotInitializedErr = -10812,
  kLSDataUnavailableErr = -10813,
  kLSApplicationNotFoundErr = -10814,
  kLSUnknownTypeErr = -10815,
  kLSDataTooOldErr = -10816,
  kLSDataErr = -10817,
  kLSLaunchInProgressErr = -10818,
  kLSNotRegisteredErr = -10819,
  kLSAppDoesNotClaimTypeErr = -10820,
  kLSAppDoesNotSupportSchemeWarning = -10821,
  kLSServerCommunicationErr = -10822,
  kLSCannotSetInfoErr = -10823,
  kLSNoRegistrationInfoErr = -10824,
  kLSIncompatibleSystemVersionErr = -10825,
  kLSNoLaunchPermissionErr = -10826,
  kLSNoExecutableErr = -10827,
  kLSNoClassicEnvironmentErr = -10828,
  kLSMultipleSessionsNotSupportedErr = -10829,
};

typedef OptionBits LSRolesMask;

enum {
  kLSRolesNone = 0x00000001,
  kLSRolesViewer = 0x00000002,
  kLSRolesEditor = 0x00000004,
  kLSRolesShell = 0x00000008,
  kLSRolesAll = (UInt32)0xFFFFFFFF
};

enum {
  kLSUnknownType = 0,
  kLSUnknownCreator = 0
};

typedef OptionBits LSAcceptanceFlags;
enum {
  kLSAcceptDefault = 0x00000001,
  kLSAcceptAllowLoginUI = 0x00000002
};

extern _Nullable CFURLRef
LSCopyDefaultApplicationURLForURL(
  CFURLRef inURL,
  LSRolesMask inRoleMask,
  _Nullable CFErrorRef *_Nullable outError);

extern _Nullable CFURLRef
LSCopyDefaultApplicationURLForContentType(
  CFStringRef inContentType,
  LSRolesMask inRoleMask,
  _Nullable CFErrorRef *_Nullable outError);

extern _Nullable CFArrayRef
LSCopyApplicationURLsForBundleIdentifier(
  CFStringRef inBundleIdentifier,
  _Nullable CFErrorRef *_Nullable outError);

extern _Nullable CFArrayRef
LSCopyApplicationURLsForURL(
  CFURLRef inURL,
  LSRolesMask inRoleMask);

extern OSStatus
LSCanURLAcceptURL(
  CFURLRef inItemURL,
  CFURLRef inTargetURL,
  LSRolesMask inRoleMask,
  LSAcceptanceFlags inFlags,
  Boolean * outAcceptsItem);

extern OSStatus
LSRegisterURL(
  CFURLRef inURL,
  Boolean inUpdate);

extern _Nullable CFStringRef
LSCopyDefaultRoleHandlerForContentType(
  CFStringRef inContentType,
  LSRolesMask inRole);

extern _Nullable CFArrayRef
LSCopyAllRoleHandlersForContentType(
  CFStringRef inContentType,
  LSRolesMask inRole);

extern OSStatus
LSSetDefaultRoleHandlerForContentType(
  CFStringRef inContentType,
  LSRolesMask inRole,
  CFStringRef inHandlerBundleID);

extern _Nullable CFStringRef
LSCopyDefaultHandlerForURLScheme(CFStringRef inURLScheme);

extern _Nullable CFArrayRef
LSCopyAllHandlersForURLScheme(CFStringRef inURLScheme);

extern OSStatus
LSSetDefaultHandlerForURLScheme(
  CFStringRef inURLScheme,
  CFStringRef inHandlerBundleID);


#pragma pack(pop)


#ifdef __cplusplus
}
#endif

#include <LaunchServices/LSInfoDeprecated.h>

#endif

