#ifndef _ILIFEMEDIABROWSER_H_
#define _ILIFEMEDIABROWSER_H_

#import <Foundation/Foundation.h>

extern NSString *const ILMediaObjectCommentsKey;

@interface ILMediaManager : NSObject
@end

@interface ILFolderMediaManager : ILMediaManager
@end

@interface ILiPhotoMediaManager : ILMediaManager
@end

@interface ILiTunesMediaManager : ILMediaManager
@end

#endif
