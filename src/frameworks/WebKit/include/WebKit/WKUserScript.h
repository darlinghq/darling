/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, WKUserScriptInjectionTime) {
    WKUserScriptInjectionTimeAtDocumentStart,
    WKUserScriptInjectionTimeAtDocumentEnd,
};

@interface WKUserScript : NSObject <NSCopying>

@property (nonatomic, readonly, copy) NSString *source;
@property (nonatomic, readonly) WKUserScriptInjectionTime injectionTime;
@property (nonatomic, readonly, getter=isForMainFrameOnly) BOOL forMainFrameOnly;

- (instancetype)initWithSource:(NSString *)source injectionTime:(WKUserScriptInjectionTime)injectionTime forMainFrameOnly:(BOOL)forMainFrameOnly;

@end

NS_ASSUME_NONNULL_END
