#import <Foundation/NSGeometry.h>
#import <Foundation/NSRange.h>
#import <AppKit/AppKitDefines.h>

@class NSMutableData, NSGraphicsContext, NSPrintPanel, NSPrintInfo, NSView, NSWindow;



enum {
    NSDescendingPageOrder = -1,
    NSSpecialPageOrder = 0,
    NSAscendingPageOrder = 1,
    NSUnknownPageOrder = 2
};
typedef NSInteger NSPrintingPageOrder;

#if MAC_OS_X_VERSION_10_7 <= MAC_OS_X_VERSION_MAX_ALLOWED
enum {


    NSPrintRenderingQualityBest,




    NSPrintRenderingQualityResponsive
};
#endif
typedef NSInteger NSPrintRenderingQuality;



EXTERN_C NSString *NSPrintOperationExistsException;

@interface NSPrintOperation : NSObject {
}



+ (NSPrintOperation *)printOperationWithView:(NSView *)view printInfo:(NSPrintInfo *)printInfo;
+ (NSPrintOperation *)PDFOperationWithView:(NSView *)view insideRect:(NSRect)rect toData:(NSMutableData *)data printInfo:(NSPrintInfo *)printInfo;
+ (NSPrintOperation *)PDFOperationWithView:(NSView *)view insideRect:(NSRect)rect toPath:(NSString *)path printInfo:(NSPrintInfo *)printInfo;
+ (NSPrintOperation *)EPSOperationWithView:(NSView *)view insideRect:(NSRect)rect toData:(NSMutableData *)data printInfo:(NSPrintInfo *)printInfo;
+ (NSPrintOperation *)EPSOperationWithView:(NSView *)view insideRect:(NSRect)rect toPath:(NSString *)path printInfo:(NSPrintInfo *)printInfo;



+ (NSPrintOperation *)printOperationWithView:(NSView *)view;
+ (NSPrintOperation *)PDFOperationWithView:(NSView *)view insideRect:(NSRect)rect toData:(NSMutableData *)data;
+ (NSPrintOperation *)EPSOperationWithView:(NSView *)view insideRect:(NSRect)rect toData:(NSMutableData *)data;



+ (NSPrintOperation *)currentOperation;
+ (void)setCurrentOperation:(NSPrintOperation *)operation;



- (BOOL)isCopyingOperation;



- (NSPrintRenderingQuality)preferredRenderingQuality;



- (void)setJobTitle:(NSString *)jobTitle;
- (NSString *)jobTitle;





- (void)setShowsPrintPanel:(BOOL)flag;
- (BOOL)showsPrintPanel;
- (void)setShowsProgressPanel:(BOOL)flag;
- (BOOL)showsProgressPanel;




- (void)setPrintPanel:(NSPrintPanel *)panel;
- (NSPrintPanel *)printPanel;



- (void)setCanSpawnSeparateThread:(BOOL)canSpawnSeparateThread;
- (BOOL)canSpawnSeparateThread;



- (void)setPageOrder:(NSPrintingPageOrder)pageOrder;
- (NSPrintingPageOrder)pageOrder;







- (void)runOperationModalForWindow:(NSWindow *)docWindow delegate:(id)delegate didRunSelector:(SEL)didRunSelector contextInfo:(void *)contextInfo;



- (BOOL)runOperation;



- (NSView *)view;



- (NSPrintInfo *)printInfo;
- (void)setPrintInfo:(NSPrintInfo *)printInfo;



- (NSGraphicsContext *)context;



- (NSRange)pageRange;




- (NSInteger)currentPage;



- (NSGraphicsContext *)createContext;
- (void)destroyContext;
- (BOOL)deliverResult;
- (void)cleanUpOperation;

@end

@interface NSPrintOperation(NSDeprecated)



- (void)setAccessoryView:(NSView *)view;
- (NSView *)accessoryView;

- (void)setJobStyleHint:(NSString *)hint;
- (NSString *)jobStyleHint;



- (void)setShowPanels:(BOOL)flag;
- (BOOL)showPanels;

@end
