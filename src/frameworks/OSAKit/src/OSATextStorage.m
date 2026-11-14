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

#import <OSAKit/OSATextStorage.h>
#import <darling-stub-forwarding.h>

@implementation OSATextStorage

@synthesize wrapsLines = _wrapsLines;
@synthesize indentsWrappedLines = _indentsWrappedLines;
@synthesize date = _date;
@synthesize controller = _controller;

- (instancetype)init
{
    return [self initWithString:@""];
}

- (instancetype)initWithString:(NSString *)string
{
    if ((self = [super initWithString:string]))
        _contents = [[NSMutableAttributedString alloc] initWithString:string];
    return self;
}

- (instancetype)initWithAttributedString:(NSAttributedString *)string
{
    if ((self = [super initWithString:@""]))
        _contents = [string mutableCopy];
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ((self = [super initWithCoder:coder]))
        _contents = [[NSMutableAttributedString alloc] initWithString:@""];
    return self;
}

- (void)dealloc
{
    [_contents release];
    [_date release];
    [super dealloc];
}

- (NSString *)string
{
    return [_contents string];
}

// Text views ask for the attributes at the end of the text (index 0 of an empty storage).
- (NSDictionary *)attributesAtIndex:(NSUInteger)location effectiveRange:(NSRangePointer)range
{
    if (location == [_contents length]) {
        if (range != NULL)
            *range = NSMakeRange(location, 0);
        return [NSDictionary dictionary];
    }
    return [_contents attributesAtIndex:location effectiveRange:range];
}

- (void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string
{
    [_contents replaceCharactersInRange:range withString:string];
    [self setDate:[NSDate date]];
    [self edited:NSTextStorageEditedCharacters | NSTextStorageEditedAttributes
           range:range
  changeInLength:(NSInteger)[string length] - (NSInteger)range.length];
}

- (void)replaceCharactersInRange:(NSRange)range withAttributedString:(NSAttributedString *)string
{
    [_contents replaceCharactersInRange:range withAttributedString:string];
    [self setDate:[NSDate date]];
    [self edited:NSTextStorageEditedCharacters | NSTextStorageEditedAttributes
           range:range
  changeInLength:(NSInteger)[string length] - (NSInteger)range.length];
}

- (void)setAttributes:(NSDictionary *)attributes range:(NSRange)range
{
    [_contents setAttributes:attributes range:range];
    [self edited:NSTextStorageEditedAttributes range:range changeInLength:0];
}

- (void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string withUndoManager:(NSUndoManager *)undoManager
{
    // New text takes the attributes of the character before it, like typing does.
    NSDictionary *attributes = nil;
    if ([self length] > 0)
        attributes = [self attributesAtIndex:range.location > 0 ? range.location - 1 : 0 effectiveRange:NULL];
    NSAttributedString *replacement = [[[NSAttributedString alloc] initWithString:string ?: @"" attributes:attributes] autorelease];
    [self replaceCharactersInRange:range withAttributedString:replacement withUndoManager:undoManager];
}

- (void)replaceCharactersInRange:(NSRange)range withAttributedString:(NSAttributedString *)string withUndoManager:(NSUndoManager *)undoManager
{
    NSAttributedString *previous = [self attributedSubstringFromRange:range];
    NSRange replaced = NSMakeRange(range.location, [string length]);
    // __block keeps the block from retaining the undo manager that owns it.
    __block NSUndoManager *manager = undoManager;
    [undoManager registerUndoWithTarget:self handler:^(OSATextStorage *storage) {
        [storage replaceCharactersInRange:replaced withAttributedString:previous withUndoManager:manager];
    }];
    [self replaceCharactersInRange:range withAttributedString:string];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end
