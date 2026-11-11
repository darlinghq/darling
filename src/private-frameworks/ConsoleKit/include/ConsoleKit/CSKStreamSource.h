#import <ConsoleKit/CSKMessage.h>

// What CSKStreamViewController needs from a message source: every stored message, oldest first,
// at most `limit` of the newest ones. Returns nil and sets `error` when the store can't be read.
@protocol CSKStreamSource <NSObject>
- (NSArray<CSKMessage *> *)loadMessagesWithLimit:(NSUInteger)limit error:(NSError **)error;
@end
