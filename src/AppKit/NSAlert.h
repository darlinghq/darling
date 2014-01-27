#import <Foundation/NSObject.h>
#import <AppKit/NSGraphics.h>
@class NSTextField, NSPanel, NSArray, NSWindow, NSImage, NSButton, NSError;
@protocol NSAlertDelegate;




enum {
    NSWarningAlertStyle = 0,
    NSInformationalAlertStyle = 1,
    NSCriticalAlertStyle = 2
};
typedef NSUInteger NSAlertStyle;

@interface NSAlert : NSObject
{
    @private

    NSTextField *_informationField;
    id _first;
    id _second;
    id _third;
    NSArray *_buttons;
    NSPanel *_panel;
    id _messageField;
    id _imageView;
    NSSize _minButtonSize;
    CGFloat _buttonSpacing;
    CGFloat _buttonPadding;
    CGFloat _messagePadding;
    CGFloat _buttonSpacingMaxX;
    CGFloat _buttonSpacingY;
    id _modalDelegate;
    NSWindow *_docWindow;
    SEL _didEndSelector;
    SEL _didDismissSelector;
    NSImage *_unbadgedImage;
    NSSize _defaultPanelSize;
    id _helpButton;
    id _delegate;
    NSAlertStyle _alertStyle;
    id _helpAnchor;
    BOOL _layoutDone;
    BOOL _showsHelp;
    BOOL _showsSuppressionButton;
    BOOL reserved;
    id _suppressionButton;
    id _accessoryView;
}




+ (NSAlert *)alertWithError:(NSError *)error;




+ (NSAlert *)alertWithMessageText:(NSString *)message defaultButton:(NSString *)defaultButton alternateButton:(NSString *)alternateButton otherButton:(NSString *)otherButton informativeTextWithFormat:(NSString *)format, ...;

- (void)setMessageText:(NSString *)messageText;
- (void)setInformativeText:(NSString *)informativeText;

- (NSString *)messageText;
- (NSString *)informativeText;



- (void)setIcon:(NSImage *)icon;
- (NSImage *)icon;



- (NSButton *)addButtonWithTitle:(NSString *)title;


- (NSArray *)buttons;
enum {
 NSAlertFirstButtonReturn = 1000,
 NSAlertSecondButtonReturn = 1001,
 NSAlertThirdButtonReturn = 1002
};
- (void)setShowsHelp:(BOOL)showsHelp;
- (BOOL)showsHelp;

- (void)setHelpAnchor:(NSString *)anchor;
- (NSString *)helpAnchor;

- (void)setAlertStyle:(NSAlertStyle)style;
- (NSAlertStyle)alertStyle;

- (void)setDelegate:(id <NSAlertDelegate>)delegate;
- (id <NSAlertDelegate>)delegate;



- (void)setShowsSuppressionButton:(BOOL)flag;
- (BOOL)showsSuppressionButton;



- (NSButton *)suppressionButton;




- (void)setAccessoryView:(NSView *)view;
- (NSView *)accessoryView;



- (void)layout;




- (NSInteger)runModal;





- (void)beginSheetModalForWindow:(NSWindow *)window modalDelegate:(id)delegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;



- (id)window;

@end

@protocol NSAlertDelegate <NSObject>
@optional


- (BOOL)alertShowHelp:(NSAlert *)alert;
@end
