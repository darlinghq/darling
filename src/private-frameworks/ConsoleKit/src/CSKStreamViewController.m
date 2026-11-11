#import <ConsoleKit/CSKStreamSource.h>
#import <ConsoleKit/CSKTableColumnLayout.h>

@interface CSKStreamViewController : NSViewController <NSTableViewDataSource, NSTableViewDelegate>
@property (readonly, strong) id<CSKStreamSource> streamSource;
@property (readonly) NSUInteger capacity;
@property (copy) NSString *messagesColumnLayoutAutosaveName;
@property (weak) id delegate;
@property BOOL wantsNowMode;
@property BOOL showsActivities;
@property BOOL showsDetailsPane;
@property (readonly, copy) NSArray *currentFilters;
@property (readonly, copy) NSArray<CSKMessage *> *messages;
@property (readonly, copy) NSError *loadError;
- (instancetype)initWithStreamSource:(id<CSKStreamSource>)source capacity:(NSUInteger)capacity;
- (void)updateMessagesColumnLayoutWithoutInvalidateAndSave:(CSKTableColumnLayout *)layout;
- (void)reload;
@end

@implementation CSKStreamViewController {
	CSKTableColumnLayout *_layout;
	NSTableView *_tableView;
	NSDateFormatter *_dateFormatter;
}

- (instancetype)initWithStreamSource:(id<CSKStreamSource>)source capacity:(NSUInteger)capacity
{
	self = [super initWithNibName:nil bundle:nil];
	if (self) {
		_streamSource = source;
		_capacity = capacity;
		_currentFilters = @[];
		_messages = @[];
		_layout = [CSKTableColumnLayout new];
		_dateFormatter = [NSDateFormatter new];
		_dateFormatter.dateFormat = @"yyyy-MM-dd HH:mm:ss.SSSSSS";
	}
	return self;
}

- (void)loadView
{
	NSScrollView *scrollView = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 800, 400)];
	scrollView.hasVerticalScroller = YES;
	scrollView.hasHorizontalScroller = YES;
	scrollView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
	_tableView = [[NSTableView alloc] initWithFrame:scrollView.bounds];
	_tableView.dataSource = self;
	_tableView.delegate = self;
	_tableView.usesAlternatingRowBackgroundColors = YES;
	scrollView.documentView = _tableView;
	self.view = scrollView;
	[self rebuildColumns];
}

- (void)rebuildColumns
{
	if (_tableView == nil)
		return;
	for (NSTableColumn *column in [_tableView.tableColumns copy])
		[_tableView removeTableColumn:column];
	for (NSString *identifier in _layout.columnIdentifiers) {
		NSTableColumn *column = [[NSTableColumn alloc] initWithIdentifier:identifier];
		[column.headerCell setStringValue:[_layout titleForColumnIdentifier:identifier]];
		column.width = [identifier isEqualToString:@"composedMessage"] ? 600 : 140;
		[_tableView addTableColumn:column];
	}
	[_tableView reloadData];
}

- (void)updateMessagesColumnLayoutWithoutInvalidateAndSave:(CSKTableColumnLayout *)layout
{
	_layout = layout;
	[self rebuildColumns];
}

- (void)reload
{
	NSError *error = nil;
	NSArray<CSKMessage *> *messages = [_streamSource loadMessagesWithLimit:_capacity error:&error];
	_loadError = [error copy];
	_messages = messages ? [messages copy] : @[];
	[_tableView reloadData];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
	return _messages.count;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)column row:(NSInteger)row
{
	id value = [_messages[row] valueForKey:column.identifier];
	if ([value isKindOfClass:[NSDate class]])
		return [_dateFormatter stringFromDate:value];
	return value;
}

@end
