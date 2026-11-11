#import <Foundation/Foundation.h>

// Console keeps the type as its own number (7 for message-type filters) and reads nothing back yet.
@interface CSKFilter : NSObject
@property (readonly) NSInteger type;
@property (readonly, copy) id value;
- (instancetype)initWithType:(NSInteger)type value:(id)value;
@end

@implementation CSKFilter

- (instancetype)initWithType:(NSInteger)type value:(id)value
{
	self = [super init];
	if (self) {
		_type = type;
		_value = [value copy];
	}
	return self;
}

@end
