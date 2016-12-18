#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface TKTokenWatcher : NSObject

@property (readonly) NSArray<NSString *> *tokenIDs;

- (instancetype)init;

- (instancetype)initWithInsertionHandler:(void(^)(NSString* tokenID)) insertionHandler;

- (void)addRemovalHandler:(void(^)(NSString* tokenID)) removalHandler forTokenID:(NSString*) tokenID;

@end

NS_ASSUME_NONNULL_END
