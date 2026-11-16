//
//  NSBidiHelper.h
//  AppKit
//
//  Created by Airy ANDRE on 05/12/12.
//
//
#import <Foundation/Foundation.h>

#ifdef __cplusplus
extern "C" {
#endif
// Returns the bidi levels for the given text
// Base level is the base bidi level for the paragraph - pass -1 to guess it
// according to the text content
NSUInteger NSBidiHelperParagraph(NSInteger *baselevel, unichar *text,
                                 uint8_t *levels, NSUInteger textLevel);

// Process a line of text, starting with baseLevel
// glyphs : the glyphs to reorder to the display order - can be NULL (can
// actually be any 32 bits info) text   : the text corresponding to the line to
// process - can be NULL levels : the bidi levels for the line mirror : 1 to
// process the text about char mirroring (ie: a "(" in a right-to-left run
// switches to ")") length : length of the text
void NSBidiHelperProcessLine(NSInteger baselevel, NSUInteger *glyphs,
                             unichar *text, uint8_t *levels, NSInteger mirror,
                             NSUInteger length);

// Return YES if the BIDI tables are available
BOOL NSBidiHelperBidiInfoAvailable();
#ifdef __cplusplus
}
#endif
