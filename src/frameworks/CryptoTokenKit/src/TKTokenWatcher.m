#import <CryptoTokenKit/TKTokenWatcher.h>
#import <Foundation/Foundation.h>

@protocol TKProtocolTokenWatcher
@end

// There is no token daemon and no smartcard stack under Darling, so the
// watcher never learns of a token: tokenIDs stays empty and no handler fires.
@implementation TKTokenWatcher

- (id)init
{
	if ((self = [super init]) != nil) {
		_tokenIDs = [[NSMutableArray alloc] init];
		_removalHandlers = [[NSMutableDictionary alloc] init];
	}
	return self;
}

- (id)initWithInsertionHandler:(id)arg1
{
	if ((self = [self init]) != nil) {
		_insertionHandler = [arg1 copy];
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

- (void)addRemovalHandler:(id)arg1 forTokenID:(id)arg2
{
	if (arg1 && arg2)
		[_removalHandlers setObject: [[arg1 copy] autorelease] forKey: arg2];
}

- (void)insertedToken:(id)arg1 reply:(id)arg2
{
}

- (void)removedToken:(id)arg1 reply:(id)arg2
{
}

- (void)setup
{
}

@end
