#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

@class NSMutableDictionary, NSPrinter;



enum {
    NSPortraitOrientation = 0,
    NSLandscapeOrientation = 1
};
typedef NSUInteger NSPrintingOrientation;



enum {
    NSAutoPagination = 0,
    NSFitPagination = 1,
    NSClipPagination = 2
};
typedef NSUInteger NSPrintingPaginationMode;



EXTERN_C NSString *const NSPrintSpoolJob;
EXTERN_C NSString *const NSPrintPreviewJob;
EXTERN_C NSString *const NSPrintSaveJob;
EXTERN_C NSString *const NSPrintCancelJob;



EXTERN_C NSString *const NSPrintPaperName;
EXTERN_C NSString *const NSPrintPaperSize;
EXTERN_C NSString *const NSPrintOrientation;
EXTERN_C NSString *const NSPrintScalingFactor;



EXTERN_C NSString *const NSPrintLeftMargin;
EXTERN_C NSString *const NSPrintRightMargin;
EXTERN_C NSString *const NSPrintTopMargin;
EXTERN_C NSString *const NSPrintBottomMargin;
EXTERN_C NSString *const NSPrintHorizontallyCentered;
EXTERN_C NSString *const NSPrintVerticallyCentered;
EXTERN_C NSString *const NSPrintHorizontalPagination;
EXTERN_C NSString *const NSPrintVerticalPagination;



EXTERN_C NSString *const NSPrintPrinter;
EXTERN_C NSString *const NSPrintCopies;
EXTERN_C NSString *const NSPrintAllPages;
EXTERN_C NSString *const NSPrintFirstPage;
EXTERN_C NSString *const NSPrintLastPage;
EXTERN_C NSString *const NSPrintMustCollate;
EXTERN_C NSString *const NSPrintReversePageOrder;
EXTERN_C NSString *const NSPrintJobDisposition;
EXTERN_C NSString *const NSPrintPagesAcross;
EXTERN_C NSString *const NSPrintPagesDown;
EXTERN_C NSString *const NSPrintTime;
EXTERN_C NSString *const NSPrintDetailedErrorReporting;
EXTERN_C NSString *const NSPrintFaxNumber;
EXTERN_C NSString *const NSPrintPrinterName;
EXTERN_C NSString *const NSPrintSelectionOnly;
EXTERN_C NSString *const NSPrintJobSavingURL;
EXTERN_C NSString *const NSPrintJobSavingFileNameExtensionHidden;



EXTERN_C NSString *const NSPrintHeaderAndFooter;

@interface NSPrintInfo : NSObject<NSCopying, NSCoding> {
    @private
    NSMutableDictionary *_attributes;
    id _moreVars;
}



+ (void)setSharedPrintInfo:(NSPrintInfo *)printInfo;
+ (NSPrintInfo *)sharedPrintInfo;



- (id)initWithDictionary:(NSDictionary *)attributes;



- (NSMutableDictionary *)dictionary;



- (void)setPaperName:(NSString *)name;
- (void)setPaperSize:(NSSize)size;
- (void)setOrientation:(NSPrintingOrientation)orientation;
- (void)setScalingFactor:(CGFloat)scalingFactor;
- (NSString *)paperName;
- (NSSize)paperSize;
- (NSPrintingOrientation)orientation;
- (CGFloat)scalingFactor;



- (void)setLeftMargin:(CGFloat)margin;
- (void)setRightMargin:(CGFloat)margin;
- (void)setTopMargin:(CGFloat)margin;
- (void)setBottomMargin:(CGFloat)margin;
- (CGFloat)leftMargin;
- (CGFloat)rightMargin;
- (CGFloat)topMargin;
- (CGFloat)bottomMargin;
- (void)setHorizontallyCentered:(BOOL)flag;
- (void)setVerticallyCentered:(BOOL)flag;
- (BOOL)isHorizontallyCentered;
- (BOOL)isVerticallyCentered;
- (void)setHorizontalPagination:(NSPrintingPaginationMode)mode;
- (void)setVerticalPagination:(NSPrintingPaginationMode)mode;
- (NSPrintingPaginationMode)horizontalPagination;
- (NSPrintingPaginationMode)verticalPagination;



- (void)setJobDisposition:(NSString *)disposition;
- (NSString *)jobDisposition;



- (void)setPrinter:(NSPrinter *)printer;
- (NSPrinter *)printer;



- (void)setUpPrintOperationDefaultValues;




- (NSRect)imageablePageBounds;





- (NSString *)localizedPaperName;




+ (NSPrinter *)defaultPrinter;




- (NSMutableDictionary *)printSettings;



- (void *)PMPrintSession ;
- (void *)PMPageFormat ;
- (void *)PMPrintSettings ;



- (void)updateFromPMPageFormat;
- (void)updateFromPMPrintSettings;





- (void)setSelectionOnly:(BOOL)selectionOnly;
- (BOOL)isSelectionOnly;

@end

@interface NSPrintInfo(NSDeprecated)



+ (void)setDefaultPrinter:(NSPrinter *)printer;



+ (NSSize)sizeForPaperName:(NSString *)name;

@end



EXTERN_C NSString *const NSPrintFormName;
EXTERN_C NSString *const NSPrintJobFeatures;
EXTERN_C NSString *const NSPrintManualFeed;
EXTERN_C NSString *const NSPrintPagesPerSheet;
EXTERN_C NSString *const NSPrintPaperFeed;



EXTERN_C NSString *const NSPrintSavePath;
