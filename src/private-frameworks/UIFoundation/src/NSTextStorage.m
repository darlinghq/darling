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

- (void) dealloc {
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

- (int) changeInLength {
    return _changeInLength;
}

- (unsigned) editedMask {
    return _editedMask;
}

- (NSRange) editedRange {
    return NS_EXTRA(NSTextStorage, self)->_editedRange;
}

- (void) setDelegate: delegate {
    _delegate = delegate;
}

- (void) addLayoutManager: (NSLayoutManager *) layoutManager {
    if (layoutManager == nil || [_layoutManagers indexOfObjectIdenticalTo: layoutManager] != NSNotFound)
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
    int i, count;

    if ([_delegate respondsToSelector: @selector
                   (textStorageWillProcessEditing:)]) {
        NSNotification *note = [NSNotification
                notificationWithName:
                        NSTextStorageWillProcessEditingNotification
                              object: self
                            userInfo: nil];
        [_delegate textStorageWillProcessEditing: note];
    }

    [[NSNotificationCenter defaultCenter]
            postNotificationName: NSTextStorageWillProcessEditingNotification
                          object: self];

    // Attribute fixing (font substitution, paragraph styles) is implemented
    // by AppKit; without AppKit loaded the attributes are left as they are.
    if ([self respondsToSelector: @selector(fixAttributesInRange:)])
        [self fixAttributesInRange: NS_EXTRA(NSTextStorage, self)->_editedRange];

    if ([_delegate
                respondsToSelector: @selector(textStorageDidProcessEditing:)]) {
        NSNotification *note = [NSNotification
                notificationWithName: NSTextStorageDidProcessEditingNotification
                              object: self
                            userInfo: nil];
        [_delegate textStorageDidProcessEditing: note];
    }

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

- (void) edited: (unsigned) editedMask
                 range: (NSRange) range
        changeInLength: (int) delta
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
        range.length += delta;

        if (NS_EXTRA(NSTextStorage, self)->_editedRange.location == -1 && NS_EXTRA(NSTextStorage, self)->_editedRange.length == -1)
            NS_EXTRA(NSTextStorage, self)->_editedRange = range;
        else
            NS_EXTRA(NSTextStorage, self)->_editedRange = NSUnionRange(NS_EXTRA(NSTextStorage, self)->_editedRange, range);
    }
}

- (void) setFont: (NSFont *) font {
    [self addAttribute: NSFontAttributeName
                 value: font
                 range: NSMakeRange(0, [self length])];
}

@end
