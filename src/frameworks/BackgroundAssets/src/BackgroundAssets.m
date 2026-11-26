#import <BackgroundAssets/BackgroundAssets.h>

const NSInteger BADownloaderPriorityMin = -1000;
const NSInteger BADownloaderPriorityDefault = 0;
const NSInteger BADownloaderPriorityMax = 1000;

@implementation BADownloadManager

+ (instancetype)sharedManager {
    static BADownloadManager *mgr = nil;
    if (!mgr) {
        mgr = [[BADownloadManager alloc] init];
    }
    return mgr;
}

- (id)delegate { return nil; }
- (void)setDelegate:(id)d { (void)d; }

- (void)fetchCurrentDownloadsWithCompletionHandler:(void (^)(NSArray *, NSError *))handler {
    if (handler) handler(nil, nil);
}

- (void)performWithExclusiveControl:(void (^)(BOOL, NSError *))handler {
    if (handler) handler(NO, nil);
}

- (BOOL)scheduleDownload:(id)d error:(NSError **)err {
    (void)d;
    if (err) *err = nil;
    return NO;
}

- (BOOL)startForegroundDownload:(id)d error:(NSError **)err {
    (void)d;
    if (err) *err = nil;
    return NO;
}

- (BOOL)cancelDownload:(id)d error:(NSError **)err {
    (void)d;
    if (err) *err = nil;
    return NO;
}

@end

@implementation BAURLDownload

- (instancetype)initWithIdentifier:(NSString *)identifier
                           request:(NSURLRequest *)request
        applicationGroupIdentifier:(NSString *)groupIdentifier {
    (void)identifier; (void)request; (void)groupIdentifier;
    return [super init];
}

- (instancetype)initWithIdentifier:(NSString *)identifier
                           request:(NSURLRequest *)request
                          fileSize:(NSUInteger)fileSize
        applicationGroupIdentifier:(NSString *)groupIdentifier {
    (void)identifier; (void)request; (void)fileSize; (void)groupIdentifier;
    return [super init];
}

@end

@implementation BAAssetPackManager

+ (instancetype)sharedManager {
    return nil;
}

@end
