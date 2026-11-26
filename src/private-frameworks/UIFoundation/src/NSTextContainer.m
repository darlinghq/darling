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

#import <AppKit/NSLayoutManager.h>
#import <Foundation/NSRaise.h>
#import <AppKit/NSTextContainer.h>
#import <AppKit/NSTextStorage.h>
#import <AppKit/NSTextView.h>
#import <Foundation/NSKeyedArchiver.h>

static const CGFloat NSTextContainerDefaultLineFragmentPadding = 5.0;

// AppKit's NSViewFrameDidChangeNotification. UIFoundation does not link
// against AppKit; notification names are matched by value, so a string with
// the same value observes the same notification.
static NSString *const _NSTextContainerViewFrameDidChangeNotification =
        @"NSViewFrameDidChangeNotification";

@interface NSTextView (NSTextView_textContainerTracking)
- (NSSize) _textContainerSizeForViewSize: (NSSize) size;
@end

@implementation NSTextContainer

- initWithCoder: (NSCoder *) coder {
    if ([coder allowsKeyedCoding]) {
        NSKeyedUnarchiver *keyed = (NSKeyedUnarchiver *) coder;

        _size.width = [keyed decodeFloatForKey: @"NSWidth"];
        _size.height = 1e7;
        _textView = [keyed decodeObjectForKey: @"NSTextView"];
        _layoutManager = [keyed decodeObjectForKey: @"NSLayoutManager"];
        _lineFragmentPadding = NSTextContainerDefaultLineFragmentPadding;

        int flags = [coder decodeIntForKey: @"NSTCFlags"];
        NS_EXTRA(NSTextContainer, self)->_widthTracksTextView = (flags & 1) != 0;
        NS_EXTRA(NSTextContainer, self)->_heightTracksTextView = (flags & 2) != 0;
        // TODO: maximumNumberOfLines
    } else {
        [NSException raise: NSInvalidArgumentException
                    format: @"-[%@ %s] is not implemented for coder %@",
                            [self class], sel_getName(_cmd), coder];
    }
    return self;
}

// initWithSize: is the documented designated initializer (macOS 10.11); the older
// initWithContainerSize: and -init go through it.
- initWithSize: (NSSize) size {
    _size = size;
    _textView = nil;
    _layoutManager = nil;
    _lineFragmentPadding = NSTextContainerDefaultLineFragmentPadding;
    NS_EXTRA(NSTextContainer, self)->_maximumNumberOfLines = 0;
    NS_EXTRA(NSTextContainer, self)->_widthTracksTextView = NO;
    NS_EXTRA(NSTextContainer, self)->_heightTracksTextView = NO;
    return self;
}

- initWithContainerSize: (NSSize) size {
    return [self initWithSize: size];
}

// The container of -init is practically unlimited, 10000000 points either way, and tracks neither the width
// nor the height of its text view (macOS 26.7.3 and 10.14.4, black-box, tier 2).
// Test: tests/text-container-tracking-defaults.m, appkit-textview-default-container.m in darling-cocotron
- init {
    return [self initWithSize: NSMakeSize(10000000, 10000000)];
}

- (void) dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver: self];
    NS_EXTRA_FREE(NSTextContainer, self);
    [super dealloc];
}
- (NSSize) containerSize {
    return _size;
}

// size and setSize: are the documented spellings of containerSize and setContainerSize:. On macOS 26.7.3 both
// names read and write the same value, and the layout manager is told once per change whichever name sets it
// (black-box, tier 2; test: tests/nstextcontainer-size.m). The setter goes through the older one because the
// framework's own resize paths (text view tracking, NSTextView, NSStringDrawer, NSViewTemplate) call
// setContainerSize:; a subclass that overrides either name then sees one call from -setSize:, but a subclass
// that overrides only setSize: is not reached by those internal resizes.
- (NSSize) size {
    return [self containerSize];
}

- (void) setSize: (NSSize) size {
    [self setContainerSize: size];
}

- (NSTextView *) textView {
    return _textView;
}

- (NSTextLayoutOrientation) layoutOrientation {
    return _textView ? [_textView layoutOrientation]
                     : NSTextLayoutOrientationHorizontal;
}

- (BOOL) widthTracksTextView {
    return NS_EXTRA(NSTextContainer, self)->_widthTracksTextView;
}

- (BOOL) heightTracksTextView {
    return NS_EXTRA(NSTextContainer, self)->_heightTracksTextView;
}

- (NSLayoutManager *) layoutManager {
    return _layoutManager;
}

// The layout manager is unretained: it calls this when it goes away or drops the container, so no dangling pointer is left.
- (void) _darlingDetachFromLayoutManager: (NSLayoutManager *) layoutManager {
    if (_layoutManager == layoutManager)
        _layoutManager = nil;
}

- (CGFloat) lineFragmentPadding {
    return _lineFragmentPadding;
}

- (NSUInteger) maximumNumberOfLines {
    return NS_EXTRA(NSTextContainer, self)->_maximumNumberOfLines;
}

- (void) setContainerSize: (NSSize) size {
    if (!NSEqualSizes(_size, size)) {
        _size = size;
        [_layoutManager textContainerChangedGeometry: self];
    }
}

- (void) setTextView: (NSTextView *) textView {
    if (NS_EXTRA(NSTextContainer, self)->_heightTracksTextView || NS_EXTRA(NSTextContainer, self)->_widthTracksTextView) {
        if (_textView) {
            [[NSNotificationCenter defaultCenter]
                    removeObserver: self
                              name: _NSTextContainerViewFrameDidChangeNotification
                            object: _textView];
        }
    }
    _textView = textView;
    [_textView setTextContainer: self];
    if (NS_EXTRA(NSTextContainer, self)->_heightTracksTextView || NS_EXTRA(NSTextContainer, self)->_widthTracksTextView) {
        if (_textView) {
            [_textView setPostsFrameChangedNotifications: YES];
            [[NSNotificationCenter defaultCenter]
                    addObserver: self
                       selector: @selector(_textViewFrameDidChange:)
                           name: _NSTextContainerViewFrameDidChangeNotification
                         object: _textView];
        }
    }
}

- (void) _textViewFrameDidChange: (NSNotification *) notification {
    if ([notification object] == _textView) {
        [self _resizeToTextView];
    }
}

- (void) _resizeToTextView {
    NSSize newSize = _size;
    NSSize tracked =
            [_textView _textContainerSizeForViewSize: [_textView frame].size];
    if (NS_EXTRA(NSTextContainer, self)->_widthTracksTextView) {
        newSize.width = tracked.width;
    }
    if (NS_EXTRA(NSTextContainer, self)->_heightTracksTextView) {
        newSize.height = tracked.height;
    }
    [self setContainerSize: newSize];
}

- (void) setWidthTracksTextView: (BOOL) flag {
    if (flag != NS_EXTRA(NSTextContainer, self)->_widthTracksTextView) {
        NS_EXTRA(NSTextContainer, self)->_widthTracksTextView = flag;
        if (_textView) {
            if (NS_EXTRA(NSTextContainer, self)->_widthTracksTextView) {
                if (NS_EXTRA(NSTextContainer, self)->_heightTracksTextView == NO) {
                    // Observe our textView frame changes
                    [_textView setPostsFrameChangedNotifications: YES];
                    [[NSNotificationCenter defaultCenter]
                            addObserver: self
                               selector: @selector(_textViewFrameDidChange:)
                                   name: _NSTextContainerViewFrameDidChangeNotification
                                 object: _textView];
                }
            } else {
                if (NS_EXTRA(NSTextContainer, self)->_heightTracksTextView == NO) {
                    [[NSNotificationCenter defaultCenter]
                            removeObserver: self
                                      name: _NSTextContainerViewFrameDidChangeNotification
                                    object: _textView];
                }
            }
        }
    }
}

- (void) setHeightTracksTextView: (BOOL) flag {
    if (flag != NS_EXTRA(NSTextContainer, self)->_heightTracksTextView) {
        NS_EXTRA(NSTextContainer, self)->_heightTracksTextView = flag;
        if (_textView) {
            if (NS_EXTRA(NSTextContainer, self)->_heightTracksTextView) {
                if (NS_EXTRA(NSTextContainer, self)->_widthTracksTextView == NO) {
                    // Observe our textView frame changes
                    [_textView setPostsFrameChangedNotifications: YES];
                    [[NSNotificationCenter defaultCenter]
                            addObserver: self
                               selector: @selector(_textViewFrameDidChange:)
                                   name: _NSTextContainerViewFrameDidChangeNotification
                                 object: _textView];
                }
            } else {
                if (NS_EXTRA(NSTextContainer, self)->_widthTracksTextView == NO) {
                    [[NSNotificationCenter defaultCenter]
                            removeObserver: self
                                      name: _NSTextContainerViewFrameDidChangeNotification
                                    object: _textView];
                }
            }
        }
    }
}

- (void) setLayoutManager: (NSLayoutManager *) layoutManager {
    _layoutManager = layoutManager;
    [_textView _setTextStorage: [_layoutManager textStorage]];
}

- (void) replaceLayoutManager: (NSLayoutManager *) newLayoutManager {
    if (newLayoutManager != _layoutManager) {
        NSLayoutManager *oldLayoutManager = _layoutManager;

        NSTextStorage *textStorage = [[oldLayoutManager textStorage] retain];
        if (textStorage == nil && newLayoutManager != nil) {
            textStorage = [[newLayoutManager textStorage] retain];
        }

        if (oldLayoutManager != nil) {
            NSArray *textContainers = [oldLayoutManager textContainers];
            NSUInteger idx = [textContainers indexOfObjectIdenticalTo: self];
            if (idx != NSNotFound) {
                [oldLayoutManager removeTextContainerAtIndex: idx];
            }
        }

        _layoutManager = newLayoutManager;

        if (_layoutManager != nil) {
            if ([[_layoutManager textContainers] indexOfObjectIdenticalTo: self] == NSNotFound) {
                [_layoutManager addTextContainer: self];
            }
            if (textStorage != nil) {
                if (oldLayoutManager != nil) {
                    [textStorage removeLayoutManager: oldLayoutManager];
                }
                [textStorage addLayoutManager: _layoutManager];
                [_textView _setTextStorage: textStorage];
            }
        }

        [textStorage release];
    }
}

- (void) setLineFragmentPadding: (CGFloat) padding {
    if (_lineFragmentPadding != padding) {
        _lineFragmentPadding = padding;
        [_layoutManager textContainerChangedGeometry: self];
    }
}

- (void) setMaximumNumberOfLines: (NSUInteger) maximumNumberOfLines {
    NS_EXTRA(NSTextContainer, self)->_maximumNumberOfLines = maximumNumberOfLines;
}

- (BOOL) isSimpleRectangularTextContainer {
    return YES;
}

- (BOOL) containsPoint: (NSPoint) point {
    return NSPointInRect(point, NSMakeRect(0, 0, _size.width, _size.height));
}

// The padding is applied by the typesetter, not here.
- (NSRect) lineFragmentRectForProposedRect: (NSRect) proposed
                            sweepDirection: (NSLineSweepDirection) sweep
                         movementDirection: (NSLineMovementDirection) movement
                             remainingRect: (NSRectPointer) remaining
{
    NSRect r = {.origin = NSZeroPoint, .size = _size};
    NSRect result = NSIntersectionRect(r, proposed);

    if (sweep != NSLineSweepRight || movement != NSLineMovesDown)
        NSUnimplementedMethod();

    if (result.origin.x + result.size.width > _size.width)
        result.size.width = _size.width - result.origin.x;

    if (result.size.width <= 0)
        result = NSZeroRect;

    if (result.size.height <= 0)
        result = NSZeroRect;

    *remaining = NSZeroRect;

    return result;
}

@end
