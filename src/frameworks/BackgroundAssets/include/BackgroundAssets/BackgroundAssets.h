#ifndef _BACKGROUNDASSETS_H_
#define _BACKGROUNDASSETS_H_

#import <Foundation/Foundation.h>

FOUNDATION_EXPORT const NSInteger BADownloaderPriorityMin;
FOUNDATION_EXPORT const NSInteger BADownloaderPriorityDefault;
FOUNDATION_EXPORT const NSInteger BADownloaderPriorityMax;

@interface BADownloadManager : NSObject
+ (instancetype)sharedManager;
@end

@interface BAURLDownload : NSObject
@end

@interface BAAssetPackManager : NSObject
+ (instancetype)sharedManager;
@end

#endif
