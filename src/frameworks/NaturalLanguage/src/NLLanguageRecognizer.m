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

#import <NaturalLanguage/NLLanguageRecognizer.h>

// Placeholder: no language is recognized. The text is not analyzed and every result is "no language":
// dominantLanguage is nil and the hypotheses are empty. Hints and constraints are stored.
//
// Observed on macOS 26.7.3 (black-box observation through the public API; tests/src/naturallanguage_placeholder.m):
// a recognizer that found no language (a new one, an empty string, text without letters) answers nil, not
// NLLanguageUndetermined, and reset does not clear the hints or the constraints.
@implementation NLLanguageRecognizer

@synthesize languageHints = _languageHints;
@synthesize languageConstraints = _languageConstraints;

+ (NLLanguage)dominantLanguageForString:(NSString *)string
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return nil;
}

- (instancetype)init
{
    if ((self = [super init])) {
        _languageHints = [[NSDictionary alloc] init];
        _languageConstraints = [[NSArray alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [_languageHints release];
    [_languageConstraints release];
    [super dealloc];
}

- (void)processString:(NSString *)string
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
}

- (NLLanguage)dominantLanguage
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return nil;
}

- (NSDictionary<NLLanguage, NSNumber *> *)languageHypothesesWithMaximum:(NSUInteger)maxHypotheses
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return [NSDictionary dictionary];
}

- (void)reset
{
    // Nothing was analyzed, so there is no result to drop; the hints and constraints stay (see above).
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
}

@end
