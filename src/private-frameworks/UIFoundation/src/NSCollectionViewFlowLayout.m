/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <AppKit/NSCollectionViewFlowLayout.h>

NSString *const NSCollectionElementKindSectionHeader =
        @"UICollectionElementKindSectionHeader";
NSString *const NSCollectionElementKindSectionFooter =
        @"UICollectionElementKindSectionFooter";

// Not declared in a public header.
APPKIT_EXPORT NSString *const NSFlowLayoutCommonRowHorizontalAlignmentKey;
APPKIT_EXPORT NSString *const NSFlowLayoutLastRowHorizontalAlignmentKey;
APPKIT_EXPORT NSString *const NSFlowLayoutRowVerticalAlignmentKey;
NSString *const NSFlowLayoutCommonRowHorizontalAlignmentKey =
        @"UIFlowLayoutCommonRowHorizontalAlignmentKey";
NSString *const NSFlowLayoutLastRowHorizontalAlignmentKey =
        @"UIFlowLayoutLastRowHorizontalAlignmentKey";
NSString *const NSFlowLayoutRowVerticalAlignmentKey =
        @"UIFlowLayoutRowVerticalAlignmentKey";

@implementation NSCollectionViewFlowLayout
@synthesize itemSize = _itemSize;
@synthesize sectionInset = _sectionInset;
@synthesize minimumLineSpacing = _minimumLineSpacing;
@synthesize minimumInteritemSpacing = _minimumInteritemSpacing;

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

- (instancetype) init {
    if ((self = [super init]) != nil) {
        _itemSize = NSMakeSize(50, 50);
        _minimumLineSpacing = 10;
        _minimumInteritemSpacing = 10;
    }
    return self;
}

// Without this, any nib containing a flow layout fails to unarchive. The unarchiver sends
// initWithCoder:, which found no implementation here and fell through to the stub forwarding above:
// -methodSignatureForSelector: advertises "v@:" for every selector, so the call came back as
//   NSForwardSignatureError: invoked with 3 args, but 2 expected. Selector initWithCoder:,
// and that one element aborted the whole nib.
//
// The archived properties are not restored; the defaults from -init are, which is all this stub
// class does anyway.
- (instancetype) initWithCoder: (NSCoder *) coder {
    (void) coder;
    return [self init];
}

@end
