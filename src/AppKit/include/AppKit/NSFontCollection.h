#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSError.h>
#import <Foundation/NSLocale.h>
#import <Foundation/NSString.h>
#import <AppKit/AppKitDefines.h>


@class NSFontDescriptor;




enum {

    NSFontCollectionVisibilityProcess = (1UL << 0),


    NSFontCollectionVisibilityUser = (1UL << 1),


    NSFontCollectionVisibilityComputer = (1UL << 2)
};
typedef NSUInteger NSFontCollectionVisibility;






@interface NSFontCollection : NSObject <NSCopying, NSMutableCopying, NSCoding>





+ (id)fontCollectionWithDescriptors:(NSArray *)queryDescriptors;


+ (id)fontCollectionWithAllAvailableDescriptors;


+ (id)fontCollectionWithLocale:(NSLocale *)locale;
+ (BOOL)showFontCollection:(NSFontCollection *)collection withName:(NSString *)name visibility:(NSFontCollectionVisibility)visibility error:(NSError **)error;


+ (BOOL)hideFontCollectionWithName:(NSString *)name visibility:(NSFontCollectionVisibility)visibility error:(NSError **)error;


+ (BOOL)renameFontCollectionWithName:(NSString *)name visibility:(NSFontCollectionVisibility)visibility toName:(NSString *)name error:(NSError **)error;


+ (NSArray *)allFontCollectionNames;


+ (id)fontCollectionWithName:(NSString *)name;


+ (id)fontCollectionWithName:(NSString *)name visibility:(NSFontCollectionVisibility)visibility;






- (NSArray *)queryDescriptors;


- (NSArray *)exclusionDescriptors;




- (NSArray *)matchingDescriptors;
- (NSArray *)matchingDescriptorsWithOptions:(NSDictionary *)options;


- (NSArray *)matchingDescriptorsForFamily:(NSString *)family;
- (NSArray *)matchingDescriptorsForFamily:(NSString *)family options:(NSDictionary *)options;

@end





@interface NSMutableFontCollection : NSFontCollection


- (void)setQueryDescriptors:(NSArray *)descriptors;


- (void)setExclusionDescriptors:(NSArray *)descriptors;


- (void)addQueryForDescriptors:(NSArray *)descriptors;


- (void)removeQueryForDescriptors:(NSArray *)descriptors;

@end





EXTERN_C NSString * const NSFontCollectionIncludeDisabledFontsOption;


EXTERN_C NSString * const NSFontCollectionRemoveDuplicatesOption;


EXTERN_C NSString * const NSFontCollectionDisallowAutoActivationOption;







EXTERN_C NSString * const NSFontCollectionDidChangeNotification;


EXTERN_C NSString * const NSFontCollectionActionKey;
EXTERN_C NSString * const NSFontCollectionNameKey;
EXTERN_C NSString * const NSFontCollectionOldNameKey;
EXTERN_C NSString * const NSFontCollectionVisibilityKey;


EXTERN_C NSString * const NSFontCollectionWasShown;
EXTERN_C NSString * const NSFontCollectionWasHidden;
EXTERN_C NSString * const NSFontCollectionWasRenamed;







EXTERN_C NSString * const NSFontCollectionAllFonts;
EXTERN_C NSString * const NSFontCollectionUser;
EXTERN_C NSString * const NSFontCollectionFavorites;
EXTERN_C NSString * const NSFontCollectionRecentlyUsed;
