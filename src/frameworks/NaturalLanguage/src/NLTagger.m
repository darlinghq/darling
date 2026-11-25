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

#import <NaturalLanguage/NLTagger.h>

// Placeholder: no tagging is implemented. Every token is reported with no tag: tagAtIndex:... returns nil and
// leaves tokenRange untouched (the documentation sets it only when a tag is returned), enumerateTagsInRange:...
// calls its block with a nil tag for each token that meets the range, and tagsInRange:... returns an empty
// string for each such token, the documented entry for a token without a tag. The tokens are NLTokenizer's
// tokens for the unit. Each of those calls logs that it was called, once. tokenRangeAtIndex:unit: and
// tokenRangeForRange:unit: are not placeholders: they ask an NLTokenizer for the same unit.
//
// GUESS: the omit options are applied only for the word unit, and a token is classified by its characters:
// all whitespace and newlines is whitespace, all punctuation is punctuation, all symbols is other, anything
// else is a word. The documentation names the four token types but not how a token is assigned to one.
// NLTaggerJoinNames and NLTaggerJoinContractions are ignored.
// GUESS: dominantLanguage is NLLanguageUndetermined; the documentation types it as optional and does not say
// which of nil and undetermined means that no language was found.
@implementation NLTagger

@synthesize tagSchemes = _tagSchemes;

- (instancetype)initWithTagSchemes:(NSArray<NLTagScheme> *)tagSchemes
{
    if ((self = [super init])) {
        _tagSchemes = [tagSchemes copy];
    }
    return self;
}

- (void)dealloc
{
    [_tagSchemes release];
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

- (NLLanguage)dominantLanguage
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return NLLanguageUndetermined;
}

// The omit option that applies to a word-unit token, or 0 for a word.
static NLTaggerOptions NLTaggerOmitOptionForToken(NSString *token)
{
    NSCharacterSet *sets[] = {
        [NSCharacterSet whitespaceAndNewlineCharacterSet],
        [NSCharacterSet punctuationCharacterSet],
        [NSCharacterSet symbolCharacterSet],
    };
    NLTaggerOptions options[] = {NLTaggerOmitWhitespace, NLTaggerOmitPunctuation, NLTaggerOmitOther};
    for (int i = 0; i < 3; i++) {
        if ([[token stringByTrimmingCharactersInSet:sets[i]] length] == 0) {
            return options[i];
        }
    }
    return NLTaggerOmitWords;
}

// The ranges of the tokens of the string for unit that meet range, less the tokens the options omit.
static NSArray<NSValue *> *NLTaggerTokenRanges(NSString *string, NSRange range, NLTokenUnit unit, NLTaggerOptions options)
{
    NLTokenizer *tokenizer = [[NLTokenizer alloc] initWithUnit:unit];
    tokenizer.string = string;
    NSArray<NSValue *> *tokens = [tokenizer tokensForRange:range];
    [tokenizer release];
    NLTaggerOptions omit = options & (NLTaggerOmitWords | NLTaggerOmitPunctuation | NLTaggerOmitWhitespace | NLTaggerOmitOther);
    if (unit != NLTokenUnitWord || omit == 0) {
        return tokens;
    }
    NSMutableArray<NSValue *> *kept = [NSMutableArray array];
    for (NSValue *token in tokens) {
        NSString *text = [string substringWithRange:[token rangeValue]];
        if (!(omit & NLTaggerOmitOptionForToken(text))) {
            [kept addObject:token];
        }
    }
    return kept;
}

+ (NSArray<NLTagScheme> *)availableTagSchemesForUnit:(NLTokenUnit)unit language:(NLLanguage)language
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return [NSArray array];
}

- (void)setLanguage:(NLLanguage)language range:(NSRange)range
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
}

- (NSRange)tokenRangeAtIndex:(NSUInteger)characterIndex unit:(NLTokenUnit)unit
{
    NLTokenizer *tokenizer = [[NLTokenizer alloc] initWithUnit:unit];
    tokenizer.string = _string;
    NSRange range = [tokenizer tokenRangeAtIndex:characterIndex];
    [tokenizer release];
    return range;
}

- (NSRange)tokenRangeForRange:(NSRange)range unit:(NLTokenUnit)unit
{
    NLTokenizer *tokenizer = [[NLTokenizer alloc] initWithUnit:unit];
    tokenizer.string = _string;
    NSRange result = [tokenizer tokenRangeForRange:range];
    [tokenizer release];
    return result;
}

- (NLTag)tagAtIndex:(NSUInteger)characterIndex
               unit:(NLTokenUnit)unit
             scheme:(NLTagScheme)scheme
         tokenRange:(NSRangePointer)tokenRange
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return nil;
}

- (NSArray<NLTag> *)tagsInRange:(NSRange)range
                           unit:(NLTokenUnit)unit
                         scheme:(NLTagScheme)scheme
                        options:(NLTaggerOptions)options
                    tokenRanges:(NSArray<NSValue *> **)tokenRanges
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    NSArray<NSValue *> *tokens = NLTaggerTokenRanges(_string, range, unit, options);
    NSMutableArray<NLTag> *tags = [NSMutableArray array];
    for (NSUInteger i = 0; i < [tokens count]; i++) {
        [tags addObject:@""];
    }
    if (tokenRanges) {
        *tokenRanges = tokens;
    }
    return tags;
}

- (void)enumerateTagsInRange:(NSRange)range
                        unit:(NLTokenUnit)unit
                      scheme:(NLTagScheme)scheme
                     options:(NLTaggerOptions)options
                  usingBlock:(void (^)(NLTag tag, NSRange tokenRange, BOOL *stop))block
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    if (!block) {
        return;
    }
    BOOL stop = NO;
    for (NSValue *token in NLTaggerTokenRanges(_string, range, unit, options)) {
        block(nil, [token rangeValue], &stop);
        if (stop) {
            break;
        }
    }
}

@end
