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

#import <AppKit/NSRangeArray.h>

@implementation NSRangeArray

- init {
    _count = 0;
    _capacity = 2;
    _ranges = NSZoneMalloc([self zone], sizeof(NSRange) * _capacity);
    return self;
}

- (void) dealloc {
    NSZoneFree([self zone], _ranges);
    [super dealloc];
}

- (unsigned) count {
    return _count;
}

- (NSRange) rangeAtIndex: (unsigned) index {
    NSAssert2(index < _count, @"index %d beyond count %d", index, _count);

    return _ranges[index];
}

- (void) addRange: (NSRange) range {
    if (_count >= _capacity) {
        _capacity *= 2;
        _ranges = NSZoneRealloc([self zone], _ranges,
                                sizeof(NSRange) * _capacity);
    }

    _ranges[_count++] = range;
}

- (void) removeRangeAtIndex: (unsigned) index {
    NSAssert2(index < _count, @"index %d beyond count %d", index, _count);

    _count--;
    for (; index < _count; ++index)
        _ranges[index] = _ranges[index + 1];
}

- (void) removeAllRanges {
    _count = 0;
}

@end
