#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSUserInterfaceItemIdentification.h>

@class NSTableView;
@class NSCell;
@class NSImage;
@class NSSortDescriptor;

@interface NSTableColumn : NSObject <NSCoding, NSUserInterfaceItemIdentification> {

    id _identifier;
    CGFloat _width;
    CGFloat _minWidth;
    CGFloat _maxWidth;
    NSTableView *_tableView;
    NSCell *_headerCell;
    NSCell *_dataCell;
    struct __colFlags {
        unsigned int oldIsResizable:1;
        unsigned int isEditable:1;
        unsigned int resizedPostingDisableCount:8;
        unsigned int canUseReorderResizeImageCache:1;
        unsigned int userResizingAllowed:1;
        unsigned int autoResizingAllowed:1;
        unsigned int hidden:1;
        unsigned int RESERVED:18;
    } _cFlags;
    id _tcAuxiliaryStorage;
}



- (id)initWithIdentifier:(NSString *)identifier;



- (void)setIdentifier:(NSString *)identifier;
- (NSString *)identifier;

- (void)setTableView:(NSTableView *)tableView;
- (NSTableView *)tableView;



- (void)setWidth:(CGFloat)width;
- (CGFloat)width;



- (void)setMinWidth:(CGFloat)minWidth;
- (CGFloat)minWidth;



- (void)setMaxWidth:(CGFloat)maxWidth;
- (CGFloat)maxWidth;



- (void)setHeaderCell:(NSCell *)cell;
- (id)headerCell;



- (void)setDataCell:(NSCell *)cell;
- (id)dataCell;



- (id)dataCellForRow:(NSInteger)row;



- (void)setEditable:(BOOL)flag;
- (BOOL)isEditable;

- (void)sizeToFit;



- (void)setSortDescriptorPrototype:(NSSortDescriptor *)sortDescriptor;
- (NSSortDescriptor *)sortDescriptorPrototype;

enum {
    NSTableColumnNoResizing = 0,
    NSTableColumnAutoresizingMask = ( 1 << 0 ),
    NSTableColumnUserResizingMask = ( 1 << 1 ),
};



- (void)setResizingMask:(NSUInteger)resizingMask;
- (NSUInteger)resizingMask;



- (void)setHeaderToolTip:(NSString *)string;
- (NSString *)headerToolTip;



- (BOOL)isHidden;
- (void)setHidden:(BOOL)hidden;

@end





@interface NSTableColumn(NSDeprecated)



- (void)setResizable:(BOOL)flag;
- (BOOL)isResizable;

@end
