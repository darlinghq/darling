#import <Foundation/NSObject.h>


#import <AppKit/NSControl.h>
#import <AppKit/NSPathCell.h>
#import <AppKit/NSDragging.h>

@class NSMutableArray, NSPathComponentCell, NSOpenPanel;
@protocol NSPathControlDelegate;

@interface NSPathControl : NSControl {
@private
    NSDragOperation _draggingSourceOperationMaskForLocal;
    NSDragOperation _draggingSourceOperationMaskForNonLocal;
    NSInteger _reserved;
    id _delegate;
    id _aux;
}




- (NSURL *)URL;
- (void)setURL:(NSURL *)url;

- (SEL)doubleAction;
- (void)setDoubleAction:(SEL)action;

- (NSPathStyle)pathStyle;
- (void)setPathStyle:(NSPathStyle)style;

- (NSPathComponentCell *)clickedPathComponentCell;

- (NSArray *)pathComponentCells;
- (void)setPathComponentCells:(NSArray *)cells;



- (void)setBackgroundColor:(NSColor *)color;
- (NSColor *)backgroundColor;



- (id <NSPathControlDelegate>)delegate;
- (void)setDelegate:(id <NSPathControlDelegate>)delegate;




- (void)setDraggingSourceOperationMask:(NSDragOperation)mask forLocal:(BOOL)isLocal;



- (void)setMenu:(NSMenu *)menu;
- (NSMenu *)menu;

@end


@protocol NSPathControlDelegate <NSObject>

@optional



- (BOOL)pathControl:(NSPathControl *)pathControl shouldDragPathComponentCell:(NSPathComponentCell *)pathComponentCell withPasteboard:(NSPasteboard *)pasteboard;





- (NSDragOperation)pathControl:(NSPathControl *)pathControl validateDrop:(id <NSDraggingInfo>)info;






- (BOOL)pathControl:(NSPathControl *)pathControl acceptDrop:(id <NSDraggingInfo>)info;






- (void)pathControl:(NSPathControl *)pathControl willDisplayOpenPanel:(NSOpenPanel *)openPanel;



- (void)pathControl:(NSPathControl *)pathControl willPopUpMenu:(NSMenu *)menu;

@end
