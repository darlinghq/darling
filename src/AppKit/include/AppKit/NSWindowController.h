#import <AppKit/NSResponder.h>
#import <AppKit/NSNibDeclarations.h>

@class NSArray, NSDocument, NSWindow;

@interface NSWindowController : NSResponder <NSCoding> {
    @private
    NSWindow *_window;
    NSString *_windowNibName;
    NSDocument *_document;
    NSArray *_topLevelObjects;
    id _owner;
    struct __wcFlags {
        unsigned int shouldCloseDocument:1;
        unsigned int shouldCascade:1;
        unsigned int nibIsLoaded:1;
        unsigned int nibNameIsPath:1;
        unsigned int RESERVED:28;
    } _wcFlags;
    NSString *_frameAutosaveName;
    id _moreVars;
}

- (id)initWithWindow:(NSWindow *)window;


- (id)initWithWindowNibName:(NSString *)windowNibName;
- (id)initWithWindowNibName:(NSString *)windowNibName owner:(id)owner;
- (id)initWithWindowNibPath:(NSString *)windowNibPath owner:(id)owner;






- (NSString *)windowNibName;


- (NSString *)windowNibPath;


- (id)owner;


- (void)setWindowFrameAutosaveName:(NSString *)name;
- (NSString *)windowFrameAutosaveName;


- (void)setShouldCascadeWindows:(BOOL)flag;
- (BOOL)shouldCascadeWindows;






- (id)document;


- (void)setDocument:(NSDocument *)document;


- (void)setDocumentEdited:(BOOL)dirtyFlag;


- (void)setShouldCloseDocument:(BOOL)flag;
- (BOOL)shouldCloseDocument;






- (void)setWindow:(NSWindow *)window;


- (NSWindow *)window;


- (void)synchronizeWindowTitleWithDocumentName;


- (NSString *)windowTitleForDocumentDisplayName:(NSString *)displayName;


- (void)close;


- (IBAction)showWindow:(id)sender;






- (BOOL)isWindowLoaded;


- (void)windowWillLoad;
- (void)windowDidLoad;


- (void)loadWindow;


@end
