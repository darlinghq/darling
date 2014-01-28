#import <Foundation/NSObject.h>


#import <AppKit/NSActionCell.h>

@class NSString, NSImage, NSMutableArray, NSArray, NSAnimation, NSPathComponentCell, NSURL, NSPopUpButtonCell, NSNotification, NSOpenPanel;
@protocol NSPathCellDelegate, NSOpenSavePanelDelegate;
enum {


    NSPathStyleStandard,



    NSPathStylePopUp = 2,
};

typedef NSInteger NSPathStyle;


@interface NSPathCell : NSActionCell <NSOpenSavePanelDelegate> {
@private
    NSColor *_backgroundColor;
    NSMutableArray *_borderColors;
    NSMutableArray *_cells;
    NSPathComponentCell *_clickedCell;
    NSPathComponentCell *_hoveredCell;
    NSPopUpButtonCell *_popUpButtonCell;
    NSRect _currentRect;
    NSAnimation *_animation;
    NSArray *_allowedTypes;
    SEL _doubleAction;
    id _delegate;
    struct {
        unsigned int cbs:4;
        unsigned int reserved:28;
    } _piFlags;
    NSPathStyle _pathStyle;
    id _aux;
}



- (NSPathStyle)pathStyle;
- (void)setPathStyle:(NSPathStyle)style;



- (NSURL *)URL;
- (void)setURL:(NSURL *)url;



- (void)setObjectValue:(id <NSCopying>)obj;



- (NSArray *)allowedTypes;
- (void)setAllowedTypes:(NSArray *)allowedTypes;



- (id <NSPathCellDelegate>)delegate;
- (void)setDelegate:(id <NSPathCellDelegate>)value;



+ (Class)pathComponentCellClass;



- (NSArray *)pathComponentCells;



- (void)setPathComponentCells:(NSArray *)cells;



- (NSRect)rectOfPathComponentCell:(NSPathComponentCell *)cell withFrame:(NSRect)frame inView:(NSView *)view;



- (NSPathComponentCell *)pathComponentCellAtPoint:(NSPoint)point withFrame:(NSRect)frame inView:(NSView *)view;



- (NSPathComponentCell *)clickedPathComponentCell;



- (void)mouseEntered:(NSEvent *)event withFrame:(NSRect)frame inView:(NSView *)view;
- (void)mouseExited:(NSEvent *)event withFrame:(NSRect)frame inView:(NSView *)view;



- (SEL)doubleAction;
- (void)setDoubleAction:(SEL)action;



- (void)setBackgroundColor:(NSColor *)color;
- (NSColor *)backgroundColor;



- (void)setPlaceholderString:(NSString *)string;
- (NSString *)placeholderString;

- (void)setPlaceholderAttributedString:(NSAttributedString *)string;
- (NSAttributedString *)placeholderAttributedString;


- (void)setControlSize:(NSControlSize)size;

@end

@protocol NSPathCellDelegate <NSObject>

@optional



- (void)pathCell:(NSPathCell *)pathCell willDisplayOpenPanel:(NSOpenPanel *)openPanel;



- (void)pathCell:(NSPathCell *)pathCell willPopUpMenu:(NSMenu *)menu;

@end


enum {


    NSPathStyleNavigationBar = 1
};
