#import <AppKit/AppKit.h>

// Classes Console links against or instantiates from its nibs whose own methods it has not been
// seen calling under Darling yet.

@interface CSKEntity : NSObject
@end
@implementation CSKEntity
@end

@interface CSKFilter : NSObject
@end
@implementation CSKFilter
@end

@interface CSKLoadRange : NSObject
@end
@implementation CSKLoadRange
@end

@interface CSKDeviceManager : NSObject
@end
@implementation CSKDeviceManager
@end

@interface CSKDirectoryObserver : NSObject
@end
@implementation CSKDirectoryObserver
@end

@interface CSKFileTailObserver : NSObject
@end
@implementation CSKFileTailObserver
@end

@interface CSKFileNode : NSObject
@end
@implementation CSKFileNode
@end

@interface CSKMenuItemRepresentation : NSObject
@end
@implementation CSKMenuItemRepresentation
@end

@interface CSKStreamArchiveStatisticsEntry : NSObject
@end
@implementation CSKStreamArchiveStatisticsEntry
@end

@interface CSKStreamDeviceSource : NSObject
@end
@implementation CSKStreamDeviceSource
@end

@interface CSKStreamSQLSource : NSObject
@end
@implementation CSKStreamSQLSource
@end

@interface CSKLoadRangeViewController : NSViewController
@end
@implementation CSKLoadRangeViewController
@end

@interface CSKStreamViewController : NSViewController
@end
@implementation CSKStreamViewController
@end

@interface CSKTableColumnLayout : NSObject
@end
@implementation CSKTableColumnLayout
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

@interface CSKTokenFieldController : NSObject
@end
@implementation CSKTokenFieldController
@end

@interface CSKTokenField : NSTokenField
@end
@implementation CSKTokenField
@end

@interface CSKTokenFieldCell : NSTokenFieldCell
@end
@implementation CSKTokenFieldCell
@end
