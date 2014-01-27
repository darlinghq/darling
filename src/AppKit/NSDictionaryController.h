#import <AppKit/NSArrayController.h>


@class NSDictionary;





@interface NSObject (NSDictionaryControllerKeyValuePair)

- (void)setLocalizedKey:(NSString *)localizedKey;
- (NSString *)localizedKey;
- (void)setKey:(NSString *)key;
- (NSString *)key;
- (void)setValue:(id)value;
- (id)value;
- (BOOL)isExplicitlyIncluded;

@end


@interface NSDictionaryController : NSArrayController {
@private
    void *_reserved5;
    void *_reserved6;
    void *_reserved7;
 id _contentDictionary;
 NSString *_initialKey;
 id _initialValue;
 NSUInteger _minimumInsertionKeyIndex;
 NSString *_localizedKeyStringsFileName;
 NSDictionary *_localizedKeyForKeyDictionary;
 NSDictionary *_keyForLocalizedKeyDictionary;
 NSArray *_includedKeys;
 NSArray *_excludedKeys;
    struct __dictionaryControllerFlags {
        unsigned int _deepCopiesValues:1;
        unsigned int _suppressBuildingDictionary:1;
        unsigned int _reservedDictionaryController:30;
    } _dictionaryControllerFlags;
}

- (id)newObject;


- (void)setInitialKey:(NSString *)key;
- (NSString *)initialKey;
- (void)setInitialValue:(id)value;
- (id)initialValue;


- (void)setIncludedKeys:(NSArray *)keys;
- (NSArray *)includedKeys;
- (void)setExcludedKeys:(NSArray *)keys;
- (NSArray *)excludedKeys;


- (void)setLocalizedKeyDictionary:(NSDictionary *)dictionary;
- (NSDictionary *)localizedKeyDictionary;
- (void)setLocalizedKeyTable:(NSString *)stringsFileName;
- (NSString *)localizedKeyTable;

@end
