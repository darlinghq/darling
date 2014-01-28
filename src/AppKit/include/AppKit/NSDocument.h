#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSUserInterfaceValidation.h>
//#import <Foundation/NSFilePresenter.h>

@protocol NSFilePresenter;

@class NSArray, NSData, NSDate, NSDictionary, NSError, NSFileWrapper, NSMenuItem, NSPageLayout, NSPrintInfo, NSPrintOperation, NSSavePanel, NSUndoManager, NSURL, NSView, NSWindow, NSWindowController;

enum {



    NSChangeDone = 0,



    NSChangeUndone = 1,



#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
    NSChangeRedone = 5,
#endif



    NSChangeCleared = 2,



    NSChangeReadOtherContents = 3,



    NSChangeAutosaved = 4,

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7



    NSChangeDiscardable = 256

#endif

};
typedef NSUInteger NSDocumentChangeType;

enum {



    NSSaveOperation = 0,



    NSSaveAsOperation = 1,



    NSSaveToOperation = 2,



#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    NSAutosaveInPlaceOperation = 4,
#endif



#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    NSAutosaveElsewhereOperation = 3,
#endif



    NSAutosaveAsOperation,



    NSAutosaveOperation = 3

};
typedef NSUInteger NSSaveOperationType;

@interface NSDocument : NSObject<NSFilePresenter, NSUserInterfaceValidations> {
    @private
    NSWindow *_window;
    id _windowControllers;
    NSURL *_fileURL;
    NSString *_fileType;
    NSPrintInfo *_printInfo;
    long _documentReserved;
    NSView *savePanelAccessory;
    id _displayName;
    id _privateData;
    NSUndoManager *_undoManager;
    struct __docFlags {
 unsigned int inClose:1;
 unsigned int hasUndoManager:1;
 unsigned int unused:1;
 unsigned int reconciledToFileName:1;
 unsigned int checkingDisplayName:1;
 unsigned int hasInvalidRestorableState:1;
 unsigned int hasEverHadInvalidRestorableState:1;
 unsigned int RESERVED:25;
    } _docFlags;
    NSString *_savePanelSaveType;
}







- (id)init;







- (id)initWithType:(NSString *)typeName error:(NSError **)outError;





+ (BOOL)canConcurrentlyReadDocumentsOfType:(NSString *)typeName;





- (id)initWithContentsOfURL:(NSURL *)url ofType:(NSString *)typeName error:(NSError **)outError;









- (id)initForURL:(NSURL *)urlOrNil withContentsOfURL:(NSURL *)contentsURL ofType:(NSString *)typeName error:(NSError **)outError;








- (void)setFileType:(NSString *)typeName;
- (NSString *)fileType;







- (void)setFileURL:(NSURL *)url;
- (NSURL *)fileURL;



- (void)setFileModificationDate:(NSDate *)modificationDate;
- (NSDate *)fileModificationDate;



- (void)setDraft:(BOOL)flag;
- (BOOL)isDraft;



#if NS_BLOCKS_AVAILABLE
- (void)performActivityWithSynchronousWaiting:(BOOL)waitSynchronously usingBlock:(void (^)(void (^activityCompletionHandler)(void)))block;





- (void)continueActivityUsingBlock:(void (^)(void))block;







- (void)continueAsynchronousWorkOnMainThreadUsingBlock:(void (^)(void))block;
- (void)performSynchronousFileAccessUsingBlock:(void (^)(void))block;





- (void)performAsynchronousFileAccessUsingBlock:(void (^)(void (^fileAccessCompletionHandler)(void)))block;

#endif







- (IBAction)revertDocumentToSaved:(id)sender;





- (BOOL)revertToContentsOfURL:(NSURL *)url ofType:(NSString *)typeName error:(NSError **)outError;









- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)typeName error:(NSError **)outError;





- (BOOL)readFromFileWrapper:(NSFileWrapper *)fileWrapper ofType:(NSString *)typeName error:(NSError **)outError;





- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError;



- (BOOL)isEntireFileLoaded;







- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)typeName error:(NSError **)outError;





- (NSFileWrapper *)fileWrapperOfType:(NSString *)typeName error:(NSError **)outError;





- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError;



- (void)unblockUserInteraction;





- (BOOL)autosavingIsImplicitlyCancellable;


- (BOOL)writeSafelyToURL:(NSURL *)url ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation error:(NSError **)outError;





- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation originalContentsURL:(NSURL *)absoluteOriginalContentsURL error:(NSError **)outError;
- (NSDictionary *)fileAttributesToWriteToURL:(NSURL *)url ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation originalContentsURL:(NSURL *)absoluteOriginalContentsURL error:(NSError **)outError;



- (BOOL)keepBackupFile;
- (NSURL *)backupFileURL;





- (IBAction)saveDocument:(id)sender;



- (IBAction)saveDocumentAs:(id)sender;



- (IBAction)saveDocumentTo:(id)sender;
- (void)saveDocumentWithDelegate:(id)delegate didSaveSelector:(SEL)didSaveSelector contextInfo:(void *)contextInfo;
- (void)runModalSavePanelForSaveOperation:(NSSaveOperationType)saveOperation delegate:(id)delegate didSaveSelector:(SEL)didSaveSelector contextInfo:(void *)contextInfo;



- (BOOL)shouldRunSavePanelWithAccessoryView;



- (BOOL)prepareSavePanel:(NSSavePanel *)savePanel;



- (BOOL)fileNameExtensionWasHiddenInLastRunSavePanel;



- (NSString *)fileTypeFromLastRunSavePanel;







- (void)saveToURL:(NSURL *)url ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation delegate:(id)delegate didSaveSelector:(SEL)didSaveSelector contextInfo:(void *)contextInfo;

#if NS_BLOCKS_AVAILABLE







- (void)saveToURL:(NSURL *)url ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation completionHandler:(void (^)(NSError *errorOrNil))completionHandler;

#endif



- (BOOL)canAsynchronouslyWriteToURL:(NSURL *)url ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation;









- (BOOL)checkAutosavingSafetyAndReturnError:(NSError **)outError;



- (void)scheduleAutosaving;



- (BOOL)hasUnautosavedChanges;
- (void)autosaveDocumentWithDelegate:(id)delegate didAutosaveSelector:(SEL)didAutosaveSelector contextInfo:(void *)contextInfo;

#if NS_BLOCKS_AVAILABLE





- (void)autosaveWithImplicitCancellability:(BOOL)autosavingIsImplicitlyCancellable completionHandler:(void (^)(NSError *errorOrNil))completionHandler;

#endif





+ (BOOL)autosavesInPlace;





+ (BOOL)preservesVersions;



- (IBAction)browseDocumentVersions:(id)sender;





+ (BOOL)autosavesDrafts;



- (NSString *)autosavingFileType;





- (void)setAutosavedContentsFileURL:(NSURL *)url;
- (NSURL *)autosavedContentsFileURL;









- (void)canCloseDocumentWithDelegate:(id)delegate shouldCloseSelector:(SEL)shouldCloseSelector contextInfo:(void *)contextInfo;



- (void)close;





- (IBAction)duplicateDocument:(id)sender;







- (void)duplicateDocumentWithDelegate:(id)delegate didDuplicateSelector:(SEL)didDuplicateSelector contextInfo:(void *)contextInfo;







- (NSDocument *)duplicateAndReturnError:(NSError **)outError;





- (IBAction)renameDocument:(id)sender;





- (IBAction)moveDocumentToUbiquityContainer:(id)sender;



- (IBAction)moveDocument:(id)sender;





- (void)moveDocumentWithCompletionHandler:(void (^)(BOOL didMove))completionHandler;





- (void)moveToURL:(NSURL *)url completionHandler:(void (^)(NSError *))completionHandler;





- (IBAction)lockDocument:(id)sender;
- (IBAction)unlockDocument:(id)sender;





- (void)lockDocumentWithCompletionHandler:(void (^)(BOOL didLock))completionHandler;





- (void)lockWithCompletionHandler:(void (^)(NSError *))completionHandler;





- (void)unlockDocumentWithCompletionHandler:(void (^)(BOOL didUnlock))completionHandler;





- (void)unlockWithCompletionHandler:(void (^)(NSError *))completionHandler;



- (BOOL)isLocked;





- (IBAction)runPageLayout:(id)sender;







- (void)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)printInfo delegate:(id)delegate didRunSelector:(SEL)didRunSelector contextInfo:(void *)contextInfo;



- (BOOL)preparePageLayout:(NSPageLayout *)pageLayout;



- (BOOL)shouldChangePrintInfo:(NSPrintInfo *)newPrintInfo;



- (void)setPrintInfo:(NSPrintInfo *)printInfo;
- (NSPrintInfo *)printInfo;



- (IBAction)printDocument:(id)sender;
- (void)printDocumentWithSettings:(NSDictionary *)printSettings showPrintPanel:(BOOL)showPrintPanel delegate:(id)delegate didPrintSelector:(SEL)didPrintSelector contextInfo:(void *)contextInfo;



- (NSPrintOperation *)printOperationWithSettings:(NSDictionary *)printSettings error:(NSError **)outError;





- (void)runModalPrintOperation:(NSPrintOperation *)printOperation delegate:(id)delegate didRunSelector:(SEL)didRunSelector contextInfo:(void *)contextInfo;





- (BOOL)isDocumentEdited;



- (BOOL)isInViewingMode;



- (void)updateChangeCount:(NSDocumentChangeType)change;



- (id)changeCountTokenForSaveOperation:(NSSaveOperationType)saveOperation;





- (void)updateChangeCountWithToken:(id)changeCountToken forSaveOperation:(NSSaveOperationType)saveOperation;



- (void)setUndoManager:(NSUndoManager *)undoManager;
- (NSUndoManager *)undoManager;



- (void)setHasUndoManager:(BOOL)hasUndoManager;
- (BOOL)hasUndoManager;


- (void)presentError:(NSError *)error modalForWindow:(NSWindow *)window delegate:(id)delegate didPresentSelector:(SEL)didPresentSelector contextInfo:(void *)contextInfo;
- (BOOL)presentError:(NSError *)error;
- (NSError *)willPresentError:(NSError *)error;



- (void)willNotPresentError:(NSError *)error;





- (void)makeWindowControllers;



- (NSString *)windowNibName;



- (void)windowControllerWillLoadNib:(NSWindowController *)windowController;
- (void)windowControllerDidLoadNib:(NSWindowController *)windowController;



- (void)setWindow:(NSWindow *)window;



- (void)addWindowController:(NSWindowController *)windowController;
- (void)removeWindowController:(NSWindowController *)windowController;



- (void)showWindows;



- (NSArray *)windowControllers;







- (void)shouldCloseWindowController:(NSWindowController *)windowController delegate:(id)delegate shouldCloseSelector:(SEL)shouldCloseSelector contextInfo:(void *)contextInfo;



- (void)setDisplayName:(NSString *)displayNameOrNil;



- (NSString *)displayName;





- (NSString *)defaultDraftName;



- (NSWindow *)windowForSheet;





+ (NSArray *)readableTypes;



+ (NSArray *)writableTypes;



+ (BOOL)isNativeType:(NSString *)type;







- (NSArray *)writableTypesForSaveOperation:(NSSaveOperationType)saveOperation;





- (NSString *)fileNameExtensionForType:(NSString *)typeName saveOperation:(NSSaveOperationType)saveOperation;





- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem;




+ (BOOL)usesUbiquitousStorage;

@end

@interface NSDocument(NSDeprecated)





- (BOOL)saveToURL:(NSURL *)url ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation error:(NSError **)outError;



- (NSData *)dataRepresentationOfType:(NSString *)type;
- (NSDictionary *)fileAttributesToWriteToFile:(NSString *)fullDocumentPath ofType:(NSString *)documentTypeName saveOperation:(NSSaveOperationType)saveOperationType;
- (NSString *)fileName;
- (NSFileWrapper *)fileWrapperRepresentationOfType:(NSString *)type;
- (id)initWithContentsOfFile:(NSString *)absolutePath ofType:(NSString *)typeName;
- (id)initWithContentsOfURL:(NSURL *)url ofType:(NSString *)typeName;
- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)type;
- (BOOL)loadFileWrapperRepresentation:(NSFileWrapper *)wrapper ofType:(NSString *)type;
- (void)printShowingPrintPanel:(BOOL)flag;
- (BOOL)readFromFile:(NSString *)fileName ofType:(NSString *)type;
- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)type;
- (BOOL)revertToSavedFromFile:(NSString *)fileName ofType:(NSString *)type;
- (BOOL)revertToSavedFromURL:(NSURL *)url ofType:(NSString *)type;
- (NSInteger)runModalPageLayoutWithPrintInfo:(NSPrintInfo *)printInfo;
- (void)saveToFile:(NSString *)fileName saveOperation:(NSSaveOperationType)saveOperation delegate:(id)delegate didSaveSelector:(SEL)didSaveSelector contextInfo:(void *)contextInfo;
- (void)setFileName:(NSString *)fileName;
- (BOOL)writeToFile:(NSString *)fileName ofType:(NSString *)type;
- (BOOL)writeToFile:(NSString *)fullDocumentPath ofType:(NSString *)documentTypeName originalFile:(NSString *)fullOriginalDocumentPath saveOperation:(NSSaveOperationType)saveOperationType;
- (BOOL)writeToURL:(NSURL *)url ofType:(NSString *)type;
- (BOOL)writeWithBackupToFile:(NSString *)fullDocumentPath ofType:(NSString *)documentTypeName saveOperation:(NSSaveOperationType)saveOperationType;

@end
