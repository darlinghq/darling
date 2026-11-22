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

#import <NaturalLanguage/NLEmbedding.h>

// Placeholder: no embedding is available. Every factory returns nil, which the documentation gives as the
// result when no embedding is available for the language. An instance can only come from alloc and init, and
// describes an empty embedding: no dimension, no vocabulary, no vectors and no neighbors.
@implementation NLEmbedding

@synthesize dimension = _dimension;
@synthesize vocabularySize = _vocabularySize;
@synthesize language = _language;
@synthesize revision = _revision;

- (void)dealloc
{
    [_language release];
    [super dealloc];
}

+ (NLEmbedding *)wordEmbeddingForLanguage:(NLLanguage)language
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return nil;
}

+ (NLEmbedding *)wordEmbeddingForLanguage:(NLLanguage)language revision:(NSUInteger)revision
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return nil;
}

+ (NLEmbedding *)sentenceEmbeddingForLanguage:(NLLanguage)language
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return nil;
}

+ (NLEmbedding *)sentenceEmbeddingForLanguage:(NLLanguage)language revision:(NSUInteger)revision
{
    NSLog(@"Stub called: %@ in %@", NSStringFromSelector(_cmd), [self class]);
    return nil;
}

- (BOOL)containsString:(NSString *)string
{
    return NO;
}

- (NSArray<NSNumber *> *)vectorForString:(NSString *)string
{
    return nil;
}

- (NSArray<NSString *> *)neighborsForString:(NSString *)string
                               maximumCount:(NSUInteger)maxCount
                               distanceType:(NLDistanceType)distanceType
{
    return [NSArray array];
}

@end
