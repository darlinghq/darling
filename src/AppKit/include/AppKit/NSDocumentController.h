#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSUserInterfaceValidation.h>
#import <Foundation/NSDate.h>

@class NSArray, NSDocument, NSError, NSMenuItem, NSMutableDictionary, NSOpenPanel, NSURL, NSWindow;

@interface NSDocumentController : NSObject<NSCoding, NSUserInterfaceValidations> {
    @private
    id _documents;
    id _moreVars;
    NSArray *_cachedTypeDescriptions;
    NSMutableDictionary *_recents;
    int _recentsLimit;
}







+ (id)sharedDocumentController;



- (id)init;





- (NSArray *)documents;



- (id)currentDocument;



- (NSString *)currentDirectory;





- (id)documentForURL:(NSURL *)url;



- (id)documentForWindow:(NSWindow *)window;



- (void)addDocument:(NSDocument *)document;
- (void)removeDocument:(NSDocument *)document;





- (IBAction)newDocument:(id)sender;







- (id)openUntitledDocumentAndDisplay:(BOOL)displayDocument error:(NSError **)outError;





- (id)makeUntitledDocumentOfType:(NSString *)typeName error:(NSError **)outError;





- (IBAction)openDocument:(id)sender;



- (NSArray *)URLsFromRunningOpenPanel;



- (NSInteger)runModalOpenPanel:(NSOpenPanel *)openPanel forTypes:(NSArray *)types;





- (void)beginOpenPanelWithCompletionHandler:(void (^)(NSArray *))completionHandler;





- (void)beginOpenPanel:(NSOpenPanel *)openPanel forTypes:(NSArray *)inTypes completionHandler:(void (^)(NSInteger result))completionHandler;

#if NS_BLOCKS_AVAILABLE
- (void)openDocumentWithContentsOfURL:(NSURL *)url display:(BOOL)displayDocument completionHandler:(void (^)(NSDocument *document, BOOL documentWasAlreadyOpen, NSError *error))completionHandler;

#endif





- (id)makeDocumentWithContentsOfURL:(NSURL *)url ofType:(NSString *)typeName error:(NSError **)outError;



#if NS_BLOCKS_AVAILABLE
- (void)reopenDocumentForURL:(NSURL *)urlOrNil withContentsOfURL:(NSURL *)contentsURL display:(BOOL)displayDocument completionHandler:(void (^)(NSDocument *document, BOOL documentWasAlreadyOpen, NSError *error))completionHandler;

#endif



- (id)makeDocumentForURL:(NSURL *)urlOrNil withContentsOfURL:(NSURL *)contentsURL ofType:(NSString *)typeName error:(NSError **)outError;





- (void)setAutosavingDelay:(NSTimeInterval)autosavingDelay;
- (NSTimeInterval)autosavingDelay;





- (IBAction)saveAllDocuments:(id)sender;





- (BOOL)hasEditedDocuments;





- (void)reviewUnsavedDocumentsWithAlertTitle:(NSString *)title cancellable:(BOOL)cancellable delegate:(id)delegate didReviewAllSelector:(SEL)didReviewAllSelector contextInfo:(void *)contextInfo;





- (void)closeAllDocumentsWithDelegate:(id)delegate didCloseAllSelector:(SEL)didCloseAllSelector contextInfo:(void *)contextInfo;


- (NSDocument *)duplicateDocumentWithContentsOfURL:(NSURL *)url copying:(BOOL)duplicateByCopying displayName:(NSString *)displayNameOrNil error:(NSError **)outError;


- (void)presentError:(NSError *)error modalForWindow:(NSWindow *)window delegate:(id)delegate didPresentSelector:(SEL)didPresentSelector contextInfo:(void *)contextInfo;
- (BOOL)presentError:(NSError *)error;
- (NSError *)willPresentError:(NSError *)error;





- (NSUInteger)maximumRecentDocumentCount;



- (IBAction)clearRecentDocuments:(id)sender;



- (void)noteNewRecentDocument:(NSDocument *)document;



- (void)noteNewRecentDocumentURL:(NSURL *)url;



- (NSArray *)recentDocumentURLs;





- (NSString *)defaultType;





- (NSString *)typeForContentsOfURL:(NSURL *)url error:(NSError **)outError;



- (NSArray *)documentClassNames;



- (Class)documentClassForType:(NSString *)typeName;



- (NSString *)displayNameForType:(NSString *)typeName;





- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem;

@end

@interface NSDocumentController(NSDeprecated)





- (id)openDocumentWithContentsOfURL:(NSURL *)url display:(BOOL)displayDocument error:(NSError **)outError;
- (BOOL)reopenDocumentForURL:(NSURL *)url withContentsOfURL:(NSURL *)contentsURL error:(NSError **)outError;



- (NSArray *)fileExtensionsFromType:(NSString *)typeName;
- (NSString *)typeFromFileExtension:(NSString *)fileNameExtensionOrHFSFileType;



- (id)documentForFileName:(NSString *)fileName;
- (NSArray *)fileNamesFromRunningOpenPanel;
- (id)makeDocumentWithContentsOfFile:(NSString *)fileName ofType:(NSString *)type;
- (id)makeDocumentWithContentsOfURL:(NSURL *)url ofType:(NSString *)type;
- (id)makeUntitledDocumentOfType:(NSString *)type;
- (id)openDocumentWithContentsOfFile:(NSString *)fileName display:(BOOL)display;
- (id)openDocumentWithContentsOfURL:(NSURL *)url display:(BOOL)display;
- (id)openUntitledDocumentOfType:(NSString*)type display:(BOOL)display;
- (void)setShouldCreateUI:(BOOL)flag;
- (BOOL)shouldCreateUI;

@end
