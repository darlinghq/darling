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

#import <Foundation/Foundation.h>
#import <NaturalLanguage/NLLanguage.h>

NS_ASSUME_NONNULL_BEGIN

// The kind of piece a tokenizer splits its string into.
typedef NS_ENUM(NSInteger, NLTokenUnit) {
    NLTokenUnitWord,
    NLTokenUnitSentence,
    NLTokenUnitParagraph,
    NLTokenUnitDocument,
};

typedef NS_OPTIONS(NSUInteger, NLTokenizerAttributes) {
    NLTokenizerAttributeNumeric = 1 << 0,
    NLTokenizerAttributeSymbolic = 1 << 1,
    NLTokenizerAttributeEmoji = 1 << 2,
};

// Splits a string into words, sentences or paragraphs.
@interface NLTokenizer : NSObject

- (instancetype)initWithUnit:(NLTokenUnit)unit NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;

@property (readonly) NLTokenUnit unit;
@property (nullable, retain) NSString *string;

- (void)setLanguage:(NLLanguage)language;

- (NSRange)tokenRangeAtIndex:(NSUInteger)characterIndex;
- (NSRange)tokenRangeForRange:(NSRange)range;
- (NSArray<NSValue *> *)tokensForRange:(NSRange)range;
- (void)enumerateTokensInRange:(NSRange)range usingBlock:(void (NS_NOESCAPE ^)(NSRange tokenRange, NLTokenizerAttributes flags, BOOL *stop))block;

@end

NS_ASSUME_NONNULL_END
