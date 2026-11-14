/*
 Copyright (C) 2026 Lubos Dolezel
 Darling is free software under the GNU GPL version 3 or later.
*/

#import <Foundation/Foundation.h>
#import <WebKit/WKScriptMessageHandler.h>
#import <WebKit/WKUserScript.h>

NS_ASSUME_NONNULL_BEGIN

@interface WKUserContentController : NSObject

@property (nonatomic, readonly, copy) NSArray<WKUserScript *> *userScripts;

- (void)addUserScript:(WKUserScript *)userScript;
- (void)removeAllUserScripts;
- (void)addScriptMessageHandler:(id <WKScriptMessageHandler>)scriptMessageHandler name:(NSString *)name;
- (void)removeScriptMessageHandlerForName:(NSString *)name;
- (void)removeAllScriptMessageHandlers;

@end

NS_ASSUME_NONNULL_END
