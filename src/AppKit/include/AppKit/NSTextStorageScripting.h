#import <AppKit/NSTextStorage.h>

@interface NSTextStorage (Scripting)

- (NSArray *)attributeRuns;
- (void)setAttributeRuns:(NSArray *)attributeRuns;

- (NSArray *)paragraphs;
- (void)setParagraphs:(NSArray *)paragraphs;

- (NSArray *)words;
- (void)setWords:(NSArray *)words;

- (NSArray *)characters;
- (void)setCharacters:(NSArray *)characters;

- (NSFont *)font;
- (void)setFont:(NSFont *)font;

- (NSColor *)foregroundColor;
- (void)setForegroundColor:(NSColor *)color;

@end
