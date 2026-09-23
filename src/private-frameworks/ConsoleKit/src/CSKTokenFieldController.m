#import <AppKit/AppKit.h>

@interface CSKTokenField : NSTokenField
@end

@implementation CSKTokenField
@end

@interface CSKTokenFieldCell : NSTokenFieldCell
@end

@implementation CSKTokenFieldCell
@end

@interface CSKTokenFieldController : NSObject
@property (strong) NSTokenField *tokenField;
@property (weak) id delegate;
@property BOOL isBasicSearchEnabled;
@property (readonly, copy) NSArray *filters;
- (void)updateSearchWithFilters:(NSArray *)filters;
@end

@implementation CSKTokenFieldController

- (void)updateSearchWithFilters:(NSArray *)filters
{
	_filters = [filters copy];
	self.tokenField.objectValue = _filters;
}

@end
