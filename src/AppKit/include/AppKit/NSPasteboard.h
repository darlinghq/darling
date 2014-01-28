#import <AppKit/AppKitDefines.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>
#import <CoreFoundation/CFBase.h>

@class NSArray, NSData, NSFileWrapper, NSMutableDictionary;



EXTERN_C NSString *const NSPasteboardTypeString;
EXTERN_C NSString *const NSPasteboardTypePDF;
EXTERN_C NSString *const NSPasteboardTypeTIFF;
EXTERN_C NSString *const NSPasteboardTypePNG;
EXTERN_C NSString *const NSPasteboardTypeRTF;
EXTERN_C NSString *const NSPasteboardTypeRTFD;
EXTERN_C NSString *const NSPasteboardTypeHTML;
EXTERN_C NSString *const NSPasteboardTypeTabularText;
EXTERN_C NSString *const NSPasteboardTypeFont;
EXTERN_C NSString *const NSPasteboardTypeRuler;
EXTERN_C NSString *const NSPasteboardTypeColor;
EXTERN_C NSString *const NSPasteboardTypeSound;
EXTERN_C NSString *const NSPasteboardTypeMultipleTextSelection;
EXTERN_C NSString *const NSPasteboardTypeTextFinderOptions;




EXTERN_C NSString *NSGeneralPboard;
EXTERN_C NSString *NSFontPboard;
EXTERN_C NSString *NSRulerPboard;
EXTERN_C NSString *NSFindPboard;
EXTERN_C NSString *NSDragPboard;







@class NSPasteboardItem;

@interface NSPasteboard : NSObject
{
    @private
    id _pboard;
    int _gen;
    id _owners;
    CFIndex _cachedTypeNameChangeCount;
    id _cachedTypeNames;
    NSMutableDictionary *_promiseTypeNamesByIdentifier;
    id _support;
    id _pasteboardItems;
    void * _reserved[3];
}

+ (NSPasteboard *)generalPasteboard;
+ (NSPasteboard *)pasteboardWithName:(NSString *)name;
+ (NSPasteboard *)pasteboardWithUniqueName;

- (NSString *)name;

- (NSInteger)changeCount;

- (oneway void)releaseGlobally;




- (NSInteger)clearContents;




- (BOOL)writeObjects:(NSArray *)objects;






- (NSArray *)readObjectsForClasses:(NSArray *)classArray options:(NSDictionary *)options;




- (NSArray *)pasteboardItems;




- (NSUInteger)indexOfPasteboardItem:(NSPasteboardItem *)pasteboardItem;




- (BOOL)canReadItemWithDataConformingToTypes:(NSArray *)types;




- (BOOL)canReadObjectForClasses:(NSArray *)classArray options:(NSDictionary *)options;







- (NSInteger)declareTypes:(NSArray *)newTypes owner:(id)newOwner;



- (NSInteger)addTypes:(NSArray *)newTypes owner:(id)newOwner;



- (NSArray *)types;
- (NSString *)availableTypeFromArray:(NSArray *)types;



- (BOOL)setData:(NSData *)data forType:(NSString *)dataType;
- (BOOL)setPropertyList:(id)plist forType:(NSString *)dataType;
- (BOOL)setString:(NSString *)string forType:(NSString *)dataType;



- (NSData *)dataForType:(NSString *)dataType;
- (id)propertyListForType:(NSString *)dataType;
- (NSString *)stringForType:(NSString *)dataType;


@end


@interface NSPasteboard(FilterServices)
+ (NSArray *)typesFilterableTo:(NSString *)type;

+ (NSPasteboard *)pasteboardByFilteringFile:(NSString *)filename;
+ (NSPasteboard *)pasteboardByFilteringData:(NSData *)data ofType:(NSString *)type;
+ (NSPasteboard *)pasteboardByFilteringTypesInPasteboard:(NSPasteboard *)pboard;
@end


@interface NSObject(NSPasteboardOwner)
- (void)pasteboard:(NSPasteboard *)sender provideDataForType:(NSString *)type;
- (void)pasteboardChangedOwner:(NSPasteboard *)sender;
@end
EXTERN_C NSString *const NSPasteboardURLReadingFileURLsOnlyKey;



EXTERN_C NSString *const NSPasteboardURLReadingContentsConformToTypesKey;







#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6



enum {
    NSPasteboardWritingPromised = 1 << 9,
};
#endif
typedef NSUInteger NSPasteboardWritingOptions;


@protocol NSPasteboardWriting <NSObject>

@required


- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard;


@optional


- (NSPasteboardWritingOptions)writingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard;


@required


- (id)pasteboardPropertyListForType:(NSString *)type;

@end
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
enum {
    NSPasteboardReadingAsData = 0,
    NSPasteboardReadingAsString = 1 << 0,
    NSPasteboardReadingAsPropertyList = 1 << 1,
    NSPasteboardReadingAsKeyedArchive = 1 << 2,
};
#endif
typedef NSUInteger NSPasteboardReadingOptions;


@protocol NSPasteboardReading <NSObject>



@required
+ (NSArray *)readableTypesForPasteboard:(NSPasteboard *)pasteboard;


@optional


+ (NSPasteboardReadingOptions)readingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard;




@optional
- (id)initWithPasteboardPropertyList:(id)propertyList ofType:(NSString *)type;

@end
@interface NSURL(NSPasteboardSupport) <NSPasteboardWriting, NSPasteboardReading>
+ (NSURL *)URLFromPasteboard:(NSPasteboard *)pasteBoard;
- (void)writeToPasteboard:(NSPasteboard *)pasteBoard;
@end




@interface NSString(NSPasteboardSupport) <NSPasteboardWriting, NSPasteboardReading>
@end





EXTERN_C NSString *NSStringPboardType;
EXTERN_C NSString *NSFilenamesPboardType;
EXTERN_C NSString *NSTIFFPboardType;
EXTERN_C NSString *NSRTFPboardType;
EXTERN_C NSString *NSTabularTextPboardType;
EXTERN_C NSString *NSFontPboardType;
EXTERN_C NSString *NSRulerPboardType;
EXTERN_C NSString *NSColorPboardType;
EXTERN_C NSString *NSRTFDPboardType;
EXTERN_C NSString *NSHTMLPboardType;
EXTERN_C NSString *NSURLPboardType;
EXTERN_C NSString *NSPDFPboardType;
EXTERN_C NSString *NSMultipleTextSelectionPboardType;

EXTERN_C NSString *NSPostScriptPboardType;
EXTERN_C NSString *NSVCardPboardType;
EXTERN_C NSString *NSInkTextPboardType;


EXTERN_C NSString *NSFilesPromisePboardType;


EXTERN_C NSString *const NSPasteboardTypeFindPanelSearchOptions;





@interface NSPasteboard(NSFileContents)
- (BOOL)writeFileContents:(NSString *)filename;
- (NSString *)readFileContentsType:(NSString *)type toFile:(NSString *)filename;

- (BOOL)writeFileWrapper:(NSFileWrapper *)wrapper;
- (NSFileWrapper *)readFileWrapper;

@end

EXTERN_C NSString *NSFileContentsPboardType;

EXTERN_C NSString *NSCreateFilenamePboardType(NSString *fileType);
EXTERN_C NSString *NSCreateFileContentsPboardType(NSString *fileType);
EXTERN_C NSString *NSGetFileType(NSString *pboardType);
EXTERN_C NSArray *NSGetFileTypes(NSArray *pboardTypes);
EXTERN_C NSString *NSPICTPboardType;
