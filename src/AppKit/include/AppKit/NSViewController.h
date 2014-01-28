#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSResponder.h>


@class NSArray, NSBundle, NSPointerArray, NSView;


@interface NSViewController : NSResponder <NSCoding>
{
    @private
    NSString *_nibName;
    NSBundle *_nibBundle;
    id _representedObject;
    NSString *_title;
    IBOutlet NSView *view;
    NSArray *_topLevelObjects;
    NSPointerArray *_editors;
    id _autounbinder;
    NSString *_designNibBundleIdentifier;
    id _reserved[2];
}



- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil;



- (void)setRepresentedObject:(id)representedObject;
- (id)representedObject;



- (void)setTitle:(NSString *)title;
- (NSString *)title;



- (NSView *)view;



- (void)loadView;



- (NSString *)nibName;
- (NSBundle *)nibBundle;



- (void)setView:(NSView *)view;



- (void)commitEditingWithDelegate:(id)delegate didCommitSelector:(SEL)didCommitSelector contextInfo:(void *)contextInfo;
- (BOOL)commitEditing;
- (void)discardEditing;

@end
