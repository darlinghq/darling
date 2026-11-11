#import <AppKit/AppKit.h>

// Which message attributes a stream table shows, in order. Identifiers are CSKMessage key paths.
@interface CSKTableColumnLayout : NSObject
@property (readonly, copy) NSArray<NSString *> *columnIdentifiers;
- (NSString *)titleForColumnIdentifier:(NSString *)identifier;
@end
