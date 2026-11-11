#ifndef _FINDERSYNC_H_
#define _FINDERSYNC_H_

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

typedef NSString *FIBadgeIdentifier;

@protocol FIFinderSyncProtocol
@optional
- (void)beginObservingDirectoryAtURL:(NSURL *)url;
- (void)endObservingDirectoryAtURL:(NSURL *)url;
- (void)requestBadgeIdentifierForURL:(NSURL *)url;
@end

@interface FIFinderSyncController : NSObject {
@public
    NSSet<NSURL *> *_directoryURLs;
    NSMutableDictionary<NSURL *, FIBadgeIdentifier> *_badges;
}

+ (instancetype)defaultController;
+ (BOOL)isExtensionEnabled;
+ (void)showExtensionManagementInterface;

@property (copy) NSSet<NSURL *> *directoryURLs;

- (void)setBadgeImage:(NSImage *)image label:(NSString *)label forBadgeIdentifier:(FIBadgeIdentifier)badgeID;
- (void)setBadgeIdentifier:(FIBadgeIdentifier)badgeID forURL:(NSURL *)url;
- (FIBadgeIdentifier)badgeIdentifierForURL:(NSURL *)url;

@end

@interface FIFinderSync : NSObject <FIFinderSyncProtocol>
@end

#endif
