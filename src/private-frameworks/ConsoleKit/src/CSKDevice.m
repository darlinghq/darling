#import <AppKit/AppKit.h>

@interface CSKDevice : NSObject
@property (readonly, copy) NSString *name;
@property (readonly, strong) NSImage *icon;
+ (instancetype)hostDevice;
- (BOOL)isEqualToDevice:(CSKDevice *)device;
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

// The host is the only device and a single instance.
- (BOOL)isEqualToDevice:(CSKDevice *)device
{
	return device == self;
}

@end

// Darling has no attached iOS or watchOS devices, so the host is the only device.
@interface CSKDeviceManager : NSObject
@property (weak) id delegate;
@property (readonly, copy) NSArray<CSKDevice *> *allDevices;
@end

@implementation CSKDeviceManager

- (NSArray<CSKDevice *> *)allDevices
{
	return @[[CSKDevice hostDevice]];
}

@end
