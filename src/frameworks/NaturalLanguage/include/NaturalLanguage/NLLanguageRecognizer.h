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

#ifndef _NLLANGUAGERECOGNIZER_H_
#define _NLLANGUAGERECOGNIZER_H_

#import <Foundation/Foundation.h>
#import <NaturalLanguage/NLLanguage.h>

NS_ASSUME_NONNULL_BEGIN

// The instance variables are declared here, and the properties are synthesized by name, because the
// i386 slice uses the legacy Objective-C runtime: no automatic property synthesis and no instance
// variables in @implementation.
API_AVAILABLE(macos(10.14))
@interface NLLanguageRecognizer : NSObject {
@private
    NSDictionary<NLLanguage, NSNumber *> *_languageHints;
    NSArray<NLLanguage> *_languageConstraints;
}

- (instancetype)init;

+ (nullable NLLanguage)dominantLanguageForString:(NSString *)string;

- (void)processString:(NSString *)string;
@property (nonatomic, copy, readonly, nullable) NLLanguage dominantLanguage;
- (NSDictionary<NLLanguage, NSNumber *> *)languageHypothesesWithMaximum:(NSUInteger)maxHypotheses;
- (void)reset;

@property (nonatomic, copy) NSDictionary<NLLanguage, NSNumber *> *languageHints;
@property (nonatomic, copy) NSArray<NLLanguage> *languageConstraints;

@end

NS_ASSUME_NONNULL_END

#endif
