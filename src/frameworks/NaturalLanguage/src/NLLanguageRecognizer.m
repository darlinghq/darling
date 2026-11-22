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
// dominantLanguage is NLLanguageUndetermined and the hypotheses are empty. Hints and constraints are stored.
//
// GUESS: the documentation types the dominant language as optional and describes NLLanguageUndetermined as
// the identifier for a language the framework does not recognize; it does not say which of the two (nil or
// undetermined) is returned when nothing is recognized, and undetermined is used.
// GUESS: the documentation says reset returns the recognizer to its initial state but not whether that
// includes the hints and constraints; both are cleared, the state init gives them.
@implementation NLLanguageRecognizer

+ (NLLanguage)dominantLanguageForString:(NSString *)string
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return NLLanguageUndetermined;
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
    return NLLanguageUndetermined;
}

- (NSDictionary<NLLanguage, NSNumber *> *)languageHypothesesWithMaximum:(NSUInteger)maxHypotheses
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return [NSDictionary dictionary];
}

- (void)reset
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    self.languageHints = [NSDictionary dictionary];
    self.languageConstraints = [NSArray array];
}

@end
