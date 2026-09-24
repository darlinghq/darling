#import <ConsoleKit/CSKMessage.h>

@implementation CSKMessage

+ (NSString *)localizedMessageTypeNameForType:(CSKMessageType)type
{
	switch (type) {
	case CSKMessageTypeError:
		return @"Error";
	case CSKMessageTypeFault:
		return @"Fault";
	}
	[NSException raise:NSInvalidArgumentException format:@"Unknown ConsoleKit message type %ld", (long)type];
	return nil;
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
