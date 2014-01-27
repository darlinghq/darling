#import <Foundation/NSBundle.h>

@class NSString, NSDictionary;

@interface NSBundle(NSNibLoading)

- (BOOL)loadNibNamed:(NSString *)nibName owner:(id)owner topLevelObjects:(NSArray **)topLevelObjects;


@end

@interface NSObject (NSNibAwaking)
- (void)awakeFromNib;

@end

@interface NSBundle (NSNibLoadingDeprecated)
+ (BOOL)loadNibFile:(NSString *)fileName externalNameTable:(NSDictionary *)context withZone:(NSZone *)zone;
+ (BOOL)loadNibNamed:(NSString *)nibName owner:(id)owner;
- (BOOL)loadNibFile:(NSString *)fileName externalNameTable:(NSDictionary *)context withZone:(NSZone *)zone;
@end
