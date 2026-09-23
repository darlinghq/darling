#import <ConsoleKit/CSKMessage.h>

@implementation CSKMessage

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
