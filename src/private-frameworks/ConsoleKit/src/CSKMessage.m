#import <ConsoleKit/CSKMessage.h>

@implementation CSKMessage

// Names as observed from the original framework's class method on macOS 27 (black-box probe,
// vd-upstream/mac-probes/v153-consolekit-messagetype.m): types 2 to 5 are info, debug, error and
// fault, in lowercase; every other number, negative ones included, yields "default" and nothing
// raises.
+ (NSString *)localizedMessageTypeNameForType:(CSKMessageType)type
{
	switch (type) {
	case CSKMessageTypeInfo:
		return @"info";
	case CSKMessageTypeDebug:
		return @"debug";
	case CSKMessageTypeError:
		return @"error";
	case CSKMessageTypeFault:
		return @"fault";
	}
	return @"default";
}

- (instancetype)initWithDate:(NSDate *)date sender:(NSString *)sender processID:(pid_t)pid subsystem:(NSString *)subsystem category:(NSString *)category level:(int)level message:(NSString *)message
{
	self = [super init];
	if (self) {
		_date = [date copy];
		_sender = [sender copy];
		_processID = pid;
		_subsystem = [subsystem copy];
		_category = [category copy];
		_level = level;
		_composedMessage = [message copy];
	}
	return self;
}

@end
