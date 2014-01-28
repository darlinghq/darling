#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>
#import <CoreFoundation/CFDictionary.h>

@class NSArray, NSMutableArray, NSColor, NSBundle;

@interface NSColorList : NSObject <NSCoding> {

    NSMutableArray *_keyArray;
    NSMutableArray *_colorArray;
    CFMutableDictionaryRef _keyToIndexTable;
    NSString *_name;
    NSString *_printerType;
    NSString *_fileName;
    struct _colorListFlags {
 unsigned int colorsLoaded:1;
 unsigned int editable:1;
 unsigned int hasDeviceSpecificLists:1;
 unsigned int dirty:1;
 unsigned int hasFrozen:1;
 unsigned int notificationsDisabled:1;
        unsigned int hasAttemptedLoadingBundleForDirectory:1;
 unsigned int isProfileBased:1;
 unsigned int :24;
#if __LP64__
 unsigned int :32;
#endif
    } _flags;
    id _clAuxiliaryStorage;
}



+ (NSArray *)availableColorLists;



+ (NSColorList *)colorListNamed:(NSString *)name;



- (id)initWithName:(NSString *)name;
- (id)initWithName:(NSString *)name fromFile:(NSString *)path;



- (NSString *)name;



- (void)setColor:(NSColor *)color forKey:(NSString *)key;



- (void)insertColor:(NSColor *)color key:(NSString *)key atIndex:(NSUInteger)loc;



- (void)removeColorWithKey:(NSString *)key;



- (NSColor *)colorWithKey:(NSString *)key;



- (NSArray *)allKeys;



- (BOOL)isEditable;



- (BOOL)writeToFile:(NSString *)path;



- (void)removeFile;

@end


EXTERN_C NSString *NSColorListDidChangeNotification;
