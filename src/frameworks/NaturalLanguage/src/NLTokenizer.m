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
// Observed on macOS 26.7.3 (black-box observation through the public API; tests/src/naturallanguage_tokenizer.m):
// punctuation is not part of a word token; tokenRangeAtIndex: returns {NSNotFound, 0} when no token covers
// the index (whitespace between words for the word unit, an index at or beyond the end, an empty or unset
// string); tokenRangeForRange: returns the union of the tokens that meet the range, or {NSNotFound, 0}; it
// cuts a range that reaches past the end to the string, and finds nothing for one that starts beyond it.
// tokensForRange: and enumerateTokensInRange:usingBlock: raise no exception and are stricter, see
// NLTokenizerTokensMeetingRange. Not observed, so left as it was written: which tokens a placeholder
// should flag symbolic or emoji (only numeric is reported).

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
        // CoreFoundation spells this mask "HasHasNumbers" (the documented name).
        NLTokenizerAttributes flags = (type & kCFStringTokenizerTokenHasHasNumbersMask) ? NLTokenizerAttributeNumeric : 0;
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

// Whether token meets a range that ends at end (location + length, which may have wrapped around). A
// zero-length range meets the token that covers its location.
static BOOL NLTokenizerMeets(NSRange token, NSUInteger location, NSUInteger length, NSUInteger end)
{
    if (length == 0) {
        return location >= token.location && location < NSMaxRange(token);
    }
    return token.location < end && NSMaxRange(token) > location;
}

@implementation NLTokenizer

@synthesize unit = _unit;

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

// Calls block for each token that meets range, in order. range has been checked or cut by the caller.
// end is range's end as location + length, which may have wrapped around.
static void NLTokenizerEnumerateMeeting(NSString *string, NLTokenUnit unit, NSRange range, NSUInteger end,
                                        void (^block)(NSRange tokenRange, NLTokenizerAttributes flags, BOOL *stop))
{
    __block BOOL stopped = NO;
    NLTokenizerEnumerate(string, unit, ^(NSRange token, NLTokenizerAttributes flags, BOOL *stop) {
        if (NLTokenizerMeets(token, range.location, range.length, end)) {
            block(token, flags, &stopped);
            *stop = stopped;
        } else if (end >= range.location && token.location >= end) {
            *stop = YES;
        }
    });
}

// The tokens meeting a range, with the checks of tokensForRange: and enumerateTokensInRange:usingBlock:.
// QUIRK: these two do not cut a range to the string like tokenRangeForRange: does. They find no token
// when location + length (computed with wraparound, so {1, NSUIntegerMax} ends at 0) is beyond the end of
// the string, whatever the unit; for the document unit anything else that is inside the string, including
// {1, NSUIntegerMax} and an empty range, meets the one document token; for the other units an empty range
// at location 0 finds no token, while one elsewhere meets the token that covers it.
// Not documented. Evidence: black-box observation on macOS 26.7.3 (tier 2), probe nlr.m with
// {0,48}, {2,46}, {47,1}, {1,NSUIntegerMax}, {40,NSUIntegerMax}, {0,0}, {46,0}, {47,0} on all four units.
// Test: test_quirk_tokens_for_range_past_end in tests/src/naturallanguage_tokenizer.m
static void NLTokenizerTokensMeetingRange(NSString *string, NLTokenUnit unit, NSRange range,
                                          void (^block)(NSRange tokenRange, NLTokenizerAttributes flags, BOOL *stop))
{
    NSUInteger length = [string length];
    NSUInteger end = range.location + range.length;
    if (length == 0 || end > length) {
        return;
    }
    if (unit == NLTokenUnitDocument) {
        BOOL stop = NO;
        block(NSMakeRange(0, length), 0, &stop);
        return;
    }
    if (range.length == 0 && range.location == 0) {
        return;
    }
    NLTokenizerEnumerateMeeting(string, unit, range, end, block);
}

- (void)enumerateTokensInRange:(NSRange)range
                    usingBlock:(void (^)(NSRange tokenRange, NLTokenizerAttributes flags, BOOL *stop))block
{
    if (!block) {
        return;
    }
    NLTokenizerTokensMeetingRange(_string, _unit, range, block);
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
    // Unlike tokensForRange:, a range that reaches past the end is cut to the string, and an empty range
    // meets the token that covers its location (also at location 0).
    __block BOOL found = NO;
    __block NSRange result = NSMakeRange(NSNotFound, 0);
    NSUInteger length = [_string length];
    if (length == 0 || !NLTokenizerClamp(&range, length) || (range.length == 0 && range.location == length)) {
        return result;
    }
    NLTokenizerEnumerateMeeting(_string, _unit, range, range.location + range.length,
                                ^(NSRange token, NLTokenizerAttributes flags __attribute__((unused)), BOOL *stop __attribute__((unused))) {
        if (!found) {
            result = token;
            found = YES;
        } else {
            result.length = NSMaxRange(token) - result.location;
        }
    });
    return result;
}

@end
