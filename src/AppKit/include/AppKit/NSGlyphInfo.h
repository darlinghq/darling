#import <Foundation/NSString.h>
#import <AppKit/NSFont.h>

enum {
    NSIdentityMappingCharacterCollection = 0,
    NSAdobeCNS1CharacterCollection = 1,
    NSAdobeGB1CharacterCollection = 2,
    NSAdobeJapan1CharacterCollection = 3,
    NSAdobeJapan2CharacterCollection = 4,
    NSAdobeKorea1CharacterCollection = 5,
};
typedef NSUInteger NSCharacterCollection;

@interface NSGlyphInfo : NSObject <NSCopying, NSCoding> {
    NSString *_baseString;
}


+ (NSGlyphInfo *)glyphInfoWithGlyphName:(NSString *)glyphName forFont:(NSFont *)font baseString:(NSString *)theString;


+ (NSGlyphInfo *)glyphInfoWithGlyph:(NSGlyph)glyph forFont:(NSFont *)font baseString:(NSString *)theString;


+ (NSGlyphInfo *)glyphInfoWithCharacterIdentifier:(NSUInteger)cid collection:(NSCharacterCollection)characterCollection baseString:(NSString *)theString;


- (NSString *)glyphName;


- (NSUInteger)characterIdentifier;


- (NSCharacterCollection)characterCollection;
@end
