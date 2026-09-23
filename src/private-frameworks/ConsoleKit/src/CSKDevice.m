#import <AppKit/AppKit.h>

@interface CSKDevice : NSObject
@property (readonly, copy) NSString *name;
@property (readonly, strong) NSImage *icon;
+ (instancetype)hostDevice;
@end

@implementation CSKDevice

+ (instancetype)hostDevice
{
	static CSKDevice *host;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		host = [[CSKDevice alloc] init];
	});
	return host;
}

- (NSString *)name
{
	return [NSProcessInfo processInfo].hostName;
}

- (NSImage *)icon
{
	return [NSImage imageNamed:NSImageNameComputer];
}

@end
