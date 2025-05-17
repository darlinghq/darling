#ifndef _CARBON_IB_CARBON_RUNTIME_H_
#define _CARBON_IB_CARBON_RUNTIME_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CarbonCore/MacErrors.h>
#include <CoreFoundation/CFBase.h>
#include <HIToolbox/Menus.h>
#include "../../src/MacWindows.h" // TODO: put WindowRef where it actually goes

#ifdef __cplusplus
extern "C" {
#endif

enum {
	kIBCarbonRuntimeCantFindNibFile = -10960,
	kIBCarbonRuntimeObjectNotOfRequestedType = -10961,
	kIBCarbonRuntimeCantFindObject = -10962
};

typedef struct OpaqueIBNibRef * IBNibRef;

OSStatus CreateMenuBarFromNib(IBNibRef inNibRef, CFStringRef inName, Handle *outMenuBar);
OSStatus CreateMenuFromNib(IBNibRef inNibRef, CFStringRef inName, MenuRef *outMenuRef);
OSStatus CreateNibReference(CFStringRef inNibName, IBNibRef *outNibRef);
OSStatus CreateNibReferenceWithCFBundle(CFBundleRef inBundle, CFStringRef inNibName, IBNibRef *outNibRef);
OSStatus CreateWindowFromNib(IBNibRef inNibRef, CFStringRef inName, WindowRef *outWindow);

void DisposeNibReference(IBNibRef inNibRef);

OSStatus SetMenuBarFromNib(IBNibRef inNibRef, CFStringRef inName);

#ifdef __cplusplus
}
#endif

#endif // _CARBON_IB_CARBON_RUNTIME_H_
