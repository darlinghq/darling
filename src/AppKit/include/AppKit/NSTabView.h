#import <AppKit/NSView.h>
#import <AppKit/NSCell.h>

@class NSArray, NSFont, NSTabViewItem;
@protocol NSTabViewDelegate;

#define NSAppKitVersionNumberWithDirectionalTabs 631.0

enum {
    NSTopTabsBezelBorder = 0,
    NSLeftTabsBezelBorder = 1,
    NSBottomTabsBezelBorder = 2,
    NSRightTabsBezelBorder = 3,
    NSNoTabsBezelBorder = 4,
    NSNoTabsLineBorder = 5,
    NSNoTabsNoBorder = 6
};
typedef NSUInteger NSTabViewType;

@interface NSTabView : NSView
{
    @private



    id _tabViewItems;
    NSTabViewItem *_selectedTabViewItem;
    NSFont *_font;
    NSTabViewType _tabViewType;
    BOOL _allowTruncatedLabels;
    id _delegate;



    BOOL _tabViewUnusedBOOL1;

    BOOL _drawsBackground;
    NSTabViewItem *_pressedTabViewItem;
    NSInteger _endTabWidth;
    NSInteger _maxOverlap;
    CGFloat _tabHeight;
    NSTabViewItem *_tabViewItemWithKeyView;
    NSView *_originalNextKeyView;
    struct __NSTabViewDelegateRespondTo {
        unsigned int shouldSelectTabViewItem:1;
        unsigned int willSelectTabViewItem:1;
        unsigned int didSelectTabViewItem:1;
        unsigned int didChangeNumberOfTabViewItems:1;
        unsigned int reserved:28;
    } _delegateRespondTo;

    struct __NSTabViewFlags {
        unsigned int needsLayout:1;
        unsigned int controlTint:3;
        unsigned int controlSize:2;
        unsigned int wiringNibConnections:1;
        unsigned int wiringInteriorLastKeyView:1;
        unsigned int originalNextKeyViewChanged:1;
 unsigned int liveResizeSkippedResetToolTips:1;
        unsigned int subviewsAddedForTabs:1;
        unsigned int reserved:21;
    } _flags;
    NSTabViewItem *_focusedTabViewItem;
    void *_tabViewUnused2;
}



- (void)selectTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)selectTabViewItemAtIndex:(NSInteger)index;
- (void)selectTabViewItemWithIdentifier:(id)identifier;
- (void)takeSelectedTabViewItemFromSender:(id)sender;



- (void)selectFirstTabViewItem:(id)sender;
- (void)selectLastTabViewItem:(id)sender;
- (void)selectNextTabViewItem:(id)sender;
- (void)selectPreviousTabViewItem:(id)sender;



- (NSTabViewItem *)selectedTabViewItem;
- (NSFont *)font;
- (NSTabViewType)tabViewType;
- (NSArray *)tabViewItems;
- (BOOL)allowsTruncatedLabels;
- (NSSize)minimumSize;
- (BOOL)drawsBackground;
- (NSControlTint)controlTint;
- (NSControlSize)controlSize;



- (void)setFont:(NSFont *)font;
- (void)setTabViewType:(NSTabViewType)tabViewType;
- (void)setAllowsTruncatedLabels:(BOOL)allowTruncatedLabels;
- (void)setDrawsBackground:(BOOL)flag;
- (void)setControlTint:(NSControlTint)controlTint;
- (void)setControlSize:(NSControlSize)controlSize;



- (void)addTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)insertTabViewItem:(NSTabViewItem *)tabViewItem atIndex:(NSInteger)index;
- (void)removeTabViewItem:(NSTabViewItem *)tabViewItem;



- (void)setDelegate:(id <NSTabViewDelegate>)anObject;
- (id <NSTabViewDelegate>)delegate;



- (NSTabViewItem *)tabViewItemAtPoint:(NSPoint)point;



- (NSRect)contentRect;



- (NSInteger)numberOfTabViewItems;
- (NSInteger)indexOfTabViewItem:(NSTabViewItem *)tabViewItem;
- (NSTabViewItem *)tabViewItemAtIndex:(NSInteger)index;
- (NSInteger)indexOfTabViewItemWithIdentifier:(id)identifier;

@end





@protocol NSTabViewDelegate <NSObject>
@optional
- (BOOL)tabView:(NSTabView *)tabView shouldSelectTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)tabView:(NSTabView *)tabView willSelectTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem;
- (void)tabViewDidChangeNumberOfTabViewItems:(NSTabView *)tabView;
@end
