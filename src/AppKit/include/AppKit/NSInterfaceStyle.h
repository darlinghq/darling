#import <AppKit/NSResponder.h>
#import <AppKit/AppKitDefines.h>

enum {
    NSNoInterfaceStyle = 0,
    NSNextStepInterfaceStyle = 1,
    NSWindows95InterfaceStyle = 2,
    NSMacintoshInterfaceStyle = 3
};
typedef NSUInteger NSInterfaceStyle;

EXTERN_C NSInterfaceStyle NSInterfaceStyleForKey(NSString *key, NSResponder *responder);


@interface NSResponder (NSInterfaceStyle)
- (NSInterfaceStyle)interfaceStyle;
- (void)setInterfaceStyle:(NSInterfaceStyle)interfaceStyle;

@end



EXTERN_C NSString *NSInterfaceStyleDefault;
