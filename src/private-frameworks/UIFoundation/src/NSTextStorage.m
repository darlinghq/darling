/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

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

#import "NSTextStorage_concrete.h"
#import <AppKit/NSAttributedString.h>
#import <AppKit/NSLayoutManager.h>
#import <AppKit/NSTextStorage.h>
#import <Foundation/NSKeyedArchiver.h>

NSString *const NSTextStorageWillProcessEditingNotification =
        @"NSTextStorageWillProcessEditingNotification";
NSString *const NSTextStorageDidProcessEditingNotification =
        @"NSTextStorageDidProcessEditingNotification";

@implementation NSTextStorage

+ (BOOL) supportsSecureCoding {
    return YES;
}

+ allocWithZone: (NSZone *) zone {
    if (self == [NSTextStorage class])
        return NSAllocateObject([NSTextStorage_concrete class], 0, NULL);

    return NSAllocateObject(self, 0, zone);
}

- initWithCoder: (NSCoder *) coder {
    _layoutManagers = [NSMutableArray new];
    return self;
}

- (void) encodeWithCoder: (NSCoder *) coder {
}

- initWithString: (NSString *) string {
    _layoutManagers = [NSMutableArray new];
    return self;
}

static void updateDelegateObservation(NSTextStorage *self, id delegate, BOOL add);

- (void) dealloc {
    updateDelegateObservation(self, _delegate, NO);
    [_layoutManagers release];
    NS_EXTRA_FREE(NSTextStorage, self);
    [super dealloc];
}

- delegate {
    return _delegate;
}

- (NSArray *) layoutManagers {
    return _layoutManagers;
}

- (NSInteger) changeInLength {
    return _changeInLength;
}

- (NSTextStorageEditActions) editedMask {
    return _editedMask;
}

- (NSRange) editedRange {
    return NS_EXTRA(NSTextStorage, self)->_editedRange;
}

// The delegate observes the two notifications posted by -processEditing, for the methods it implements.
static void updateDelegateObservation(NSTextStorage *self, id delegate, BOOL add) {
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

    if (delegate == nil)
        return;
    if (!add) {
        [center removeObserver: delegate name: NSTextStorageWillProcessEditingNotification object: self];
        [center removeObserver: delegate name: NSTextStorageDidProcessEditingNotification object: self];
        return;
    }
    if ([delegate respondsToSelector: @selector(textStorageWillProcessEditing:)])
        [center addObserver: delegate
                   selector: @selector(textStorageWillProcessEditing:)
                       name: NSTextStorageWillProcessEditingNotification
                     object: self];
    if ([delegate respondsToSelector: @selector(textStorageDidProcessEditing:)])
        [center addObserver: delegate
                   selector: @selector(textStorageDidProcessEditing:)
                       name: NSTextStorageDidProcessEditingNotification
                     object: self];
}

- (void) setDelegate: delegate {
    updateDelegateObservation(self, _delegate, NO);
    _delegate = delegate;
    updateDelegateObservation(self, _delegate, YES);
}

- (void) addLayoutManager: (NSLayoutManager *) layoutManager {
    if (layoutManager == nil)
        return;
    // A subclass may have been initialized through -[NSAttributedString init],
    // which does not reach the initializers above.
    if (_layoutManagers == nil)
        _layoutManagers = [NSMutableArray new];
    if ([_layoutManagers indexOfObjectIdenticalTo: layoutManager] != NSNotFound)
        return;
    [_layoutManagers addObject: layoutManager];
    [layoutManager setTextStorage: self];
}

- (void) removeLayoutManager: (NSLayoutManager *) layoutManager {
    [_layoutManagers removeObjectIdenticalTo: layoutManager];
}

- (void) beginEditing {

    if (_beginEditing == 0) {
        _editedMask = 0;
        NS_EXTRA(NSTextStorage, self)->_editedRange = NSMakeRange(-1, -1);
        _changeInLength = 0;
    }

    _beginEditing++;
}

- (void) endEditing {
    _beginEditing--;
    if (_beginEditing == 0) {
        // Prevent any change to trigger more notification
        _beginEditing++;
        [self processEditing];
        _beginEditing--;
    }
}

- (NSRange) invalidatedRange {
    return NS_EXTRA(NSTextStorage, self)->_editedRange;
}

- (void) processEditing {
    NSUInteger i, count;

    [[NSNotificationCenter defaultCenter]
            postNotificationName: NSTextStorageWillProcessEditingNotification
                          object: self];

    // Attribute fixing (font substitution, paragraph styles) is implemented
    // by AppKit; without AppKit loaded the attributes are left as they are.
    if ([self respondsToSelector: @selector(fixAttributesInRange:)])
        [self fixAttributesInRange: NS_EXTRA(NSTextStorage, self)->_editedRange];

    [[NSNotificationCenter defaultCenter]
            postNotificationName: NSTextStorageDidProcessEditingNotification
                          object: self];

    count = [_layoutManagers count];
    for (i = 0; i < count; i++) {
        NSLayoutManager *layout = [_layoutManagers objectAtIndex: i];

        [layout textStorage: self
                          edited: [self editedMask]
                           range: [self editedRange]
                  changeInLength: [self changeInLength]
                invalidatedRange: [self invalidatedRange]];
    }
}

- (void) edited: (NSTextStorageEditActions) editedMask
                 range: (NSRange) range
        changeInLength: (NSInteger) delta
{

    if (_beginEditing == 0) {
        _editedMask = editedMask;
        _changeInLength = delta;
        range.length += delta;
        NS_EXTRA(NSTextStorage, self)->_editedRange = range;

        // Prevent any change to trigger more notification
        _beginEditing++;
        [self processEditing];
        _beginEditing--;
    } else {
        _editedMask |= editedMask;
        _changeInLength += delta;

        // The edited range so far and the range of this edit are both in the
        // string as it was before this edit. Their union contains every
        // character this edit replaced, so the change in length applies to
        // it as a whole and the result is a range in the edited string.
        NSRange edited = NS_EXTRA(NSTextStorage, self)->_editedRange;
        if (edited.location != -1 || edited.length != -1)
            range = NSUnionRange(edited, range);
        range.length += delta;
        NS_EXTRA(NSTextStorage, self)->_editedRange = range;
    }
}

- (void) setFont: (NSFont *) font {
    [self addAttribute: NSFontAttributeName
                 value: font
                 range: NSMakeRange(0, [self length])];
}

@end
