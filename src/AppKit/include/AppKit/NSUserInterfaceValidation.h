#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>
@protocol NSValidatedUserInterfaceItem
- (SEL)action;
- (NSInteger)tag;
@end


@protocol NSUserInterfaceValidations
- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem;
@end
