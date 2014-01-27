#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSPanel.h>

@class NSBox, NSMutableArray, NSTextField, NSTextView, NSView, NSURL, NSProgressIndicator, NSControl;
@protocol NSOpenSavePanelDelegate;



enum {
    NSFileHandlingPanelCancelButton = NSCancelButton,
    NSFileHandlingPanelOKButton = NSOKButton,
};

typedef struct __SPFlags {
    unsigned int saveMode:1;
    unsigned int isExpanded:1;
    unsigned int allowsOtherFileTypes:1;
    unsigned int canCreateDirectories:1;
    unsigned int canSelectedHiddenExtension:1;
    unsigned int reserved2:1;
    unsigned int delegate_shouldShowFilename:1;
    unsigned int delegate_compareFilename:1;
    unsigned int delegate_shouldEnableURL:1;
    unsigned int delegate_validateURL:1;
    unsigned int delegate_didChangeToDirectoryURL:1;
    unsigned int changingFrameSize:1;
    unsigned int movingAccessoryView:1;
    unsigned int userAccessoryViewFrameChange:1;
    unsigned int canChooseDirectories:1;
    unsigned int canChooseFiles:1;
    unsigned int delegate_selectionDidChange:1;
    unsigned int delegate_didChangeToDirectory:1;
    unsigned int calledWindowOrderedIn:1;
    unsigned int appCentric:1;
    unsigned int disableBottomControls:1;
    unsigned int accessoryViewDisclosed:1;
    unsigned int reserved:10;
} _SPFlags;

@class NSSavePanelAuxiliary;

@interface NSSavePanel : NSPanel {
@protected

    NSView *__navView;
    NSView *_accessoryView;
    NSArray *_allowedFileTypes;
    NSString *_validatedPosixName;
    NSString *_hiddenExtension;

    IBOutlet NSTextField *_messageTextField;
    IBOutlet NSView *_savePane;
    IBOutlet NSBox *_saveNavSeparatorBox;
    IBOutlet NSView *_savePaneTopPartsContainer;
    IBOutlet NSTextField *_nameField;
    IBOutlet NSTextField *_nameFieldLabel;
    IBOutlet NSButton *_expansionButton;
    IBOutlet NSView *_directoryPopUpContainer;
    IBOutlet NSControl *_directoryPopUp;
    IBOutlet NSTextField *_directoryPopUpLabel;
    IBOutlet NSBox *_navViewContainer;
    IBOutlet NSBox *_accessoryViewContainer;
    IBOutlet NSView *_bottomControlsContainer;
    IBOutlet NSButton *_hideExtensionButton;
    IBOutlet NSButton *_newFolderButton;
    IBOutlet NSButton *_cancelButton;
    IBOutlet NSButton *_okButton;

    id _filepathInputController;
    id _reservedID;

    _SPFlags _spFlags;

    NSSavePanelAuxiliary *_spAuxiliaryStorage;

@private
    char _directorySet:1;
    char _reserved[4];
    IBOutlet NSProgressIndicator *_openProgressIndicator;
}



+ (NSSavePanel *)savePanel;







- (NSURL *)URL;






- (NSURL *)directoryURL;
- (void)setDirectoryURL:(NSURL *)url;





- (NSArray *)allowedFileTypes;
- (void)setAllowedFileTypes:(NSArray *)types;




- (BOOL)allowsOtherFileTypes;
- (void)setAllowsOtherFileTypes:(BOOL)flag;



- (NSView *)accessoryView;
- (void)setAccessoryView:(NSView *)view;



- (id <NSOpenSavePanelDelegate>)delegate;
- (void)setDelegate:(id<NSOpenSavePanelDelegate>)delegate;




- (BOOL)isExpanded;



- (BOOL)canCreateDirectories;
- (void)setCanCreateDirectories:(BOOL)flag;




- (BOOL)canSelectHiddenExtension;
- (void)setCanSelectHiddenExtension:(BOOL)flag;




- (BOOL)isExtensionHidden;
- (void)setExtensionHidden:(BOOL)flag;



- (BOOL)treatsFilePackagesAsDirectories;
- (void)setTreatsFilePackagesAsDirectories:(BOOL)flag;



- (NSString *)prompt;
- (void)setPrompt:(NSString *)prompt;



- (NSString *)title;
- (void)setTitle:(NSString *)title;




- (NSString *)nameFieldLabel;
- (void)setNameFieldLabel:(NSString *)label;




- (NSString *)nameFieldStringValue;
- (void)setNameFieldStringValue:(NSString *)value;



- (NSString *)message;
- (void)setMessage:(NSString *)message;

- (void)validateVisibleColumns;



- (BOOL)showsHiddenFiles;
- (void)setShowsHiddenFiles:(BOOL)flag;




- (IBAction)ok:(id)sender;
- (IBAction)cancel:(id)sender;




#if NS_BLOCKS_AVAILABLE



- (void)beginSheetModalForWindow:(NSWindow *)window completionHandler:(void (^)(NSInteger result))handler;



- (void)beginWithCompletionHandler:(void (^)(NSInteger result))handler;

#endif



- (NSInteger)runModal;

@end


@protocol NSOpenSavePanelDelegate <NSObject>
@optional





- (BOOL)panel:(id)sender shouldEnableURL:(NSURL *)url;





- (BOOL)panel:(id)sender validateURL:(NSURL *)url error:(NSError **)outError;



- (void)panel:(id)sender didChangeToDirectoryURL:(NSURL *)url;



- (NSString *)panel:(id)sender userEnteredFilename:(NSString *)filename confirmed:(BOOL)okFlag;



- (void)panel:(id)sender willExpand:(BOOL)expanding;



- (void)panelSelectionDidChange:(id)sender;

@end

@interface NSObject(NSSavePanelDelegateDeprecated)



- (BOOL)panel:(id)sender isValidFilename:(NSString *)filename;



- (void)panel:(id)sender directoryDidChange:(NSString *)path;



 - (NSComparisonResult)panel:(id)sender compareFilename:(NSString *)name1 with:(NSString *)name2 caseSensitive:(BOOL)caseSensitive;



- (BOOL)panel:(id)sender shouldShowFilename:(NSString *)filename;

@end

@interface NSSavePanel(NSDeprecated)



- (NSString *)filename;



- (NSString *)directory;
- (void)setDirectory:(NSString *)path;



- (NSString *)requiredFileType;
- (void)setRequiredFileType:(NSString *)type;



- (void)beginSheetForDirectory:(NSString *)path file:(NSString *)name modalForWindow:(NSWindow *)docWindow modalDelegate:(id)delegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;



- (NSInteger)runModalForDirectory:(NSString *)path file:(NSString *)name;



- (IBAction)selectText:(id)sender;

@end
