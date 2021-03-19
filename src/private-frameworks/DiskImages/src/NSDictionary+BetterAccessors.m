#import <DiskImages/NSDictionary+BetterAccessors.h>
#import <Foundation/NSString.h>

// why is this in the DiskImages framework, of all places?
// and why does Xcode depend on it?!

@implementation NSDictionary (BetterAccessors)

- (id)stringForKey: (id)key
{
	id object = [self objectForKey: key];
	if ([object isKindOfClass: [NSString class]]) {
		return object;
	}
	return nil;
}

@end
