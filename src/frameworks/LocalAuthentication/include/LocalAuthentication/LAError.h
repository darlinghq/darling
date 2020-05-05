#import <Foundation/Foundation.h>
#import <LocalAuthentication/LAPublicDefines.h>

typedef NS_ENUM(NSInteger, LAError)
{
    LAErrorAuthenticationFailed = kLAErrorAuthenticationFailed,
    
    LAErrorUserCancel           = kLAErrorUserCancel,
    
    LAErrorUserFallback         = kLAErrorUserFallback,
    
    LAErrorSystemCancel         = kLAErrorSystemCancel,
    
    LAErrorPasscodeNotSet       = kLAErrorPasscodeNotSet,

    LAErrorTouchIDNotAvailable  = kLAErrorTouchIDNotAvailable,
    
    LAErrorTouchIDNotEnrolled = kLAErrorTouchIDNotEnrolled,

    LAErrorTouchIDLockout   NS_ENUM_AVAILABLE(10_11, 9_0) __WATCHOS_AVAILABLE(3.0) __TVOS_AVAILABLE(10.0) = kLAErrorTouchIDLockout,

    LAErrorAppCancel        NS_ENUM_AVAILABLE(10_11, 9_0) = kLAErrorAppCancel,

    LAErrorInvalidContext   NS_ENUM_AVAILABLE(10_11, 9_0) = kLAErrorInvalidContext
} NS_ENUM_AVAILABLE(10_10, 8_0) __WATCHOS_AVAILABLE(3.0) __TVOS_AVAILABLE(10.0);

extern NSString *const __nonnull LAErrorDomain
NS_AVAILABLE(10_10, 8_3) __WATCHOS_AVAILABLE(3.0) __TVOS_AVAILABLE(10.0);
