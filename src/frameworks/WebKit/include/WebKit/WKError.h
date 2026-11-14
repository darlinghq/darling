/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern NSString * const WKErrorDomain;

typedef NS_ENUM(NSInteger, WKErrorCode) {
    WKErrorUnknown = 1,
    WKErrorWebContentProcessTerminated,
    WKErrorWebViewInvalidated,
    WKErrorJavaScriptExceptionOccurred,
    WKErrorJavaScriptResultTypeIsUnsupported,
    WKErrorContentRuleListStoreCompileFailed,
    WKErrorContentRuleListStoreLookUpFailed,
    WKErrorContentRuleListStoreRemoveFailed,
    WKErrorContentRuleListStoreVersionMismatch,
    WKErrorAttributedStringContentFailedToLoad,
    WKErrorAttributedStringContentLoadTimedOut,
    WKErrorJavaScriptInvalidFrameTarget,
    WKErrorNavigationAppBoundDomain,
    WKErrorJavaScriptAppBoundDomain,
    WKErrorDuplicateCredential,
    WKErrorMalformedCredential,
    WKErrorCredentialNotFound,
};

NS_ASSUME_NONNULL_END
