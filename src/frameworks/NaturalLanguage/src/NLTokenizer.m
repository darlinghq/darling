/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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
#import <CoreFoundation/CFStringTokenizer.h>

// NLTokenizer is built on CoreFoundation's string tokenizer. Words are the
// tokenizer's word tokens that contain a letter, digit or symbol (whitespace
// and punctuation between words are not tokens); the other units are taken as
// they come.

static CFOptionFlags tokenizerUnit(NLTokenUnit unit)
{
    switch (unit) {
    case NLTokenUnitWord:
        return kCFStringTokenizerUnitWord;
    case NLTokenUnitSentence:
        return kCFStringTokenizerUnitSentence;
    case NLTokenUnitParagraph:
        return kCFStringTokenizerUnitParagraph;
    case NLTokenUnitDocument:
        break;
    }
    return kCFStringTokenizerUnitParagraph;
}

static BOOL isWord(NSString *string, NSRange range, NLTokenizerAttributes *attributes)
{
    static NSCharacterSet *wordCharacters;
    static NSCharacterSet *letters;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        NSMutableCharacterSet *set = [[NSCharacterSet alphanumericCharacterSet] mutableCopy];
        [set formUnionWithCharacterSet: [NSCharacterSet symbolCharacterSet]];
        wordCharacters = [set copy];
        [set release];
        letters = [[NSCharacterSet letterCharacterSet] retain];
    });
    NSString *token = [string substringWithRange: range];
    if ([token rangeOfCharacterFromSet: wordCharacters].location == NSNotFound)
        return NO;
    NLTokenizerAttributes flags = 0;
    if ([token rangeOfCharacterFromSet: letters].location == NSNotFound) {
        if ([token rangeOfCharacterFromSet: [NSCharacterSet decimalDigitCharacterSet]].location != NSNotFound)
            flags |= NLTokenizerAttributeNumeric;
        else
            flags |= NLTokenizerAttributeSymbolic;
    }
    *attributes = flags;
    return YES;
}

@implementation NLTokenizer {
    NLTokenUnit _unit;
    NSString *_string;
    NLLanguage _language;
}

- (instancetype)initWithUnit:(NLTokenUnit)unit
{
    self = [super init];
    if (self)
        _unit = unit;
    return self;
}

- (void)dealloc
{
    [_string release];
    [_language release];
    [super dealloc];
}

- (NLTokenUnit)unit
{
    return _unit;
}

- (NSString *)string
{
    return _string;
}

- (void)setString:(NSString *)string
{
    NSString *copy = [string copy];
    [_string release];
    _string = copy;
}

- (void)setLanguage:(NLLanguage)language
{
    NLLanguage copy = [language copy];
    [_language release];
    _language = copy;
}

- (void)enumerateTokensInRange:(NSRange)range usingBlock:(void (NS_NOESCAPE ^)(NSRange, NLTokenizerAttributes, BOOL *))block
{
    NSString *string = _string;
    if (string == nil || range.length == 0)
        return;
    if (NSMaxRange(range) > string.length)
        [NSException raise: NSRangeException format: @"-[NLTokenizer enumerateTokensInRange:usingBlock:]: range %@ is out of bounds", NSStringFromRange(range)];

    if (_unit == NLTokenUnitDocument) {
        BOOL stop = NO;
        block(NSMakeRange(0, string.length), 0, &stop);
        return;
    }

    CFLocaleRef locale = _language != nil ? CFLocaleCreate(NULL, (CFStringRef) _language) : NULL;
    CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(NULL, (CFStringRef) string,
        CFRangeMake(0, string.length), tokenizerUnit(_unit), locale);
    if (locale != NULL)
        CFRelease(locale);
    if (tokenizer == NULL)
        return;

    CFStringTokenizerGoToTokenAtIndex(tokenizer, range.location);
    CFRange token = CFStringTokenizerGetCurrentTokenRange(tokenizer);
    if (token.location == kCFNotFound) {
        if (CFStringTokenizerAdvanceToNextToken(tokenizer) == kCFStringTokenizerTokenNone) {
            CFRelease(tokenizer);
            return;
        }
        token = CFStringTokenizerGetCurrentTokenRange(tokenizer);
    }
    while (token.location != kCFNotFound && (NSUInteger) token.location < NSMaxRange(range)) {
        NSRange tokenRange = NSMakeRange(token.location, token.length);
        NLTokenizerAttributes attributes = 0;
        BOOL stop = NO;
        if (_unit != NLTokenUnitWord || isWord(string, tokenRange, &attributes))
            block(tokenRange, attributes, &stop);
        if (stop)
            break;
        if (CFStringTokenizerAdvanceToNextToken(tokenizer) == kCFStringTokenizerTokenNone)
            break;
        token = CFStringTokenizerGetCurrentTokenRange(tokenizer);
    }
    CFRelease(tokenizer);
}

- (NSArray<NSValue *> *)tokensForRange:(NSRange)range
{
    NSMutableArray<NSValue *> *tokens = [NSMutableArray array];
    [self enumerateTokensInRange: range usingBlock: ^(NSRange tokenRange, NLTokenizerAttributes flags, BOOL *stop) {
        (void) flags;
        (void) stop;
        [tokens addObject: [NSValue valueWithRange: tokenRange]];
    }];
    return tokens;
}

- (NSRange)tokenRangeAtIndex:(NSUInteger)characterIndex
{
    __block NSRange result = NSMakeRange(NSNotFound, 0);
    if (_string == nil || characterIndex >= _string.length)
        return result;
    [self enumerateTokensInRange: NSMakeRange(characterIndex, 1) usingBlock: ^(NSRange tokenRange, NLTokenizerAttributes flags, BOOL *stop) {
        (void) flags;
        if (NSLocationInRange(characterIndex, tokenRange))
            result = tokenRange;
        *stop = YES;
    }];
    return result;
}

// the smallest range of whole tokens that covers the range
- (NSRange)tokenRangeForRange:(NSRange)range
{
    NSArray<NSValue *> *tokens = [self tokensForRange: range];
    if (tokens.count == 0)
        return NSMakeRange(NSNotFound, 0);
    NSRange first = tokens.firstObject.rangeValue;
    NSRange last = tokens.lastObject.rangeValue;
    return NSMakeRange(first.location, NSMaxRange(last) - first.location);
}

@end
