#import <CryptoTokenKit/TKTokenWatcher.h>
#import <Foundation/Foundation.h>

// There is no token daemon and no smartcard stack under Darling, so the
// watcher never learns of a token: tokenIDs stays empty and no handler fires.
@implementation TKTokenWatcher

- (instancetype)init
{
	if ((self = [super init]) != nil) {
		_tokenIDs = [[NSMutableArray alloc] init];
		_removalHandlers = [[NSMutableDictionary alloc] init];
	}
	return self;
}

- (instancetype)initWithInsertionHandler:(void (^)(NSString *tokenID))insertionHandler
{
	if ((self = [self init]) != nil) {
		[self setInsertionHandler: insertionHandler];
	}
	return self;
}

- (void)dealloc
{
	[_insertionHandler release];
	[_removalHandlers release];
	[_tokenIDs release];
	[super dealloc];
}

- (NSArray *)tokenIDs
{
	return [[_tokenIDs copy] autorelease];
}

- (void)setInsertionHandler:(void (^)(NSString *tokenID))insertionHandler
{
	id handler = [insertionHandler copy];
	[_insertionHandler release];
	_insertionHandler = handler;
}

- (void)addRemovalHandler:(void (^)(NSString *tokenID))removalHandler forTokenID:(NSString *)tokenID
{
	if (removalHandler && tokenID)
		[_removalHandlers setObject: [[removalHandler copy] autorelease] forKey: tokenID];
}

@end
