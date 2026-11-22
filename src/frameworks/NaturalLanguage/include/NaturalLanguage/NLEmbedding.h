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

#ifndef _NLEMBEDDING_H_
#define _NLEMBEDDING_H_

#import <Foundation/Foundation.h>
#import <NaturalLanguage/NLLanguage.h>

typedef double NLDistance API_AVAILABLE(macos(10.14));

// The documentation names a single case and gives no numeric value.
// GUESS: cosine is 0.
typedef NS_ENUM(NSInteger, NLDistanceType) {
    NLDistanceTypeCosine = 0,
} API_AVAILABLE(macos(10.14));

NS_ASSUME_NONNULL_BEGIN

// The instance variables are declared here, and the properties are synthesized by name, because the
// i386 slice uses the legacy Objective-C runtime: no automatic property synthesis and no instance
// variables in @implementation.
API_AVAILABLE(macos(10.15))
@interface NLEmbedding : NSObject {
@private
    NSUInteger _dimension;
    NSUInteger _vocabularySize;
    NLLanguage _language;
    NSUInteger _revision;
}

+ (nullable NLEmbedding *)wordEmbeddingForLanguage:(NLLanguage)language;
+ (nullable NLEmbedding *)wordEmbeddingForLanguage:(NLLanguage)language revision:(NSUInteger)revision;
+ (nullable NLEmbedding *)sentenceEmbeddingForLanguage:(NLLanguage)language API_AVAILABLE(macos(11.0));
+ (nullable NLEmbedding *)sentenceEmbeddingForLanguage:(NLLanguage)language revision:(NSUInteger)revision API_AVAILABLE(macos(11.0));

@property (readonly) NSUInteger dimension;
@property (readonly) NSUInteger vocabularySize;
@property (copy, readonly, nullable) NLLanguage language;
@property (readonly) NSUInteger revision;

- (BOOL)containsString:(NSString *)string;
- (nullable NSArray<NSNumber *> *)vectorForString:(NSString *)string;
- (NSArray<NSString *> *)neighborsForString:(NSString *)string
                               maximumCount:(NSUInteger)maxCount
                               distanceType:(NLDistanceType)distanceType;

@end

NS_ASSUME_NONNULL_END

#endif
