/* Copyright (c) 2007 Christopher J. W. Lloyd

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

#import "UIFoundation-ExtraIvars.h"
#import <AppKit/NSTextTable.h>

@implementation NSTextTable

- (unsigned) numberOfColumns {
    return UIF_EXTRA_NSTextTable(self)->_numberOfColumns;
}

- (BOOL) collapsesBorders {
    return UIF_EXTRA_NSTextTable(self)->_collapsesBorders;
}

- (BOOL) hidesEmptyCells {
    return UIF_EXTRA_NSTextTable(self)->_hidesEmptyCells;
}

- (NSTextTableLayoutAlgorithm) layoutAlgorithm {
    return UIF_EXTRA_NSTextTable(self)->_layoutAlgorithm;
}

- (void) setNumberOfColumns: (unsigned) numberOfColumns {
    UIF_EXTRA_NSTextTable(self)->_numberOfColumns = numberOfColumns;
}

- (void) setCollapsesBorders: (BOOL) yorn {
    UIF_EXTRA_NSTextTable(self)->_collapsesBorders = yorn;
}

- (void) setHidesEmptyCells: (BOOL) yorn {
    UIF_EXTRA_NSTextTable(self)->_hidesEmptyCells = yorn;
}

- (void) setLayoutAlgorithm: (NSTextTableLayoutAlgorithm) algorithm {
    UIF_EXTRA_NSTextTable(self)->_layoutAlgorithm = algorithm;
}

UIF_EXTRA_DEALLOC(NSTextTable)

@end
