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
#import <Foundation/NSKeyedArchiver.h>

#import <AppKit/NSParagraphStyle.h>
#import <AppKit/NSTextTab.h>

NSTextTabOptionKey NSTabColumnTerminatorsAttributeName =
        @"NSTabColumnTerminatorsAttributeName";

@implementation NSTextTab
#if __OBJC2__
{
    _NSTEXTTAB_IVARS
}
#endif

+ (BOOL) supportsSecureCoding {
    return YES;
}

+ (NSCharacterSet *) columnTerminatorsForLocale: (NSLocale *) locale {
    if (locale == nil)
        locale = [NSLocale systemLocale];
    return [NSCharacterSet characterSetWithCharactersInString: [locale objectForKey: NSLocaleDecimalSeparator]];
}

- (instancetype) initWithTextAlignment: (NSTextAlignment) alignment
                              location: (CGFloat) location
                               options: (NSDictionary *) options
{
    if ((self = [super init])) {
        _alignment = alignment;
        _location = location;
        _options = options ? [options copy] : [[NSDictionary alloc] init];
    }
    return self;
}

// Apple documents NSDecimalTabStopType as right alignment with the current
// locale's column terminators.
- (instancetype) initWithType: (NSTextTabType) type location: (CGFloat) location {
    NSTextAlignment alignment = NSTextAlignmentLeft;
    NSDictionary *options = nil;

    switch (type) {
    case NSLeftTabStopType:
        alignment = NSTextAlignmentLeft;
        break;
    case NSRightTabStopType:
        alignment = NSTextAlignmentRight;
        break;
    case NSCenterTabStopType:
        alignment = NSTextAlignmentCenter;
        break;
    case NSDecimalTabStopType:
        alignment = NSTextAlignmentRight;
        options = @{
            NSTabColumnTerminatorsAttributeName :
                    [NSTextTab columnTerminatorsForLocale: [NSLocale currentLocale]]
        };
        break;
    }
    return [self initWithTextAlignment: alignment location: location options: options];
}

- (id) initWithCoder: (NSCoder *) aDecoder {
    if ([aDecoder allowsKeyedCoding]) {
        CGFloat location = [aDecoder decodeFloatForKey: @"Location"];
        if (![aDecoder containsValueForKey: @"Alignment"])
            return [self initWithType: [aDecoder decodeIntForKey: @"Type"] location: location];
        NSSet *classes = [NSSet setWithObjects: [NSDictionary class], [NSString class],
                                                [NSCharacterSet class], nil];
        return [self initWithTextAlignment: [aDecoder decodeIntegerForKey: @"Alignment"]
                                  location: location
                                   options: [aDecoder decodeObjectOfClasses: classes forKey: @"Options"]];
    }
    // QUIRK: NSArchiver (typedstream) layout of a tab stop: the tab type as a
    // char and the location as a float. Not documented.
    // Evidence: archive written by NSArchiver on macOS 27 (tier 2, MC13.7,
    // fixture texttab.typedstream).
    // Test: test_quirk_typedstream_text_tab in tests/appkit-typedstream-text.m
    unsigned char type;
    float location;
    [aDecoder decodeValuesOfObjCTypes: "Cf", &type, &location];
    return [self initWithType: type location: location];
}

- (void) encodeWithCoder: (NSCoder *) aCoder {
    if (![aCoder allowsKeyedCoding])
        [NSException raise: NSInvalidArgumentException
                    format: @"-[%@ %s]: only keyed archiving is supported", [self class], sel_getName(_cmd)];
    [aCoder encodeInt: [self tabStopType] forKey: @"Type"];
    [aCoder encodeInteger: _alignment forKey: @"Alignment"];
    [aCoder encodeFloat: _location forKey: @"Location"];
    [aCoder encodeObject: _options forKey: @"Options"];
}

- (void) dealloc {
    [_options release];
    [super dealloc];
}

- copyWithZone: (NSZone *) zone {
    return [self retain];
}

- (NSTextAlignment) alignment {
    return _alignment;
}

- (NSDictionary *) options {
    return _options;
}

- (NSTextTabType) tabStopType {
    switch (_alignment) {
    case NSTextAlignmentRight:
        return [_options objectForKey: NSTabColumnTerminatorsAttributeName] ? NSDecimalTabStopType
                                                                            : NSRightTabStopType;
    case NSTextAlignmentCenter:
        return NSCenterTabStopType;
    case NSTextAlignmentNatural:
        return [[NSParagraphStyle defaultParagraphStyle] baseWritingDirection] ==
                               NSWritingDirectionRightToLeft
                       ? NSRightTabStopType
                       : NSLeftTabStopType;
    default:
        return NSLeftTabStopType;
    }
}

- (CGFloat) location {
    return _location;
}

- (NSUInteger) hash {
    return (NSUInteger) (NSInteger) _location ^ (NSUInteger) _alignment;
}

- (BOOL) isEqual: (id) object {
    if (self == object) {
        return YES;
    }
    if (![object isKindOfClass: [NSTextTab class]]) {
        return NO;
    }
    NSTextTab *other = (NSTextTab *) object;
    return _location == other->_location && _alignment == other->_alignment &&
           [_options isEqual: other->_options];
}

- (NSComparisonResult) compare: (NSTextTab *) other {
    if (other == self)
        return NSOrderedSame;
    if (other == nil || ![other isKindOfClass: [NSTextTab class]])
        return NSOrderedAscending;
    if (_location < other->_location)
        return NSOrderedAscending;
    else if (_location > other->_location)
        return NSOrderedDescending;
    else
        return NSOrderedSame;
}

@end
