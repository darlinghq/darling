#ifdef __OBJC__
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern NSString *const TKErrorDomain;

typedef NS_ENUM(NSInteger, TKErrorCode) {
    TKErrorCodeNotImplemented        = -1,
    TKErrorCodeCommunicationError    = -2,
    TKErrorCodeCorruptedData         = -3,
    TKErrorCodeCanceledByUser        = -4,
    TKErrorCodeAuthenticationFailed  = -5,
    TKErrorCodeObjectNotFound        = -6,
    TKErrorCodeTokenNotFound         = -7,
    TKErrorCodeBadParameter          = -8,
    TKErrorCodeAuthenticationNeeded  = -9,

    TKErrorAuthenticationFailed      = TKErrorCodeAuthenticationFailed,
    TKErrorObjectNotFound            = TKErrorCodeObjectNotFound,
    TKErrorTokenNotFound             = TKErrorCodeTokenNotFound,
};

NS_ASSUME_NONNULL_END

#endif

