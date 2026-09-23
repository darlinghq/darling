#import <Foundation/Foundation.h>

@interface CSKHelpers : NSObject
+ (NSString *)formattedNumberFromUnsignedInteger:(NSUInteger)number;
@end

@implementation CSKHelpers

+ (NSString *)formattedNumberFromUnsignedInteger:(NSUInteger)number
{
	return [NSNumberFormatter localizedStringFromNumber:@(number) numberStyle:NSNumberFormatterDecimalStyle];
}

@end
