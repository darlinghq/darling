/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import "UIFoundation-ExtraIvars.h"
#import "NSTypesetter_concrete.h"
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSColor.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSLayoutManager.h>
#import <AppKit/NSParagraphStyle.h>
#import <Foundation/NSRaise.h>
#import "NSRangeArray.h"
#import <AppKit/NSTextAttachment.h>
#import <AppKit/NSTextContainer.h>
#import <AppKit/NSTextStorage.h>
#import <AppKit/NSTextTab.h>

#import "NSBidiHelper.h"

@interface NSLayoutManager (private)
- (void) _rollbackLatestFragment;
@end

@interface NSTypesetter_concrete (forward)
- (void) _updateBidiLevelsForRange: (NSRange) range;
@end

// Text with no font attribute uses the fallback font AppKit provides. Without
// AppKit loaded there is none, and its glyphs take no space.
static NSPoint _positionOfGlyphWithoutFont(NSFont *font, SEL selector,
                                           NSGlyph glyph, NSGlyph previous,
                                           BOOL *isNominal)
{
    if (isNominal != NULL)
        *isNominal = YES;
    return NSZeroPoint;
}

@implementation NSTypesetter_concrete

- init {
    [super init];
    _layoutNextFragment =
            [self methodForSelector: @selector(layoutNextFragment)];
    _glyphCacheRange = NSMakeRange(0, 0);
    _glyphCacheCapacity = 256;
    _glyphCache =
            NSZoneMalloc([self zone], sizeof(NSGlyph) * _glyphCacheCapacity);
    _characterCache =
            NSZoneMalloc([self zone], sizeof(unichar) * _glyphCacheCapacity);

    _glyphRangesInLine = [NSRangeArray new];
    return self;
}

- (void) dealloc {
    NSZoneFree([self zone], _glyphCache);
    NSZoneFree([self zone], _characterCache);
    NSZoneFree([self zone], _bidiLevels);
    [_container release];

    [_glyphRangesInLine release];
    [super dealloc];
}

static void loadGlyphAndCharacterCacheForLocation(NSTypesetter_concrete *self,
                                                  NSUInteger location)
{
    NSUInteger length = MIN(self->_glyphCacheCapacity,
                            NSMaxRange(self->_attributesGlyphRange) - location);

    self->_glyphCacheRange = NSMakeRange(location, length);

    [UIF_EXTRA_NSTypesetter(self)->_string getCharacters: self->_characterCache
                           range: self->_glyphCacheRange];
    [UIF_EXTRA_NSTypesetter(self)->_layoutManager getGlyphs: self->_glyphCache
                              range: self->_glyphCacheRange];
}

#define DEBUG_TYPESETTER 0

/**
 * Calculates line fragment rectangle, line fragment used rectangle, and
 * remaining rectangle for a line fragment.
 */
- (void) getLineFragmentRect: (NSRectPointer) lineFragmentRect
                       usedRect: (NSRectPointer) lineFragmentUsedRect
                  remainingRect: (NSRectPointer) remainingRect
        forStartingGlyphAtIndex: (NSUInteger) startingGlyphIndex
                   proposedRect: (NSRect) proposedRect
                    lineSpacing: (CGFloat) lineSpacing
         paragraphSpacingBefore: (CGFloat) paragraphSpacingBefore
          paragraphSpacingAfter: (CGFloat) paragraphSpacingAfter
{
#if DEBUG_TYPESETTER
    NSLog(@"getLineFragmentRect: %p usedRect: %p remainingRect: %p "
          @"forStartingGlyphAtIndex: %u proposedRect: %@ lineSpacing: %f "
          @"paragraphSpacingBefore: %f paragraphSpacingAfter: %f",
          lineFragmentRect, lineFragmentUsedRect, remainingRect,
          startingGlyphIndex, NSStringFromRect(proposedRect), lineSpacing,
          paragraphSpacingBefore, paragraphSpacingAfter);

#define DEBUG_GETLINEFRAGMENTRECT 0
#endif

    _scanRect = proposedRect;

    CGFloat fragmentHeight = _fontDefaultLineHeight;

    CGFloat wantedHeight = MAX(proposedRect.size.height, fragmentHeight);
    _scanRect.size.height = wantedHeight;

    // 1. Check with the container that we have space available for a rect with
    // the wanted height

    _scanRect = [_container lineFragmentRectForProposedRect: _scanRect
                                             sweepDirection: NSLineSweepRight
                                          movementDirection: NSLineMovesDown
                                              remainingRect: remainingRect];

    if ([_glyphRangesInLine count] != 0 &&
        _scanRect.origin.y > proposedRect.origin.y) {
#if DEBUG_GETLINEFRAGMENTRECT
        NSLog(@"don't move the rects down");
#endif
        // We don't want the rects to move down, if it's not the first one of
        // the line We could use NSLineNoMove with
        // lineFragmentRectForProposedRect but Cocoa doesn't do that
        _scanRect = NSZeroRect;
    }
    if (_scanRect.size.height <= 0) {
        // Too small for our text
        _scanRect = NSZeroRect;
    }
    if (NSEqualRects(_scanRect, NSZeroRect) == NO) {
#if DEBUG_GETLINEFRAGMENTRECT
        NSLog(@"applying padding: %f", _container.lineFragmentPadding);
#endif
        // Pad the first fragment of a line on both sides; later fragments
        // of the line are already bounded by _fullLineRect.
        if ([_glyphRangesInLine count] == 0) {
            _scanRect.origin.x += _container.lineFragmentPadding;
            _scanRect.size.width -= 2 * _container.lineFragmentPadding;
            _fullLineRect = _scanRect;
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"_fullLineRect.size.width: %f", _fullLineRect.size.width);
#endif
        }

        // If adding the padding caused us to use up all the available width
        // then we're done.
        if (_scanRect.size.width <= 0.) {
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"which made _scanRect width too small");
#endif
            _scanRect = NSZeroRect;
        }
    }
    if (NSEqualRects(_scanRect, NSZeroRect)) {
        if ([_glyphRangesInLine count] == 0) {
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"no more room for another line - bailing");
#endif
            // No more room for another line
            return;
        } else {
            // No more room on that line - we will try next one - just reset the
            // scanRect location to the end of the line so the advanceScanRect
            // logic will continue from the right place
            _scanRect.origin.x = NSMaxX(_fullLineRect);
            _scanRect.origin.y = NSMinY(proposedRect);
            _scanRect.size.width = 0;
            _scanRect.size.height =
                    MAX(proposedRect.size.height, fragmentHeight);
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"No more room on that line - we will try next one. "
                  @"_scanRect now: %@",
                  NSStringFromRect(_scanRect));
#endif
        }
    }

    NSUInteger glyphIndex = 0;
    NSRange fragmentRange = NSMakeRange(_nextGlyphLocation, 0);
    CGFloat fragmentWidth = 0;

    NSRange wordWrapRange = NSMakeRange(_nextGlyphLocation, 0);
    CGFloat wordWrapWidth = 0;
    NSGlyph wordWrapPreviousGlyph = NSNullGlyph;
    NSRect fragmentRect = NSZeroRect;
    BOOL isNominal = YES, advanceScanRect = NO, endOfString = NO,
         endOfLine = NO;

    // 2. So we have our rect - now find out how many glyphs can fit - use the
    // fragmentRange and fragmentWidth to keep track

    for (; (glyphIndex = NSMaxRange(fragmentRange)) <
           NSMaxRange(_attributesGlyphRange);) {

#if DEBUG_GETLINEFRAGMENTRECT
        NSLog(@"Examining glyph index: %u", glyphIndex);
#endif

        NSGlyph glyph = NSNullGlyph;
        unichar character = 0;
        CGFloat glyphAdvance = 0, glyphMaxWidth = 0;
        BOOL fragmentExit = NO;

        _paragraphBreak = NO;
        fragmentRange.length++;
        _lineRange.length++;

#if DEBUG_GETLINEFRAGMENTRECT
        NSLog(@"   fragmentRange: %@", NSStringFromRange(fragmentRange));
        NSLog(@"   _lineRange: %@", NSStringFromRange(_lineRange));
#endif

        if (!NSLocationInRange(glyphIndex, _glyphCacheRange)) {

#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"loading glyph and character cache");
#endif

            loadGlyphAndCharacterCacheForLocation(self, glyphIndex);
        }

        glyph = _glyphCache[glyphIndex - _glyphCacheRange.location];
        character = _characterCache[glyphIndex - _glyphCacheRange.location];
        // TODO: there must be other chars to check for word wrapping
        if (character == ' ') {

#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"found a space char");
#endif

            // We can word wrap from here if needed
            wordWrapRange = fragmentRange;
            wordWrapWidth = fragmentWidth;
            wordWrapPreviousGlyph = _previousGlyph;
            // Keep the info so we can rollback to that point even if we switch
            // to another fragment
            _wordWrapWidth = wordWrapWidth;
            _wordWrapRange = wordWrapRange;
            _wordWrapPreviousGlyph = wordWrapPreviousGlyph;
            _wordWrapScanRect = _scanRect;
        }

        if (character == NSAttachmentCharacter) {

#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"found an attachment char");
#endif

            NSTextAttachment *attachment =
                    [_attributes objectForKey: NSAttachmentAttributeName];
            id<NSTextAttachmentCell> cell = [attachment attachmentCell];
            NSSize size = [cell cellSize];

            fragmentHeight = size.height;
            /* An attachment has no glyph, so it must reserve its own width. Taking the font's
             * null-glyph advance here left the run a few points wide while the attachment cell
             * painted its full width over whatever followed (measured: a 24pt attachment reserved
             * ~8pt and overprinted the next glyph). glyphMaxWidth already carried size.width for
             * the wrapping tests, but only glyphAdvance moves the pen. */
            glyphAdvance = size.width;
            glyphMaxWidth = size.width;
            _previousGlyph = NSNullGlyph;
        } else {
            if (glyph == NSControlGlyph) {
#if DEBUG_GETLINEFRAGMENTRECT
                NSLog(@"found a control glyph");
#endif
                fragmentWidth += _positionOfGlyph(_font, NULL, NSNullGlyph,
                                                  _previousGlyph, &isNominal)
                                         .x;
                _previousGlyph = NSNullGlyph;

                switch ([self actionForControlCharacterAtIndex: glyphIndex]) {

                case NSTypesetterZeroAdvancementAction:
#if DEBUG_GETLINEFRAGMENTRECT
                    NSLog(@"action: NSTypesetterZeroAdvancementAction");
#endif
                    // do nothing
                    break;

                case NSTypesetterWhitespaceAction:
#if DEBUG_GETLINEFRAGMENTRECT
                    NSLog(@"action: NSTypesetterWhitespaceAction");
#endif
                    fragmentWidth += _whitespaceAdvancement;
                    break;

                case NSTypesetterHorizontalTabAction: {
#if DEBUG_GETLINEFRAGMENTRECT
                    NSLog(@"action: NSTypesetterHorizontalTabAction");
#endif
                    CGFloat x = _scanRect.origin.x + fragmentWidth;
                    NSTextTab *tab =
                            [self textTabForGlyphLocation: x
                                         writingDirection:
                                                 [UIF_EXTRA_NSTypesetter(self)->_currentParagraphStyle
                                                         baseWritingDirection]
                                              maxLocation: NSMaxX(_scanRect)];
                    CGFloat nextx;

                    if (tab != nil)
                        nextx = [tab location];
                    else {
                        CGFloat interval =
                                [UIF_EXTRA_NSTypesetter(self)->_currentParagraphStyle defaultTabInterval];

                        if (interval > 0)
                            nextx = (((NSInteger)(x / interval)) + 1) *
                                    interval;
                        else
                            nextx = x;
                    }

                    fragmentWidth += nextx - x;
                } break;

                case NSTypesetterParagraphBreakAction:
#if DEBUG_GETLINEFRAGMENTRECT
                    NSLog(@"action: NSTypesetterParagraphBreakAction");
#endif
                    _paragraphBreak = YES;
                    advanceScanRect = YES;
                    break;
                case NSTypesetterLineBreakAction:
#if DEBUG_GETLINEFRAGMENTRECT
                    NSLog(@"action: NSTypesetterLineBreakAction");
#endif
                    advanceScanRect = YES;
                    break;
                case NSTypesetterContainerBreakAction:
#if DEBUG_GETLINEFRAGMENTRECT
                    NSLog(@"action: NSTypesetterContainerBreakAction");
#endif
                    advanceScanRect = YES;
                    break;
                }

                break;
            }

            glyphAdvance = _positionOfGlyph(_font, NULL, glyph, _previousGlyph,
                                            &isNominal)
                                   .x;
            if (!isNominal && fragmentRange.length > 1) {
#if DEBUG_GETLINEFRAGMENTRECT
                NSLog(@"found special kerning pair");
#endif
                _lineRange.length--;
                fragmentRange.length--;
                fragmentWidth += glyphAdvance;
                _previousGlyph = NSNullGlyph;
                fragmentExit = YES;
                break;
            }
            // Get the width of the glyph
            glyphMaxWidth = _positionOfGlyph(_font, NULL, NSNullGlyph, glyph,
                                             &isNominal)
                                    .x;
        }

        switch (_lineBreakMode) {

        case NSLineBreakByWordWrapping:
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"_lineBreakMode: NSLineBreakByWordWrapping");
#endif
            if (_lineRange.length > 1) {
                if (fragmentWidth + glyphAdvance + glyphMaxWidth >
                    _scanRect.size.width) {
#if DEBUG_GETLINEFRAGMENTRECT
                    NSLog(@"cumulative width is too large: %f > %f",
                          fragmentWidth + glyphAdvance + glyphMaxWidth,
                          _scanRect.size.width);
#endif
                    if (wordWrapWidth > 0) {
                        // Break the line at previously found wrap location
                        _lineRange.length =
                                NSMaxRange(wordWrapRange) - _lineRange.location;
                        fragmentRange = wordWrapRange;
                        fragmentWidth = wordWrapWidth;
                        _previousGlyph = wordWrapPreviousGlyph;
                        wordWrapWidth = _wordWrapWidth = 0;
                    } else {
                        if ([_glyphRangesInLine count] != 0) {
                            // We need to rollback all of the previous fragments
                            // until one ending with a white space
                            //		 because words can span on several
                            // fragment because of some attribute changes
                            if (_wordWrapWidth > 0) {
                                _lineRange.length = NSMaxRange(_wordWrapRange) -
                                                    _lineRange.location;
                                // Rollback all fragments up to the previous
                                // wrapable one
                                do {
                                    NSRange range = [_glyphRangesInLine
                                            rangeAtIndex: [_glyphRangesInLine
                                                                  count] -
                                                          1];
                                    [UIF_EXTRA_NSTypesetter(self)->_layoutManager _rollbackLatestFragment];
                                    [_glyphRangesInLine
                                            removeRangeAtIndex:
                                                    [_glyphRangesInLine count] -
                                                    1];
                                    if (_wordWrapRange.location ==
                                        range.location) {
                                        break;
                                    }
                                } while ([_glyphRangesInLine count]);
                                _scanRect = _wordWrapScanRect;
                                fragmentRange = _wordWrapRange;
                                fragmentWidth = _wordWrapWidth;
                                _previousGlyph = _wordWrapPreviousGlyph;
                                wordWrapWidth = _wordWrapWidth = 0;
                            } else {
                                // We'll put the whole fragment on next line
                                // No more room on that line - we will try next
                                // one - just reset the scanRect location to the
                                // end of the line so the advanceScanRect logic
                                // will continue from the right place
                                _scanRect.origin.x = NSMaxX(_fullLineRect);
                                _scanRect.origin.y = NSMinY(proposedRect);
                                _scanRect.size.width = 0;
                                _scanRect.size.height =
                                        MAX(proposedRect.size.height,
                                            fragmentHeight);
                                _lineRange.length -= fragmentRange.length;
                                fragmentRange.length = 0;
                                fragmentWidth = 0;
                            }
                        } else {
                            // No wrapping location candidate - we'll just break
                            // the line at current glyph
                            _lineRange.length--;
                            fragmentRange.length--;
                        }
                    }
                    fragmentExit = YES;
                    advanceScanRect = YES;
                }
            }
            break;

        case NSLineBreakByCharWrapping:
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"_lineBreakMode: NSLineBreakByCharWrapping");
#endif
            if (_lineRange.length > 1) {
                if (fragmentWidth + glyphAdvance + glyphMaxWidth >
                    _scanRect.size.width) {
                    // Break the line at current glyph
                    _lineRange.length--;
                    fragmentRange.length--;
                    fragmentExit = YES;
                    advanceScanRect = YES;
                }
            }
            break;

        case NSLineBreakByClipping:
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"_lineBreakMode: NSLineBreakByClipping");
#endif
            // Nothing special to do
            break;

        case NSLineBreakByTruncatingHead:
        case NSLineBreakByTruncatingTail:
        case NSLineBreakByTruncatingMiddle:
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"_lineBreakMode: %d", _lineBreakMode);
#endif
            // TODO: implement these styles
            break;
        default:
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"_lineBreakMode: %d", _lineBreakMode);
#endif
            break;
        }

        if (fragmentExit == NO && _alignment == NSJustifiedTextAlignment) {
            if (character == ' ') {
#if DEBUG_GETLINEFRAGMENTRECT
                NSLog(@"making a new fragment after a space because of "
                      @"justified alignment");
#endif
                // Make a new fragment after a ' ' so we can insert some white
                // spaces to justify the line
                _previousGlyph = glyph;
                fragmentWidth += glyphAdvance;
                break;
            }
        }
        if (fragmentExit) {
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"exiting fragment");
#endif
            _previousGlyph = NSNullGlyph;
            fragmentWidth += glyphAdvance;
            break;
        }
        _previousGlyph = glyph;
        fragmentWidth += glyphAdvance;
    }

#if DEBUG_GETLINEFRAGMENTRECT
    NSLog(@"---- looking at the fragment -----");
    NSLog(@"     fragmentRange: %@", NSStringFromRange(fragmentRange));
    NSLog(@"     fragmentWidth: %f", fragmentWidth);
    NSLog(@"     _lineRange: %@", NSStringFromRange(_lineRange));
#endif

    if (fragmentRange.length > 0) {
#if DEBUG_GETLINEFRAGMENTRECT
        NSLog(@"have a fragment...");
#endif
        _nextGlyphLocation = NSMaxRange(fragmentRange);

        if (_nextGlyphLocation >= _numberOfGlyphs) {
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"and at the end of the string...");
#endif
            endOfString = YES;
        } else {
            if (!NSLocationInRange(_nextGlyphLocation, _glyphCacheRange))
                loadGlyphAndCharacterCacheForLocation(self, _nextGlyphLocation);

            NSInteger nextChar =
                    _characterCache[glyphIndex - _glyphCacheRange.location];
            if ([[NSCharacterSet newlineCharacterSet]
                        characterIsMember: nextChar]) {
#if DEBUG_GETLINEFRAGMENTRECT
                NSLog(@"and at the end of the line...");
#endif
                endOfLine = YES;
            }
        }

        if (!advanceScanRect) {
            CGFloat glyphAdvance = _positionOfGlyph(_font, NULL, NSNullGlyph,
                                                    _previousGlyph, &isNominal)
                                           .x;

            fragmentWidth += glyphAdvance;

#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"   added on glyphAdvance: %f", glyphAdvance);
            NSLog(@"   fragmentWidth now: %f", fragmentWidth);
#endif

            // This should be done only when switching font or something?
            if (glyphIndex == NSMaxRange(_attributesGlyphRange)) {
                _previousGlyph = NSNullGlyph;
            } else {
                // Break because of full justification
            }
        }
        CGFloat height = MAX(_scanRect.size.height, fragmentHeight);
        if (_scanRect.size.height != height) {
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"_scanRect height changed");
#endif
            // Check we still fit if something changed our height
            _scanRect.size.height = height;
            _scanRect = [_container
                    lineFragmentRectForProposedRect: _scanRect
                                     sweepDirection: NSLineSweepRight
                                  movementDirection: NSLineMovesDown
                                      remainingRect: remainingRect];
        }
        if (!NSEqualRects(_scanRect, NSZeroRect)) {
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"setting the fragment");
#endif
            [_glyphRangesInLine addRange: fragmentRange];
            _maxAscender = MAX(_maxAscender, _fontAscender);
            [UIF_EXTRA_NSTypesetter(self)->_layoutManager setTextContainer: _container
                               forGlyphRange: fragmentRange];
            fragmentRect = _scanRect;
            fragmentRect.size.width = fragmentWidth;
            [UIF_EXTRA_NSTypesetter(self)->_layoutManager setLineFragmentRect: _scanRect
                                  forGlyphRange: fragmentRange
                                       usedRect: fragmentRect];
            [UIF_EXTRA_NSTypesetter(self)->_layoutManager setLocation: _scanRect.origin
                    forStartOfGlyphRange: fragmentRange];
        } else {
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"no more room for text - bailing");
#endif
            // Can't fit any more text
            return;
        }
    }

    if (advanceScanRect || endOfString) {
#if DEBUG_GETLINEFRAGMENTRECT
        NSLog(@"done with a line");
#endif
        // We're done with a line - fix the fragment rects for its
        NSInteger i, count = [_glyphRangesInLine count];
        CGFloat alignmentDelta = 0;

        // First, we need to reorder the fragments according to the bidi algo
        if (_bidiLevels && count > 1) {
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"reordering fragments for bidi");
#endif
            uint8_t levels[count];
            NSUInteger indexes[count];
            for (NSInteger i = 0; i < count; ++i) {
                NSRange range = [_glyphRangesInLine rangeAtIndex: i];
                levels[i] = _bidiLevels[range.location];
                indexes[i] = i;
            }
            NSInteger baseLevel = _currentParagraphBidiLevel;
            NSBidiHelperProcessLine(baseLevel, indexes, NULL, levels, NO,
                                    count);

            NSRange range = [_glyphRangesInLine rangeAtIndex: 0];
            NSPoint currentLocation =
                    [UIF_EXTRA_NSTypesetter(self)->_layoutManager locationForGlyphAtIndex: range.location];
            for (NSInteger i = 0; i < count; ++i) {
                NSInteger idx = indexes[i];
                NSRange range = [_glyphRangesInLine rangeAtIndex: idx];
#if DEBUG_GETLINEFRAGMENTRECT
                NSLog(@"reordering fragment range: %@",
                      NSStringFromRange(range));
#endif
                NSPoint location = [UIF_EXTRA_NSTypesetter(self)->_layoutManager
                        locationForGlyphAtIndex: range.location];
                NSRect usedRect = [UIF_EXTRA_NSTypesetter(self)->_layoutManager
                        lineFragmentUsedRectForGlyphAtIndex: range.location
                                             effectiveRange: NULL];
                NSRect rect = [UIF_EXTRA_NSTypesetter(self)->_layoutManager
                        lineFragmentRectForGlyphAtIndex: range.location
                                         effectiveRange: NULL];
                usedRect.origin = currentLocation;
                rect.origin = currentLocation;
                [UIF_EXTRA_NSTypesetter(self)->_layoutManager setLineFragmentRect: rect
                                      forGlyphRange: range
                                           usedRect: usedRect];
                [UIF_EXTRA_NSTypesetter(self)->_layoutManager setLocation: currentLocation
                        forStartOfGlyphRange: range];
                currentLocation.x += usedRect.size.width;
            }
            // Reorder the ranges LtoR
            for (NSInteger i = 0; i < count; ++i) {
                NSInteger idx = indexes[i];
                [_glyphRangesInLine
                        addRange: [_glyphRangesInLine rangeAtIndex: idx]];
            }
            for (NSInteger i = 0; i < count; ++i) {
                [_glyphRangesInLine removeRangeAtIndex: 0];
            }
        }

        if (_alignment != NSLeftTextAlignment) {
#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"adjusting for non-left alignment");
#endif
            // Find the total line width so we can adjust the rect x origin
            // according to the alignment
            CGFloat totalWidth = NSMaxX(_fullLineRect);

            CGFloat totalUsedWidth = 0;
            if (count) {
                NSRange range = [_glyphRangesInLine rangeAtIndex: count - 1];
                NSRect usedRect = [UIF_EXTRA_NSTypesetter(self)->_layoutManager
                        lineFragmentUsedRectForGlyphAtIndex: range.location
                                             effectiveRange: NULL];
                totalUsedWidth = NSMaxX(usedRect);
            }

            // Calc the delta to apply to the origins so the alignment is
            // respected
            switch (_alignment) {
            case NSRightTextAlignment:
#if DEBUG_GETLINEFRAGMENTRECT
                NSLog(@"NSRightTextAlignment");
#endif
                alignmentDelta = totalWidth - totalUsedWidth;
                break;

            case NSCenterTextAlignment:
#if DEBUG_GETLINEFRAGMENTRECT
                NSLog(@"NSCenterTextAlignment");
#endif
                alignmentDelta = (totalWidth - totalUsedWidth) / 2;
                break;

            case NSJustifiedTextAlignment:
#if DEBUG_GETLINEFRAGMENTRECT
                NSLog(@"NSJustifiedTextAlignment");
#endif
                if (!endOfString && !endOfLine) {
                    NSInteger blankCount = 0;
                    // Find the number of fragments ending with a ' '
                    for (i = 1; i < count; i++) {
                        NSRange range = [_glyphRangesInLine rangeAtIndex: i];
                        if (!NSLocationInRange(range.location,
                                               _glyphCacheRange))
                            loadGlyphAndCharacterCacheForLocation(
                                    self, range.location - 1);

                        int character =
                                _characterCache[range.location - 1 -
                                                _glyphCacheRange.location];
                        if (character == ' ') {
                            blankCount++;
                        }
                    }
                    if (blankCount) {
                        alignmentDelta =
                                (totalWidth - totalUsedWidth) / blankCount;
                    }
                }
                break;

            default:
                break;
            }
        }

        NSInteger blankCount = 0;
        for (i = 0; i < count; i++) {

            NSRange range = [_glyphRangesInLine rangeAtIndex: i];

#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"adjusting glyph range: %@", NSStringFromRange(range));
#endif

            NSRect backRect = [UIF_EXTRA_NSTypesetter(self)->_layoutManager
                    lineFragmentRectForGlyphAtIndex: range.location
                                     effectiveRange: NULL];
            NSRect usedRect = [UIF_EXTRA_NSTypesetter(self)->_layoutManager
                    lineFragmentUsedRectForGlyphAtIndex: range.location
                                         effectiveRange: NULL];
            NSPoint location =
                    [UIF_EXTRA_NSTypesetter(self)->_layoutManager locationForGlyphAtIndex: range.location];

#if DEBUG_GETLINEFRAGMENTRECT
            NSLog(@"   backRect: %@", NSStringFromRect(backRect));
            NSLog(@"   usedRect: %@", NSStringFromRect(usedRect));
            NSLog(@"   location: %@", NSStringFromPoint(location));
#endif

            usedRect.size.height = backRect.size.height = _scanRect.size.height;
            location.y += _maxAscender;
            // The line fragment padding is part of the line: the used rect
            // covers it on both ends of the line, around the glyphs. The
            // fragment rect itself is widened below, once its final extent
            // is known.
            CGFloat padding = _container.lineFragmentPadding;
            if (i == 0) {
                usedRect.origin.x -= padding;
                usedRect.size.width += padding;
                backRect.origin.x -= padding;
                backRect.size.width += padding;
            }
            if (i + 1 == count) {
                usedRect.size.width += padding;
                backRect.size.width += padding;
            }
            if (_alignment == NSJustifiedTextAlignment) {
#if DEBUG_GETLINEFRAGMENTRECT
                NSLog(@"    NSJustifiedTextAlignment");
#endif
                if (i > 0) {
                    if (!NSLocationInRange(range.location, _glyphCacheRange))
                        loadGlyphAndCharacterCacheForLocation(
                                self, range.location - 1);

                    NSInteger character =
                            _characterCache[range.location - 1 -
                                            _glyphCacheRange.location];
                    if (character == ' ') {
                        // Add some more pixels from here
                        blankCount++;
                    }
#if DEBUG_GETLINEFRAGMENTRECT
                    NSLog(@"    adjusting usedRect origin by delta: %f",
                          alignmentDelta * blankCount);
#endif
                    usedRect.origin.x += alignmentDelta * blankCount;
                    location.x += alignmentDelta * blankCount;
                    if (i + 1 == count) {
                        // Adjust the last rect
#if DEBUG_GETLINEFRAGMENTRECT
                        NSLog(@"    adjusting last backRect by delta: %f",
                              alignmentDelta * blankCount);
#endif
                        backRect.origin.x += alignmentDelta * blankCount;
                        backRect.size.width -= alignmentDelta * blankCount;
                    }
                }
            } else {
                if (i == 0) {
                    // Grow the first rect to it contains the white space added
                    // for justification
#if DEBUG_GETLINEFRAGMENTRECT
                    NSLog(@"    growing first rect to contain white space: %f",
                          alignmentDelta);
#endif
                    backRect.size.width += alignmentDelta;
                }
                usedRect.origin.x += alignmentDelta;
                location.x += alignmentDelta;
                if (i + 1 == count) {
                    // Adjust the last rect
#if DEBUG_GETLINEFRAGMENTRECT
                    NSLog(@"    adjusting last rect by delta: %f",
                          alignmentDelta);
#endif
                    backRect.origin.x += alignmentDelta;
                    backRect.size.width -= alignmentDelta;
                }
            }
            if (i + 1 < count) {
#if DEBUG_GETLINEFRAGMENTRECT
                NSLog(@"setting fragment rect");
#endif
                [UIF_EXTRA_NSTypesetter(self)->_layoutManager setLineFragmentRect: usedRect
                                      forGlyphRange: range
                                           usedRect: usedRect];
            } else {
                // Last rect
#if DEBUG_GETLINEFRAGMENTRECT
                NSLog(@"setting last fragment rect");
#endif
                [UIF_EXTRA_NSTypesetter(self)->_layoutManager setLineFragmentRect: backRect
                                      forGlyphRange: range
                                           usedRect: usedRect];
            }
            [UIF_EXTRA_NSTypesetter(self)->_layoutManager setLocation: location forStartOfGlyphRange: range];
        }
        // Some cleaning after we're done with the current line
        [_glyphRangesInLine removeAllRanges];
        _wordWrapWidth = 0;
        _wordWrapRange = NSMakeRange(0, 0);
    }

    if (advanceScanRect) {
        _lineRange.location = NSMaxRange(fragmentRange);
        _lineRange.length = 0;
        _scanRect.origin.x = 0;
        _scanRect.origin.y += _scanRect.size.height;
        _scanRect.size.width = 1e7; // That's what Cocoa is sending
        _scanRect.size.height = 0;
        _maxAscender = 0;
#if DEBUG_GETLINEFRAGMENTRECT
        NSLog(@"advanced to next line, set _scanRect to: %@",
              NSStringFromRect(_scanRect));
#endif
    } else {
        _scanRect.origin.x += fragmentWidth;
        _scanRect.size.width = NSMaxX(_fullLineRect) - _scanRect.origin.x;
#if DEBUG_GETLINEFRAGMENTRECT
        NSLog(@"moved _scanRect to the end of the last fragment: %@",
              NSStringFromRect(_scanRect));
#endif
    }
}

- (void) layoutNextFragment {

#if DEBUG_TYPESETTER
    NSLog(@"layoutNextFragment");
#endif

    NSRect lineFragmentUsedRect;
    NSRect remainingRect;
    NSRect proposedRect = _scanRect;

    // FIXME: getLineFragmentRect: does actually more that it should do - part
    // of its code (like filing the typesetter layout info) should really be
    // moved to the layoutNextFragment mehod
    [self getLineFragmentRect: &_scanRect
                           usedRect: &lineFragmentUsedRect
                      remainingRect: &remainingRect
            forStartingGlyphAtIndex: _nextGlyphLocation
                       proposedRect: proposedRect
                        lineSpacing: 1
             paragraphSpacingBefore: 0
              paragraphSpacingAfter: 0];
}

- (void) fetchAttributes {

#if DEBUG_TYPESETTER
    NSLog(@"fetchAttributes");
#define DEBUG_FETCHATTRIBUTES 0
#endif

    NSFont *nextFont;
    NSUInteger characterIndex = _nextGlyphLocation; // FIX
    NSGlyph spaceGlyph;
    unichar space = ' ';

    _attributes = [UIF_EXTRA_NSTypesetter(self)->_attributedString attributesAtIndex: characterIndex
                                        effectiveRange: &_attributesRange];

#if DEBUG_FETCHATTRIBUTES
    NSLog(@"using attributes: %@ at characterIndex: %u effectiveRange: %@",
          _attributes, characterIndex, NSStringFromRange(_attributesRange));
#endif

    _currentBidiLevel = 0;

    if (_bidiLevels) {
        // Let's process the line - that's mostly resolving blanks and check if
        // we have a direction switch
        NSRange lineRange = [UIF_EXTRA_NSTypesetter(self)->_string lineRangeForRange: _lineRange];
        NSRange range;
        range.location = _lineRange.location;
        range.length = NSMaxRange(lineRange) - range.location;

        unichar text[range.length];
        uint8_t levels[range.length];
        for (NSInteger i = 0; i < range.length; ++i) {
            levels[i] = _bidiLevels[range.location + i];
        }
        [UIF_EXTRA_NSTypesetter(self)->_string getCharacters: text range: range];

        // Resolve the bidi level for the whitespace in this line [we should
        // save the current bidi level at the start of the line and use it]
        NSBidiHelperProcessLine(_currentParagraphBidiLevel, NULL, text, levels,
                                NO, range.length);

        // We'll create a new fragment everytime the bidi level switches
        // direction
        _currentBidiLevel = levels[characterIndex - range.location];

        NSInteger max = MIN(NSMaxRange(range), NSMaxRange(_attributesRange));

        for (NSInteger i = characterIndex + 1; i < max; ++i) {
            if ((levels[i - range.location] & 1) != (_currentBidiLevel & 1)) {
                _attributesRange.length = i - _attributesRange.location;
                break;
            }
        }
    }
    _attributesGlyphRange = _attributesRange; // FIX

    nextFont = NSFontAttributeInDictionary(_attributes);
    if (_font != nextFont) {
#if DEBUG_FETCHATTRIBUTES
        NSLog(@"switching font to %@", nextFont);
#endif
        _previousGlyph = NSNullGlyph;
        _font = nextFont;
        _fontAscender = ceilf([_font ascender]);
        _fontDefaultLineHeight = ceilf([_font defaultLineHeightForFont]);
        _positionOfGlyph = (void *) [_font
                methodForSelector: @selector(positionOfGlyph:
                                             precededByGlyph:isNominal:)];
        if (_positionOfGlyph == NULL)
            _positionOfGlyph = _positionOfGlyphWithoutFont;

        [_font getGlyphs: &spaceGlyph forCharacters: &space length: 1];
        _whitespaceAdvancement = [_font advancementForGlyph: spaceGlyph].width;
    }

    if ((UIF_EXTRA_NSTypesetter(self)->_currentParagraphStyle = [_attributes
                 objectForKey: NSParagraphStyleAttributeName]) == nil)
#if DEBUG_FETCHATTRIBUTES
        NSLog(@"using default paragraph style");
#endif

    UIF_EXTRA_NSTypesetter(self)->_currentParagraphStyle = [NSParagraphStyle defaultParagraphStyle];
    _alignment = [UIF_EXTRA_NSTypesetter(self)->_currentParagraphStyle alignment];
    _currentParagraphBidiLevel = 0;
    if (_bidiLevels) {
        NSRange paragraphRange = [UIF_EXTRA_NSTypesetter(self)->_string
                paragraphRangeForRange: NSMakeRange(characterIndex, 0)];
        _currentParagraphBidiLevel = _bidiLevels[paragraphRange.location];
    }
    if (_alignment == NSNaturalTextAlignment) {
        _alignment = NSLeftTextAlignment;
        if (_currentParagraphBidiLevel & 1) {
#if DEBUG_FETCHATTRIBUTES
            NSLog(@"switching to right text alignment for bidi");
#endif
            _alignment = NSRightTextAlignment;
        }
    }
    _lineBreakMode = [UIF_EXTRA_NSTypesetter(self)->_currentParagraphStyle lineBreakMode];
}

- (void) layoutGlyphsInLayoutManager: (NSLayoutManager *) layoutManager
                startingAtGlyphIndex: (NSUInteger) glyphIndex
            maxNumberOfLineFragments: (NSUInteger) maxNumLines
                      nextGlyphIndex: (NSUInteger *) nextGlyph
{

#if DEBUG_TYPESETTER
    NSLog(@"layoutGlyphsInLayoutManager: %@ startingAtGlyphIndex: %u "
          @"maxNumberOfLineFragments: %u nextGlyphIndex: %p",
          layoutManager, glyphIndex, maxNumLines, nextGlyph);
#define DEBUG_LAYOUTGLYPHSINLAYOUTMANAGER 0
#endif

    [layoutManager retain];
    [UIF_EXTRA_NSTypesetter(self)->_layoutManager release];
    UIF_EXTRA_NSTypesetter(self)->_layoutManager = layoutManager;
    UIF_EXTRA_NSTypesetter(self)->_textContainers = [layoutManager textContainers];
    if ([UIF_EXTRA_NSTypesetter(self)->_textContainers count] == 0)
        return;

    [self setAttributedString: [layoutManager textStorage]];

    NSUInteger length = [UIF_EXTRA_NSTypesetter(self)->_attributedString length];
    _currentBidiLevel = 0;
    _currentParagraphBidiLevel = 0;
    [self _updateBidiLevelsForRange: NSMakeRange(0, length)];

    _nextGlyphLocation = 0;
    _numberOfGlyphs = [UIF_EXTRA_NSTypesetter(self)->_string length];
    _glyphCacheRange = NSMakeRange(0, 0);
    _previousGlyph = NSNullGlyph;

    NSTextContainer *container = [[UIF_EXTRA_NSTypesetter(self)->_textContainers objectAtIndex: 0] retain];

    [_container release];
    _container = container;
    _containerSize = [_container containerSize];

    _attributesRange = NSMakeRange(0, 0);
    _attributesGlyphRange = NSMakeRange(0, 0);
    _attributes = nil;
    _font = nil;

    _lineRange = NSMakeRange(0, 0);
    [_glyphRangesInLine removeAllRanges];
    _previousGlyph = NSNullGlyph;
    _scanRect.origin.x = 0;
    _scanRect.origin.y = 0;
    _scanRect.size.width = 1e7; // That's what Cocoa is sending
    _scanRect.size.height = 0;
    _maxAscender = 0;
    _wordWrapWidth = 0;
    while (_nextGlyphLocation < _numberOfGlyphs) {

#if DEBUG_LAYOUTGLYPHSINLAYOUTMANAGER
        NSLog(@"checking glyph location: %d", _nextGlyphLocation);
#endif

        if (!NSLocationInRange(_nextGlyphLocation, _attributesRange))
            [self fetchAttributes];

        ((void(*)(id, SEL))_layoutNextFragment)(self, NULL);
        if (NSEqualRects(_scanRect, NSZeroRect)) {
#if DEBUGLAYOUTGLYPHSINLAYOUTMANAGER
            NSLog(@"_scanRect is zero - bailing...");
#endif
            break;
        }
    }

    if (_font == nil) {
        _font = NSFontAttributeInDictionary(nil);

#if DEBUGLAYOUTGLYPHSINLAYOUTMANAGER
        NSLog(@"_font is nil - defaulting to font: %@", _font);
#endif

        _fontAscender = ceilf([_font ascender]);
        _fontDefaultLineHeight = ceilf([_font defaultLineHeightForFont]);
        _positionOfGlyph = (void *) [_font
                methodForSelector: @selector(positionOfGlyph:
                                             precededByGlyph:isNominal:)];
        if (_positionOfGlyph == NULL)
            _positionOfGlyph = _positionOfGlyphWithoutFont;
    }

    if (((_paragraphBreak && _nextGlyphLocation >= _numberOfGlyphs) ||
         _numberOfGlyphs == 0)) {
        NSRect remainingRect; // Ignored for now
        _scanRect.size.height =
                MAX(_scanRect.size.height, _fontDefaultLineHeight);
        if (UIF_EXTRA_NSTypesetter(self)->_currentParagraphStyle) {
            if (UIF_EXTRA_NSTypesetter(self)->_currentParagraphStyle.maximumLineHeight != 0) {
                _scanRect.size.height =
                        MIN(_scanRect.size.height,
                            UIF_EXTRA_NSTypesetter(self)->_currentParagraphStyle.maximumLineHeight);
            }
            _scanRect.size.height =
                    MAX(_scanRect.size.height,
                        UIF_EXTRA_NSTypesetter(self)->_currentParagraphStyle.minimumLineHeight);
        }
        _scanRect =
                [_container lineFragmentRectForProposedRect: _scanRect
                                             sweepDirection: NSLineSweepRight
                                          movementDirection: NSLineMovesDown
                                              remainingRect: &remainingRect];
        NSRect usedRect = _scanRect;
        usedRect.origin.x += _container.lineFragmentPadding;
        usedRect.size.width = 10;
        [UIF_EXTRA_NSTypesetter(self)->_layoutManager setExtraLineFragmentRect: _scanRect
                                        usedRect: usedRect
                                   textContainer: _container];
    } else {
        [UIF_EXTRA_NSTypesetter(self)->_layoutManager setExtraLineFragmentRect: NSZeroRect
                                        usedRect: NSZeroRect
                                   textContainer: _container];
    }
    UIF_EXTRA_NSTypesetter(self)->_currentParagraphStyle = nil;
    _font = nil;
    _attributes = nil;

    [_container release];
    _container = nil;
    [self setAttributedString: nil];
    UIF_EXTRA_NSTypesetter(self)->_textContainers = nil;
    [UIF_EXTRA_NSTypesetter(self)->_layoutManager release];
    UIF_EXTRA_NSTypesetter(self)->_layoutManager = nil;
}

- (void) insertGlyphs: (const NSGlyph *) glyphs
                         length: (NSUInteger) length
        forStartingGlyphAtIndex: (NSUInteger) glyphIndex
                 characterIndex: (NSUInteger) characterIndex
{
    NSUnimplementedMethod();
}

- (void) setIntAttribute: (NSInteger) intAttribute
                   value: (NSInteger) value
         forGlyphAtIndex: (NSUInteger) glyphIndex;
{
    NSUnimplementedMethod();
}

- (NSUInteger) getGlyphsInRange: (NSRange) glyphRange
                         glyphs: (NSGlyph *) glyphs
               characterIndexes: (NSUInteger *) characterIndexes
              glyphInscriptions: (NSGlyphInscription *) glyphInscriptions
                    elasticBits: (BOOL *) elasticBits
                     bidiLevels: (unsigned char *) bidiLevels
{
#if DEBUG_TYPESETTER
    NSLog(@"getGlyphsInRange: %@ glyphs: %p characterIndexes: %p "
          @"glyphInscriptions: %p elasticBits: %p bidiLevels: %p",
          NSStringFromRange(glyphRange), glyphs, characterIndexes,
          glyphInscriptions, elasticBits, bidiLevels);
#define DEBUG_GETGLYPHSINRANGE 0
#endif

    NSUInteger result = 0;
    if (glyphs) {
        result = [UIF_EXTRA_NSTypesetter(self)->_layoutManager getGlyphs: glyphs range: glyphRange];
    }
    if (characterIndexes) {
        for (NSInteger i = 0; i < glyphRange.length; ++i) {
            characterIndexes[i] = glyphRange.location + i;
        }
    }
    if (glyphInscriptions) {
        // Not supported for now
        for (NSInteger i = 0; i < glyphRange.length; ++i) {
            glyphInscriptions[i] = 0;
        }
    }
    if (elasticBits) {
        // Not supported for now
        for (NSInteger i = 0; i < glyphRange.length; ++i) {
            elasticBits[i] = NO;
        }
    }
    if (bidiLevels) {
        for (NSInteger i = 0; i < glyphRange.length; ++i) {
            if (_bidiLevels) {
                bidiLevels[i] = _bidiLevels[glyphRange.location + i];
            } else {
                bidiLevels[i] = 0;
            }
        }
    }
    return result;
}

- (void) setBidiLevels: (const unsigned char *) bidiLevels
         forGlyphRange: (NSRange) glyphRange
{
#if DEBUG_TYPESETTER
    NSLog(@"setBidiLevels: %p forGlyphRange: %@", bidiLevels,
          NSStringFromRange(glyphRange));
#endif
    // Check if we're all left-to-right - if so, we don't have to use any bidi
    // level
    BOOL ltor = YES;
    if (_bidiLevels) {
        for (NSUInteger i = 0; ltor && i < _bidiLevelsCapacity; ++i) {
            if (!NSLocationInRange(i, glyphRange)) {
                ltor = _bidiLevels[i] == 0;
            }
        }
    }
    if (ltor) {
        for (NSUInteger i = 0; ltor && i < glyphRange.length; ++i) {
            ltor = bidiLevels[i] == 0;
        }
        if (ltor) {
            // Both the old data and the new one are Left-to-Right only - we
            // don't need any bidilevel
            if (_bidiLevels) {
                NSZoneFree([self zone], _bidiLevels);
                _bidiLevels = NULL;
            }
            return;
        }
    }
    // Both ltor & ltor
    if (_bidiLevels == NULL) {
        _bidiLevelsCapacity = MAX([UIF_EXTRA_NSTypesetter(self)->_attributedString length], 32);
        _bidiLevels = (uint8_t *) NSZoneMalloc(
                [self zone], _bidiLevelsCapacity * sizeof(uint8_t));
        // Anything new is left-to-right by default
        bzero(_bidiLevels, _bidiLevelsCapacity);
    }
    if (_bidiLevelsCapacity < NSMaxRange(glyphRange)) {
        NSInteger newBidiLevelsCapacity =
                MAX(_bidiLevelsCapacity * 2, NSMaxRange(glyphRange));
        _bidiLevels = (uint8_t *) NSZoneRealloc([self zone], _bidiLevels,
                                                newBidiLevelsCapacity *
                                                        sizeof(uint8_t));
        // Anything new is left-to-right by default
        bzero(_bidiLevels + _bidiLevelsCapacity,
              newBidiLevelsCapacity - _bidiLevelsCapacity);
        _bidiLevelsCapacity = newBidiLevelsCapacity;
    }
    memcpy(_bidiLevels + glyphRange.location, bidiLevels,
           glyphRange.length * sizeof(uint8_t));
}

- (void) _updateBidiLevelsForRange: (NSRange) range {
#if DEBUG_TYPESETTER
    NSLog(@"_updateBidiLevelsForRange: %@", NSStringFromRange(range));
#endif
    if (NSBidiHelperBidiInfoAvailable()) {
        NSString *string = [[self attributedString] string];
        range = [string paragraphRangeForRange: range];
        if (range.length == 0) {
            return;
        }
        uint8_t *bidiLevels = malloc(range.length * sizeof(uint8_t));
        unichar *buffer = malloc(range.length * sizeof(unichar));
        [string getCharacters: buffer range: range];

        NSInteger level = -1;
        NSBidiHelperParagraph(&level, buffer, bidiLevels, range.length);
        free(buffer);

        [self setBidiLevels: bidiLevels forGlyphRange: range];

        free(bidiLevels);
    }
}
@end
