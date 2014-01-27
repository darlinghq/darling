#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSArray, NSDictionary, NSFont, NSFontPanel, NSMenu, NSFontDescriptor;

typedef NSUInteger NSFontTraitMask;
enum {
    NSItalicFontMask = 0x00000001,
    NSBoldFontMask = 0x00000002,
    NSUnboldFontMask = 0x00000004,
    NSNonStandardCharacterSetFontMask = 0x00000008,
    NSNarrowFontMask = 0x00000010,
    NSExpandedFontMask = 0x00000020,
    NSCondensedFontMask = 0x00000040,
    NSSmallCapsFontMask = 0x00000080,
    NSPosterFontMask = 0x00000100,
    NSCompressedFontMask = 0x00000200,
    NSFixedPitchFontMask = 0x00000400,
    NSUnitalicFontMask = 0x01000000
};



enum {
   NSFontCollectionApplicationOnlyMask = 1 << 0
};




enum {
    NSNoFontChangeAction = 0,
    NSViaPanelFontAction = 1,
    NSAddTraitFontAction = 2,
    NSSizeUpFontAction = 3,
    NSSizeDownFontAction = 4,
    NSHeavierFontAction = 5,
    NSLighterFontAction = 6,
    NSRemoveTraitFontAction = 7
};
typedef NSUInteger NSFontAction;


@interface NSFontManager : NSObject
{

@private
    NSFontPanel * _panel;
    unsigned int _fmReserved1;
    SEL _action;
    id _actionOrigin;
    id _target;
    NSFont *_selFont;
    struct _fmFlags {
 unsigned int multipleFont:1;
 unsigned int disabled:1;
        unsigned int senderTagMode:2;
 unsigned int _RESERVED:12;
    } _fmFlags;
    unsigned short _fmReserved3;
    id _delegate;
    id _collections;
    id _hiddenCollections;
    NSUInteger _fmReserved4;
}

+ (void)setFontPanelFactory:(Class)factoryId;
+ (void)setFontManagerFactory:(Class)factoryId;
+ (NSFontManager *)sharedFontManager;

- (BOOL)isMultiple;
- (NSFont *)selectedFont;
- (void)setSelectedFont:(NSFont *)fontObj isMultiple:(BOOL)flag;
- (void)setFontMenu:(NSMenu *)newMenu;
- (NSMenu *)fontMenu:(BOOL)create;
- (NSFontPanel *)fontPanel:(BOOL)create;
- (NSFont *)fontWithFamily:(NSString *)family traits:(NSFontTraitMask)traits weight:(NSInteger)weight size:(CGFloat)size;
- (NSFontTraitMask)traitsOfFont:(NSFont *)fontObj;
- (NSInteger)weightOfFont:(NSFont *)fontObj;



- (NSArray *)availableFonts;
- (NSArray *)availableFontFamilies;
- (NSArray *)availableMembersOfFontFamily:(NSString *)fam;

- (NSFont *)convertFont:(NSFont *)fontObj;
- (NSFont *)convertFont:(NSFont *)fontObj toSize:(CGFloat)size;
- (NSFont *)convertFont:(NSFont *)fontObj toFace:(NSString *)typeface;
- (NSFont *)convertFont:(NSFont *)fontObj toFamily:(NSString *)family;
- (NSFont *)convertFont:(NSFont *)fontObj toHaveTrait:(NSFontTraitMask)trait;
- (NSFont *)convertFont:(NSFont *)fontObj toNotHaveTrait:(NSFontTraitMask)trait;
- (NSFont *)convertWeight:(BOOL)upFlag ofFont:(NSFont *)fontObj;
- (BOOL)isEnabled;
- (void)setEnabled:(BOOL)flag;
- (SEL)action;
- (void)setAction:(SEL)aSelector;
- (BOOL)sendAction;
- (void)setDelegate:(id)anObject;
- (id)delegate;

- (NSString *) localizedNameForFamily:(NSString *)family face:(NSString *)faceKey;

- (void)setSelectedAttributes:(NSDictionary *)attributes isMultiple:(BOOL)flag;
- (NSDictionary *)convertAttributes:(NSDictionary *)attributes;

- (NSArray *)availableFontNamesMatchingFontDescriptor:(NSFontDescriptor *)descriptor;

- (NSArray *)collectionNames;
- (NSArray *)fontDescriptorsInCollection:(NSString *)collectionNames;
- (BOOL)addCollection:(NSString *)collectionName options:(NSInteger)collectionOptions;
- (BOOL)removeCollection:(NSString *)collectionName;

- (void)addFontDescriptors:(NSArray *)descriptors toCollection:(NSString *)collectionName;
- (void)removeFontDescriptor:(NSFontDescriptor *)descriptor fromCollection:(NSString *)collection;



- (NSFontAction)currentFontAction;



- (NSFontTraitMask)convertFontTraits:(NSFontTraitMask)traits;

- (void)setTarget:(id)aTarget;
- (id)target;

@end

@interface NSFontManager(NSFontManagerMenuActionMethods)

- (BOOL)fontNamed:(NSString *)fName hasTraits:(NSFontTraitMask)someTraits;



- (NSArray *)availableFontNamesWithTraits:(NSFontTraitMask)someTraits;



- (void)addFontTrait:(id)sender;
- (void)removeFontTrait:(id)sender;
- (void)modifyFontViaPanel:(id)sender;
- (void)modifyFont:(id)sender;
- (void)orderFrontFontPanel:(id)sender;
- (void)orderFrontStylesPanel:(id)sender;

@end



@interface NSObject(NSFontManagerDelegate)
- (BOOL)fontManager:(id)sender willIncludeFont:(NSString *)fontName;
@end



@interface NSObject(NSFontManagerResponderMethod)
- (void)changeFont:(id)sender;
@end
