/*
 This file is part of Darling.

 Copyright (C) 2026 Darling developers

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

#import <NaturalLanguage/NLTokenizer.h>
#import <CoreFoundation/CoreFoundation.h>

// Word, sentence and paragraph tokens come from CFStringTokenizer; the document unit is the whole string.
// Placeholder: the language is accepted and not used, because those CFStringTokenizer units ignore the locale.
// Only NLTokenizerAttributeNumeric is ever reported, from the tokenizer's has-numbers flag; emoji and symbolic
// are not detected.
//
// GUESS: the documentation does not say whether a word token includes punctuation; CFStringTokenizer's choice is kept.
// GUESS: tokenRangeAtIndex: returns {NSNotFound, 0} when no token covers the index (whitespace between words
// for the word unit, an index at or beyond the end, an empty or nil string); the documentation names no result.
// GUESS: tokenRangeForRange: returns {NSNotFound, 0} when no token meets the range.
// GUESS: a zero-length range meets the token that covers its location, as tokenRangeForRange: documents;
// the documentation does not say what tokensForRange: and enumerateTokensInRange:usingBlock: do with one.
// GUESS: a range reaching past the end of the string is cut to the string, and one starting beyond the end
// meets no token; the documentation does not say whether an exception is raised.

// Calls block for each token of the whole string in order until block sets *stop.
static void NLTokenizerEnumerate(NSString *string, NLTokenUnit unit,
                                 void (^block)(NSRange range, NLTokenizerAttributes flags, BOOL *stop))
{
    NSUInteger length = [string length];
    BOOL stop = NO;
    if (length == 0) {
        return;
    }
    if (unit == NLTokenUnitDocument) {
        block(NSMakeRange(0, length), 0, &stop);
        return;
    }
    CFOptionFlags cfUnit = unit == NLTokenUnitSentence ? kCFStringTokenizerUnitSentence
                         : unit == NLTokenUnitParagraph ? kCFStringTokenizerUnitParagraph
                         : kCFStringTokenizerUnitWord;
    // The word, sentence and paragraph results are right only with the darling-corefoundation CFStringTokenizer
    // fixes (text lifetime, paragraph unit, word unit skipping whitespace and punctuation, numbers flag).
    CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(kCFAllocatorDefault, (CFStringRef)string,
                                                             CFRangeMake(0, length), cfUnit, NULL);
    if (!tokenizer) {
        return;
    }
    CFStringTokenizerTokenType type;
    while (!stop && (type = CFStringTokenizerAdvanceToNextToken(tokenizer)) != kCFStringTokenizerTokenNone) {
        CFRange token = CFStringTokenizerGetCurrentTokenRange(tokenizer);
        NLTokenizerAttributes flags = (type & kCFStringTokenizerTokenHasNumbersMask) ? NLTokenizerAttributeNumeric : 0;
        block(NSMakeRange(token.location, token.length), flags, &stop);
    }
    CFRelease(tokenizer);
}

// Cuts range to the string; NO if it starts beyond the end.
static BOOL NLTokenizerClamp(NSRange *range, NSUInteger length)
{
    if (range->location > length) {
        return NO;
    }
    // Not NSMaxRange: location + length overflows for a range such as {1, NSUIntegerMax}.
    if (range->length > length - range->location) {
        range->length = length - range->location;
    }
    return YES;
}

static BOOL NLTokenizerMeets(NSRange token, NSRange range)
{
    if (range.length == 0) {
        return range.location >= token.location && range.location < NSMaxRange(token);
    }
    return token.location < NSMaxRange(range) && NSMaxRange(token) > range.location;
}

@implementation NLTokenizer {
    NSString *_string;
}

- (instancetype)initWithUnit:(NLTokenUnit)unit
{
    if ((self = [super init])) {
        _unit = unit;
    }
    return self;
}

- (void)dealloc
{
    [_string release];
    [super dealloc];
}

- (NSString *)string
{
    return _string;
}

- (void)setString:(NSString *)string
{
    if (_string != string) {
        [_string release];
        _string = [string retain];
    }
}

- (void)setLanguage:(NLLanguage)language
{
}

- (void)enumerateTokensInRange:(NSRange)range
                    usingBlock:(void (^)(NSRange tokenRange, NLTokenizerAttributes flags, BOOL *stop))block
{
    if (!block || !NLTokenizerClamp(&range, [_string length])) {
        return;
    }
    __block BOOL stopped = NO;
    NLTokenizerEnumerate(_string, _unit, ^(NSRange token, NLTokenizerAttributes flags, BOOL *stop) {
        if (NLTokenizerMeets(token, range)) {
            block(token, flags, &stopped);
            *stop = stopped;
        } else if (token.location >= NSMaxRange(range)) {
            *stop = YES;
        }
    });
}

- (NSArray<NSValue *> *)tokensForRange:(NSRange)range
{
    NSMutableArray<NSValue *> *tokens = [NSMutableArray array];
    [self enumerateTokensInRange:range usingBlock:^(NSRange token, NLTokenizerAttributes flags __attribute__((unused)), BOOL *stop __attribute__((unused))) {
        [tokens addObject:[NSValue valueWithRange:token]];
    }];
    return tokens;
}

- (NSRange)tokenRangeAtIndex:(NSUInteger)characterIndex
{
    return [self tokenRangeForRange:NSMakeRange(characterIndex, 0)];
}

- (NSRange)tokenRangeForRange:(NSRange)range
{
    NSArray<NSValue *> *tokens = [self tokensForRange:range];
    if ([tokens count] == 0) {
        return NSMakeRange(NSNotFound, 0);
    }
    NSRange first = [[tokens objectAtIndex:0] rangeValue];
    NSRange last = [[tokens objectAtIndex:[tokens count] - 1] rangeValue];
    return NSMakeRange(first.location, NSMaxRange(last) - first.location);
}

@end
