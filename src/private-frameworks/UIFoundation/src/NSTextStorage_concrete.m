/* Copyright (c) 2006-2009 Christopher J. W. Lloyd <cjwl@objc.net>

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
#import <Foundation/NSKeyedArchiver.h>

@implementation NSTextStorage_concrete

- initWithCoder: (NSCoder *) coder {
    [super initWithCoder: coder];

    if ([coder allowsKeyedCoding]) {
        NSKeyedUnarchiver *keyed = (NSKeyedUnarchiver *) coder;

        _delegate = [keyed decodeObjectForKey: @"NSDelegate"];
        // A mutable copy, as -initWithString: keeps: the decoded string is immutable, and edits change it in place.
        NSString *decoded = [keyed decodeObjectForKey: @"NSString"];
        _string = decoded ? [decoded mutableCopy] : [NSMutableString new];
        _rangeToAttributes = NSCreateRangeToCopiedObjectEntries(0);
        NSRangeEntryInsert(_rangeToAttributes, NSMakeRange(0, [_string length]),
                           [NSDictionary dictionary]);
    } else {
        // QUIRK: NSArchiver (typedstream) layout of a text storage: the string,
        // then "iI" runs of (1-based attribute dictionary number, length)
        // covering it; a dictionary is archived right after the first run that
        // uses it. Not documented.
        // Evidence: archives written by NSArchiver on macOS 27 (tier 2, MC13.7,
        // fixtures textstorage.typedstream and attributedstring.typedstream).
        // Test: test_quirk_typedstream_text_storage in
        // tests/appkit-typedstream-text.m
        NSString *decoded = [coder decodeObject];
        _string = decoded ? [decoded mutableCopy] : [NSMutableString new];
        _rangeToAttributes = NSCreateRangeToCopiedObjectEntries(0);
        NSUInteger length = [_string length], location = 0;
        if (length == 0)
            NSRangeEntryInsert(_rangeToAttributes, NSMakeRange(0, 0), [NSDictionary dictionary]);
        NSMutableArray *dictionaries = [NSMutableArray array];
        while (location < length) {
            int number;
            unsigned int runLength;
            [coder decodeValuesOfObjCTypes: "iI", &number, &runLength];
            if (number == (int) [dictionaries count] + 1) {
                NSDictionary *attributes = [coder decodeObject];
                if (attributes != nil)
                    [dictionaries addObject: attributes];
            }
            if (number < 1 || number > (int) [dictionaries count] || runLength == 0 || runLength > length - location)
                [NSException raise: NSInvalidArgumentException
                            format: @"-[%@ %s]: bad attribute run %d length %u at %lu of %lu", [self class],
                                    sel_getName(_cmd), number, runLength, (unsigned long) location, (unsigned long) length];
            NSRangeEntryInsert(_rangeToAttributes, NSMakeRange(location, runLength), dictionaries[number - 1]);
            location += runLength;
        }
    }
    return self;
}

- initWithString: (NSString *) string {
    [super initWithString: string];
    _string = [string mutableCopy];
    _rangeToAttributes = NSCreateRangeToCopiedObjectEntries(0);
    NSRangeEntryInsert(_rangeToAttributes, NSMakeRange(0, [_string length]),
                       [NSDictionary dictionary]);
    return self;
}

- initWithString: (NSString *) string attributes: (NSDictionary *) attributes {
    // NSAttributedString only provides this initializer for its own concrete
    // classes, so the text storage supplies it for itself.
    if ((self = [self initWithString: string]) != nil && [attributes count] > 0)
        [self setAttributes: attributes range: NSMakeRange(0, [string length])];
    return self;
}

- initWithAttributedString: (NSAttributedString *) attributedString {
    if ((self = [self initWithString: @""]) != nil)
        [self setAttributedString: attributedString];
    return self;
}

- init {
    return [self initWithString: @""];
}

- (void) dealloc {
    [_string release];
    NSFreeRangeEntries(_rangeToAttributes);
    [super dealloc];
}

- (NSString *) string {
    return _string;
}

- (NSDictionary *) attributesAtIndex: (NSUInteger) location
                      effectiveRange: (NSRangePointer) effectiveRangep
{
    NSDictionary *result;

    NSAssert2(location <= [self length], @"index %lu beyond length %lu", (unsigned long) location,
              (unsigned long) [self length]);

    if ((result = NSRangeEntryAtIndex(_rangeToAttributes, location,
                                      effectiveRangep)) == nil)
        result = [NSDictionary dictionary];

    if (effectiveRangep != NULL && effectiveRangep->length == NSNotFound)
        effectiveRangep->length = [self length] - effectiveRangep->location;

    return [[result retain] autorelease];
}

static inline NSInteger
replaceCharactersInRangeWithString(NSTextStorage_concrete *self, NSRange range,
                                   NSString *string)
{
    NSInteger delta = (NSInteger) [string length] - (NSInteger) range.length;

    [self->_string replaceCharactersInRange: range withString: string];

    // NSRangeEntriesDump(self->_rangeToAttributes);

    NSRangeEntriesExpandAndWipe(self->_rangeToAttributes, range, delta);
    if (NSCountRangeEntries(self->_rangeToAttributes) == 0)
        NSRangeEntryInsert(self->_rangeToAttributes,
                           NSMakeRange(0, [self->_string length]),
                           [NSDictionary dictionary]);

    NSRangeEntriesVerify(self->_rangeToAttributes, [self length]);

    return delta;
}

static inline void setAttributes(NSTextStorage_concrete *self,
                                 NSDictionary *attributes, NSRange range)
{
    if (attributes == nil)
        attributes = [NSDictionary dictionary];

    if ([self->_string length] == 0) {
        NSResetRangeEntries(self->_rangeToAttributes);
        NSRangeEntryInsert(self->_rangeToAttributes, range, attributes);
    } else if (range.length > 0) {
        NSRangeEntriesDivideAndConquer(self->_rangeToAttributes, range);
        NSRangeEntryInsert(self->_rangeToAttributes, range, attributes);
    }

    NSRangeEntriesVerify(self->_rangeToAttributes, [self length]);
}

static inline void
replaceCharactersInRangeWithAttributedString(NSTextStorage_concrete *self,
                                             NSRange replaced,
                                             NSAttributedString *other)
{
    NSString *string = [other string];
    NSUInteger location = 0;
    NSUInteger limit = [string length];
    NSInteger delta = replaceCharactersInRangeWithString(self, replaced, string);

    while (location < limit) {
        NSRange effectiveRange;
        NSDictionary *attributes = [other attributesAtIndex: location
                                             effectiveRange: &effectiveRange];
        NSRange range = NSMakeRange(replaced.location + location,
                                    effectiveRange.length);

        setAttributes(self, attributes, range);

        location = NSMaxRange(effectiveRange);
    }

    if (limit == 0) {
        // That will just try to merge attributes at the location when possible
        NSRangeEntryInsert(self->_rangeToAttributes,
                           NSMakeRange(replaced.location, 0), nil);
    }

    [self edited: NSTextStorageEditedAttributes | NSTextStorageEditedCharacters
                     range: replaced
            changeInLength: delta];
}

- (void) replaceCharactersInRange: (NSRange) range
                       withString: (NSString *) string
{
    NSInteger delta = replaceCharactersInRangeWithString(self, range, string);
    [self edited: NSTextStorageEditedAttributes | NSTextStorageEditedCharacters
                     range: range
            changeInLength: delta];
}

- (void) setAttributes: (NSDictionary *) attributes range: (NSRange) range {
    setAttributes(self, attributes, range);
    [self edited: NSTextStorageEditedAttributes range: range changeInLength: 0];
}

- (void) replaceCharactersInRange: (NSRange) replaced
             withAttributedString: (NSAttributedString *) other
{
    replaceCharactersInRangeWithAttributedString(self, replaced, other);
}

- (void) setAttributedString: (NSAttributedString *) attributedString {
    [self beginEditing];
    replaceCharactersInRangeWithAttributedString(
            self, NSMakeRange(0, [self length]), attributedString);
    [self endEditing];
}

- (NSMutableString *) mutableString {
    return [[[NSClassFromString(
            @"NSMutableStringProxyForMutableAttributedString")
            allocWithZone: NULL]
            performSelector: @selector(initWithMutableAttributedString:)
                 withObject: self] autorelease];
}

- (void) fixAttributesAfterEditingRange: (NSRange) range {
}

@end
