#import <Foundation/NSObject.h>
#include <mach/message.h>

@class NSString, NSURL, NSError;

@interface _LSQueryResult : NSObject <NSCopying, NSSecureCoding>
@end

@interface LSResourceProxy : _LSQueryResult
@property (nonatomic, copy, readonly) NSString *localizedName;
@end

@interface LSBundleProxy : LSResourceProxy {
    NSURL *_bundleURL;
}
+ (instancetype)bundleProxyWithAuditToken:(audit_token_t)auditToken error:(NSError **)outError;
+ (instancetype)bundleProxyForCurrentProcess;
@property (nonatomic, readonly) NSString *bundleIdentifier;
@property (nonatomic, readonly) NSURL *bundleURL;
@end
