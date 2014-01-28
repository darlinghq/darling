#import <AppKit/NSWindow.h>

@class NSCloseCommand;
@class NSScriptCommand;

@interface NSWindow(NSScripting)


- (BOOL)hasCloseBox;
- (BOOL)hasTitleBar;
- (BOOL)isFloatingPanel;
- (BOOL)isMiniaturizable;
- (BOOL)isModalPanel;
- (BOOL)isResizable;
- (BOOL)isZoomable;
- (NSInteger)orderedIndex;


- (void)setIsMiniaturized:(BOOL)flag;
- (void)setIsVisible:(BOOL)flag;
- (void)setIsZoomed:(BOOL)flag;
- (void)setOrderedIndex:(NSInteger)index;


- (id)handleCloseScriptCommand:(NSCloseCommand *)command;
- (id)handlePrintScriptCommand:(NSScriptCommand *)command;
- (id)handleSaveScriptCommand:(NSScriptCommand *)command;

@end
