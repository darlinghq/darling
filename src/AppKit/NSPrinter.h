#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>

@class NSArray, NSDictionary, NSString;



enum {
    NSPrinterTableOK = 0,
    NSPrinterTableNotFound = 1,
    NSPrinterTableError = 2
};
typedef NSUInteger NSPrinterTableStatus;

@interface NSPrinter: NSObject<NSCopying, NSCoding> {
    @private
    NSString *_printerName;
    void *_printer;
    NSDictionary *_cachedDeviceDescription;
    int _ppdCreationNum;
    void *_ppdNodes;
    void *_ppdPriv;
#if __LP64__
    id _reserved[3];
#else
    unsigned char _compatibilityPadding[20];
#endif
}



+ (NSArray *)printerNames;



+ (NSArray *)printerTypes;



+ (NSPrinter *)printerWithName:(NSString *)name;



+ (NSPrinter *)printerWithType:(NSString *)type;



- (NSString *)name;



- (NSString *)type;



- (NSInteger)languageLevel;



- (NSSize)pageSizeForPaper:(NSString *)paperName;



- (NSPrinterTableStatus)statusForTable:(NSString *)tableName;



- (BOOL)isKey:(NSString *)key inTable:(NSString *)table;
- (BOOL)booleanForKey:(NSString *)key inTable:(NSString *)table;
- (float)floatForKey:(NSString *)key inTable:(NSString *)table;
- (int)intForKey:(NSString *)key inTable:(NSString *)table;
- (NSRect)rectForKey:(NSString *)key inTable:(NSString *)table;
- (NSSize)sizeForKey:(NSString *)key inTable:(NSString *)table;
- (NSString *)stringForKey:(NSString *)key inTable:(NSString *)table;
- (NSArray *)stringListForKey:(NSString *)key inTable:(NSString *)table;



- (NSDictionary *)deviceDescription;

@end

@interface NSPrinter(NSDeprecated)



- (NSRect)imageRectForPaper:(NSString *)paperName;



- (BOOL)acceptsBinary;
- (BOOL)isColor;
- (BOOL)isFontAvailable:(NSString *)faceName;
- (BOOL)isOutputStackInReverseOrder;



+ (NSPrinter *)printerWithName:(NSString *)name domain:(NSString *)domain includeUnavailable:(BOOL)flag;
- (NSString *)domain;
- (NSString *)host;
- (NSString *)note;

@end
