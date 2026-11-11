#import <ConsoleKit/CSKTableColumnLayout.h>

@implementation CSKTableColumnLayout

- (NSArray<NSString *> *)columnIdentifiers
{
	return @[@"date", @"sender", @"processID", @"subsystem", @"category", @"composedMessage"];
}

- (NSString *)titleForColumnIdentifier:(NSString *)identifier
{
	static NSDictionary<NSString *, NSString *> *titles;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		titles = @{
			@"date": @"Time",
			@"sender": @"Process",
			@"processID": @"PID",
			@"subsystem": @"Subsystem",
			@"category": @"Category",
			@"composedMessage": @"Message",
		};
	});
	return titles[identifier];
}

@end

@interface CSKASLTableColumnLayout : CSKTableColumnLayout
@end

@implementation CSKASLTableColumnLayout
@end

@interface CSKArchiveMessagesTableColumnLayout : CSKTableColumnLayout
@end

@implementation CSKArchiveMessagesTableColumnLayout
@end

@interface CSKArchiveActivitiesTableColumnLayout : CSKTableColumnLayout
@end

@implementation CSKArchiveActivitiesTableColumnLayout
@end
