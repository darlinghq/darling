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

#ifndef _NLTAGGER_H_
#define _NLTAGGER_H_

#import <Foundation/Foundation.h>
#import <NaturalLanguage/NLLanguage.h>
#import <NaturalLanguage/NLTokenizer.h>

NS_ASSUME_NONNULL_BEGIN

typedef NSString *NLTag API_AVAILABLE(macos(10.14));
typedef NSString *NLTagScheme API_AVAILABLE(macos(10.14));

extern NLTagScheme const NLTagSchemeLexicalClass API_AVAILABLE(macos(10.14));
extern NLTagScheme const NLTagSchemeNameTypeOrLexicalClass API_AVAILABLE(macos(10.14));

extern NLTag const NLTagNoun API_AVAILABLE(macos(10.14));
extern NLTag const NLTagVerb API_AVAILABLE(macos(10.14));
extern NLTag const NLTagAdjective API_AVAILABLE(macos(10.14));
extern NLTag const NLTagNumber API_AVAILABLE(macos(10.14));
extern NLTag const NLTagPersonalName API_AVAILABLE(macos(10.14));
extern NLTag const NLTagOrganizationName API_AVAILABLE(macos(10.14));
extern NLTag const NLTagPlaceName API_AVAILABLE(macos(10.14));

// The documentation lists the options in this order and gives no numeric values.
// GUESS: one bit per option in the documented order, omitWords first, joinContractions last.
typedef NS_OPTIONS(NSUInteger, NLTaggerOptions) {
    NLTaggerOmitWords = 1 << 0,
    NLTaggerOmitPunctuation = 1 << 1,
    NLTaggerOmitWhitespace = 1 << 2,
    NLTaggerOmitOther = 1 << 3,
    NLTaggerJoinNames = 1 << 4,
    NLTaggerJoinContractions = 1 << 5,
} API_AVAILABLE(macos(10.14));

// The instance variables are declared here, and the properties are synthesized by name, because the
// i386 slice uses the legacy Objective-C runtime: no automatic property synthesis and no instance
// variables in @implementation.
API_AVAILABLE(macos(10.14))
@interface NLTagger : NSObject {
@private
    NSString *_string;
    NSArray<NLTagScheme> *_tagSchemes;
}

- (instancetype)initWithTagSchemes:(NSArray<NLTagScheme> *)tagSchemes;

@property (nonatomic, copy, readonly) NSArray<NLTagScheme> *tagSchemes;
@property (nonatomic, retain, nullable) NSString *string;
@property (nonatomic, copy, readonly, nullable) NLLanguage dominantLanguage;

+ (NSArray<NLTagScheme> *)availableTagSchemesForUnit:(NLTokenUnit)unit language:(NLLanguage)language;

- (void)setLanguage:(NLLanguage)language range:(NSRange)range;

- (nullable NLTag)tagAtIndex:(NSUInteger)characterIndex
                        unit:(NLTokenUnit)unit
                      scheme:(NLTagScheme)scheme
                  tokenRange:(nullable NSRangePointer)tokenRange;
- (NSArray<NLTag> *)tagsInRange:(NSRange)range
                           unit:(NLTokenUnit)unit
                         scheme:(NLTagScheme)scheme
                        options:(NLTaggerOptions)options
                    tokenRanges:(NSArray<NSValue *> *_Nullable *_Nullable)tokenRanges;
- (NSRange)tokenRangeAtIndex:(NSUInteger)characterIndex unit:(NLTokenUnit)unit;
- (NSRange)tokenRangeForRange:(NSRange)range unit:(NLTokenUnit)unit API_AVAILABLE(macos(11.0));
- (void)enumerateTagsInRange:(NSRange)range
                        unit:(NLTokenUnit)unit
                      scheme:(NLTagScheme)scheme
                     options:(NLTaggerOptions)options
                  usingBlock:(void (^)(NLTag _Nullable tag, NSRange tokenRange, BOOL *stop))block;

@end

NS_ASSUME_NONNULL_END

#endif
