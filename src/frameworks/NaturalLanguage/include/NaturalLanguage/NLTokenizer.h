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

#ifndef _NLTOKENIZER_H_
#define _NLTOKENIZER_H_

#import <Foundation/Foundation.h>
#import <NaturalLanguage/NLLanguage.h>

// The documentation lists the cases in this order and gives no numeric values.
// GUESS: values 0 to 3 in the documented order word, sentence, paragraph, document.
typedef NS_ENUM(NSInteger, NLTokenUnit) {
    NLTokenUnitWord = 0,
    NLTokenUnitSentence = 1,
    NLTokenUnitParagraph = 2,
    NLTokenUnitDocument = 3,
} API_AVAILABLE(macos(10.14));

// The documentation lists the members alphabetically (emoji, numeric, symbolic) and states no bit values; that
// order is no evidence of them, and callers compile the values in.
// GUESS: one bit per member in the listed order, emoji first.
typedef NS_OPTIONS(NSUInteger, NLTokenizerAttributes) {
    NLTokenizerAttributeEmoji = 1 << 0,
    NLTokenizerAttributeNumeric = 1 << 1,
    NLTokenizerAttributeSymbolic = 1 << 2,
} API_AVAILABLE(macos(10.14));

NS_ASSUME_NONNULL_BEGIN

API_AVAILABLE(macos(10.14))
@interface NLTokenizer : NSObject

- (instancetype)initWithUnit:(NLTokenUnit)unit;

@property (nonatomic, retain, nullable) NSString *string;
@property (nonatomic, readonly) NLTokenUnit unit;

- (void)setLanguage:(NLLanguage)language;

- (void)enumerateTokensInRange:(NSRange)range
                    usingBlock:(void (^)(NSRange tokenRange, NLTokenizerAttributes flags, BOOL *stop))block;
- (NSArray<NSValue *> *)tokensForRange:(NSRange)range;
- (NSRange)tokenRangeAtIndex:(NSUInteger)characterIndex;
- (NSRange)tokenRangeForRange:(NSRange)range API_AVAILABLE(macos(11.0));

@end

NS_ASSUME_NONNULL_END

#endif
