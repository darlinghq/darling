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

// The documentation lists the cases in this order and gives no numeric values. The values are 0 to 3 in
// that order, as in the public headers of macOS 26.7.3's SDK (also the ABI of 10.14).
// Test: tests/src/naturallanguage_placeholder.m
typedef NS_ENUM(NSInteger, NLTokenUnit) {
    NLTokenUnitWord = 0,
    NLTokenUnitSentence = 1,
    NLTokenUnitParagraph = 2,
    NLTokenUnitDocument = 3,
} API_AVAILABLE(macos(10.14));

// The documentation lists the members alphabetically (emoji, numeric, symbolic) and states no bit values;
// that order is no evidence of them, and callers compile the values in. The values are the ones in the
// public headers of macOS 26.7.3's SDK: numeric 1, symbolic 2, emoji 4 (also the ABI of 10.14).
// Test: tests/src/naturallanguage_placeholder.m
typedef NS_OPTIONS(NSUInteger, NLTokenizerAttributes) {
    NLTokenizerAttributeNumeric = 1 << 0,
    NLTokenizerAttributeSymbolic = 1 << 1,
    NLTokenizerAttributeEmoji = 1 << 2,
} API_AVAILABLE(macos(10.14));

NS_ASSUME_NONNULL_BEGIN

// The instance variables are declared here, and the properties are synthesized by name, because the
// i386 slice uses the legacy Objective-C runtime: no automatic property synthesis and no instance
// variables in @implementation.
API_AVAILABLE(macos(10.14))
@interface NLTokenizer : NSObject {
@private
    NSString *_string;
    NLTokenUnit _unit;
}

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
