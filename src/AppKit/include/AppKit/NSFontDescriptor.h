#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitDefines.h>
typedef uint32_t NSFontSymbolicTraits;







enum {
    NSFontUnknownClass = 0 << 28,
    NSFontOldStyleSerifsClass = 1 << 28,
    NSFontTransitionalSerifsClass = 2 << 28,
    NSFontModernSerifsClass = 3 << 28,
    NSFontClarendonSerifsClass = 4 << 28,
    NSFontSlabSerifsClass = 5 << 28,
    NSFontFreeformSerifsClass = 7 << 28,
    NSFontSansSerifClass = 8 << 28,
    NSFontOrnamentalsClass = 9 << 28,
    NSFontScriptsClass = 10 << 28,
    NSFontSymbolicClass = 12 << 28
};
typedef uint32_t NSFontFamilyClass;

enum {
    NSFontFamilyClassMask = 0xF0000000
};


enum {
    NSFontItalicTrait = (1 << 0),
    NSFontBoldTrait = (1 << 1),
    NSFontExpandedTrait = (1 << 5),
    NSFontCondensedTrait = (1 << 6),
    NSFontMonoSpaceTrait = (1 << 10),
    NSFontVerticalTrait = (1 << 11),
    NSFontUIOptimizedTrait = (1 << 12)
};

@class NSMutableDictionary, NSDictionary, NSArray, NSSet, NSAffineTransform;

@interface NSFontDescriptor: NSObject <NSCopying, NSCoding> {

    NSMutableDictionary *_attributes;
    id _reserved1;
    id _reserved2;
    id _reserved3;
    id _reserved4;
    id _reserved5;
}


- (NSString *)postscriptName;
- (CGFloat)pointSize;
- (NSAffineTransform *)matrix;
- (NSFontSymbolicTraits)symbolicTraits;

- (id)objectForKey:(NSString *)anAttribute;

- (NSDictionary *)fontAttributes;


+ (NSFontDescriptor *)fontDescriptorWithFontAttributes:(NSDictionary *)attributes;
+ (NSFontDescriptor *)fontDescriptorWithName:(NSString *)fontName size:(CGFloat)size;
+ (NSFontDescriptor *)fontDescriptorWithName:(NSString *)fontName matrix:(NSAffineTransform *)matrix;

- (id)initWithFontAttributes:(NSDictionary *)attributes;



- (NSArray *)matchingFontDescriptorsWithMandatoryKeys:(NSSet *)mandatoryKeys;



- (NSFontDescriptor *)matchingFontDescriptorWithMandatoryKeys:(NSSet *)mandatoryKeys;

- (NSFontDescriptor *)fontDescriptorByAddingAttributes:(NSDictionary *)attributes;
- (NSFontDescriptor *)fontDescriptorWithSymbolicTraits:(NSFontSymbolicTraits)symbolicTraits;
- (NSFontDescriptor *)fontDescriptorWithSize:(CGFloat)newPointSize;
- (NSFontDescriptor *)fontDescriptorWithMatrix:(NSAffineTransform *)matrix;
- (NSFontDescriptor *)fontDescriptorWithFace:(NSString *)newFace;
- (NSFontDescriptor *)fontDescriptorWithFamily:(NSString *)newFamily;
@end



EXTERN_C NSString *NSFontFamilyAttribute;
EXTERN_C NSString *NSFontNameAttribute;
EXTERN_C NSString *NSFontFaceAttribute;
EXTERN_C NSString *NSFontSizeAttribute;
EXTERN_C NSString *NSFontVisibleNameAttribute;

EXTERN_C NSString *NSFontMatrixAttribute ;
EXTERN_C NSString *NSFontVariationAttribute ;
EXTERN_C NSString *NSFontCharacterSetAttribute ;
EXTERN_C NSString *NSFontCascadeListAttribute ;
EXTERN_C NSString *NSFontTraitsAttribute ;
EXTERN_C NSString *NSFontFixedAdvanceAttribute ;
EXTERN_C NSString *NSFontFeatureSettingsAttribute;

EXTERN_C NSString *NSFontColorAttribute;




EXTERN_C NSString *NSFontSymbolicTrait;



EXTERN_C NSString *NSFontWeightTrait;



EXTERN_C NSString *NSFontWidthTrait;



EXTERN_C NSString *NSFontSlantTrait;




EXTERN_C NSString *NSFontVariationAxisIdentifierKey;



EXTERN_C NSString *NSFontVariationAxisMinimumValueKey;



EXTERN_C NSString *NSFontVariationAxisMaximumValueKey;



EXTERN_C NSString *NSFontVariationAxisDefaultValueKey;



EXTERN_C NSString *NSFontVariationAxisNameKey;




EXTERN_C NSString *NSFontFeatureTypeIdentifierKey;



EXTERN_C NSString *NSFontFeatureSelectorIdentifierKey;
