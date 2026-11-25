// CFLAGS: -framework Foundation -framework NaturalLanguage
// Copyright (C) 2026 Darling developers
//
// NLTokenizer is built on CFStringTokenizer, so these expectations about the word, sentence and paragraph
// boundaries, the exclusion of whitespace, the numeric attribute and everything NLTagger derives from the
// tokenizer's tokens need the darling-corefoundation CFStringTokenizer fixes: the text kept alive for the
// break iterator, the paragraph unit, the word unit skipping whitespace and punctuation, and the numbers flag.
// Until the submodule pin includes them the word, paragraph and numeric cases are expected to fail. The
// placeholder behaviour that does not depend on those boundaries is in naturallanguage_placeholder.m. The
// handling of punctuation, the result when no token covers an index and the zero-length and out-of-range
// ranges are not documented: they are pinned to what macOS 26.7.3 reports for the same calls (black-box
// observation), see test_quirk_tokens_for_range_past_end and test_token_range_for_range_clamps below.
// Prints OK, or the failures with a non-zero exit status.
#import <Foundation/Foundation.h>
#import <NaturalLanguage/NaturalLanguage.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wnonnull"

static int failures;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } } while (0)

// The substrings of the tokens the tokenizer reports for the whole string.
static NSArray<NSString *> *tokenStrings(NLTokenUnit unit, NSString *string)
{
	NLTokenizer *tokenizer = [[NLTokenizer alloc] initWithUnit:unit];
	tokenizer.string = string;
	NSArray<NSValue *> *ranges = [tokenizer tokensForRange:NSMakeRange(0, [string length])];
	NSMutableArray<NSString *> *strings = [NSMutableArray array];
	NSUInteger end = 0;
	BOOL ordered = YES;
	for (NSValue *value in ranges) {
		NSRange range = [value rangeValue];
		if (range.length == 0 || range.location < end || NSMaxRange(range) > [string length]) {
			ordered = NO;
			continue;
		}
		end = NSMaxRange(range);
		[strings addObject:[string substringWithRange:range]];
	}
	CHECK(ordered, "tokens are not ordered, disjoint, non-empty and inside the string");
	[tokenizer release];
	return strings;
}

// QUIRK (see NLTokenizerTokensMeetingRange in src/NLTokenizer.m): tokensForRange: and
// enumerateTokensInRange:usingBlock: find no token when location + length (with wraparound) is beyond the
// end of the string, instead of cutting the range to the string. Expected counts, per unit
// (word, sentence, paragraph, document), are the ones macOS 26.7.3 returns for a 47 character string.
static void test_quirk_tokens_for_range_past_end(void)
{
	NSString *string = @"Hello, big world! It's 12.5 now.\nNew line here.";
	CHECK([string length] == 47, "the sample string is not 47 characters long");
	struct { NSUInteger location, length; int counts[4]; } cases[] = {
		{0, 47, {9, 3, 2, 1}},
		{0, 48, {0, 0, 0, 0}},
		{0, NSUIntegerMax, {0, 0, 0, 0}},
		{2, 45, {9, 3, 2, 1}},
		{2, 46, {0, 0, 0, 0}},
		{0, 0, {0, 0, 0, 1}},
		{46, 1, {0, 1, 1, 1}},
		{46, 0, {0, 1, 1, 1}},
		{47, 0, {0, 0, 0, 1}},
		{47, 1, {0, 0, 0, 0}},
		{44, 40, {0, 0, 0, 0}},
		{49, 3, {0, 0, 0, 0}},
		{1, NSUIntegerMax - 1, {0, 0, 0, 0}},
		{1, NSUIntegerMax, {0, 0, 0, 1}},
		{40, NSUIntegerMax, {1, 1, 1, 1}},
		{100, NSUIntegerMax, {0, 0, 0, 0}},
	};
	for (NLTokenUnit unit = NLTokenUnitWord; unit <= NLTokenUnitDocument; unit++) {
		NLTokenizer *tokenizer = [[NLTokenizer alloc] initWithUnit:unit];
		tokenizer.string = string;
		for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
			NSRange range = NSMakeRange(cases[i].location, cases[i].length);
			NSUInteger tokens = [[tokenizer tokensForRange:range] count];
			__block NSUInteger enumerated = 0;
			[tokenizer enumerateTokensInRange:range usingBlock:^(NSRange r, NLTokenizerAttributes f, BOOL *stop) { enumerated++; }];
			if (tokens != (NSUInteger)cases[i].counts[unit] || enumerated != tokens) {
				printf("FAIL: unit %d range {%lu, %lu}: %lu tokens, %lu enumerated, expected %d\n", (int)unit,
				       (unsigned long)range.location, (unsigned long)range.length, (unsigned long)tokens,
				       (unsigned long)enumerated, cases[i].counts[unit]);
				failures++;
			}
		}
		[tokenizer release];
	}
	// The tokens themselves for the sample text (word unit), and one range inside the string.
	NLTokenizer *words = [[NLTokenizer alloc] initWithUnit:NLTokenUnitWord];
	words.string = string;
	NSArray<NSValue *> *all = [words tokensForRange:NSMakeRange(0, 47)];
	NSRange expected[] = {{0, 5}, {7, 3}, {11, 5}, {18, 4}, {23, 4}, {28, 3}, {33, 3}, {37, 4}, {42, 4}};
	CHECK([all count] == 9, "word tokens of the sample text: wrong count");
	for (NSUInteger i = 0; i < [all count] && i < 9; i++) {
		CHECK(NSEqualRanges([[all objectAtIndex:i] rangeValue], expected[i]), "word tokens of the sample text: wrong range");
	}
	[words release];
}

// tokenRangeForRange: cuts a range that reaches past the end and unions the tokens it meets (macOS 26.7.3).
static void test_token_range_for_range_clamps(void)
{
	NSString *string = @"Hello, big world! It's 12.5 now.\nNew line here.";
	NLTokenizer *words = [[NLTokenizer alloc] initWithUnit:NLTokenUnitWord];
	words.string = string;
	CHECK(NSEqualRanges([words tokenRangeForRange:NSMakeRange(3, 6)], NSMakeRange(0, 10)), "tokenRangeForRange: {3, 6}");
	CHECK(NSEqualRanges([words tokenRangeForRange:NSMakeRange(0, NSUIntegerMax)], NSMakeRange(0, 46)), "tokenRangeForRange: {0, NSUIntegerMax}");
	CHECK([words tokenRangeForRange:NSMakeRange(5, 0)].location == NSNotFound, "tokenRangeForRange: empty range in the comma");
	CHECK([words tokenRangeForRange:NSMakeRange(49, 3)].location == NSNotFound, "tokenRangeForRange: range beyond the end");
	CHECK([words tokenRangeForRange:NSMakeRange(49, 0)].location == NSNotFound, "tokenRangeForRange: empty range beyond the end");
	CHECK(NSEqualRanges([words tokenRangeAtIndex:0], NSMakeRange(0, 5)), "tokenRangeAtIndex:0");
	CHECK(NSEqualRanges([words tokenRangeAtIndex:7], NSMakeRange(7, 3)), "tokenRangeAtIndex:7");
	CHECK([words tokenRangeAtIndex:5].location == NSNotFound, "tokenRangeAtIndex: in the comma");
	CHECK([words tokenRangeAtIndex:47].location == NSNotFound, "tokenRangeAtIndex: at the end");
	CHECK([words tokenRangeAtIndex:52].location == NSNotFound, "tokenRangeAtIndex: beyond the end");
	[words release];
	NLTokenizer *sentences = [[NLTokenizer alloc] initWithUnit:NLTokenUnitSentence];
	sentences.string = string;
	CHECK(NSEqualRanges([sentences tokenRangeForRange:NSMakeRange(5, 0)], NSMakeRange(0, 18)), "sentence tokenRangeForRange: empty range");
	CHECK(NSEqualRanges([sentences tokenRangeForRange:NSMakeRange(3, 6)], NSMakeRange(0, 18)), "sentence tokenRangeForRange: {3, 6}");
	CHECK(NSEqualRanges([sentences tokenRangeForRange:NSMakeRange(0, NSUIntegerMax)], NSMakeRange(0, 47)), "sentence tokenRangeForRange: {0, NSUIntegerMax}");
	[sentences release];
	NLTokenizer *empty = [[NLTokenizer alloc] initWithUnit:NLTokenUnitWord];
	CHECK([empty tokenRangeAtIndex:0].location == NSNotFound, "tokenRangeAtIndex: with no string");
	empty.string = @"";
	CHECK([empty tokenRangeAtIndex:0].location == NSNotFound, "tokenRangeAtIndex: with an empty string");
	CHECK([[empty tokensForRange:NSMakeRange(0, 0)] count] == 0, "tokensForRange: of an empty string");
	[empty release];
}

int main(void)
{
	@autoreleasepool {
		test_quirk_tokens_for_range_past_end();
		test_token_range_for_range_clamps();

		// Word unit: whitespace separates tokens and is not a token.
		NSArray<NSString *> *words = tokenStrings(NLTokenUnitWord, @"Hello big world 42");
		CHECK([words containsObject:@"Hello"], "word unit: Hello missing");
		CHECK([words containsObject:@"big"], "word unit: big missing");
		CHECK([words containsObject:@"world"], "word unit: world missing");
		CHECK([words containsObject:@"42"], "word unit: 42 missing");
		for (NSString *word in words) {
			CHECK([[word stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] length] > 0, "word unit: a token is whitespace");
		}
		NSArray<NSString *> *unicode = tokenStrings(NLTokenUnitWord, @"Grüße aus München");
		CHECK([unicode containsObject:@"Grüße"], "word unit: non-ASCII word missing");
		CHECK([unicode containsObject:@"München"], "word unit: non-ASCII word missing (2)");
		CHECK([tokenStrings(NLTokenUnitWord, @"   ") count] == 0, "word unit: whitespace-only string has tokens");
		// Punctuation handling is not documented; tokenStrings still checks that the tokens are well formed.
		(void)tokenStrings(NLTokenUnitWord, @"Hello, world! (42) .,!?");

		NSArray<NSString *> *sentences = tokenStrings(NLTokenUnitSentence, @"First sentence. Second sentence.");
		CHECK([sentences count] == 2, "sentence unit: two sentences expected");

		// A sentence rule would give three tokens here: it also breaks after "A." and "B.".
		NSArray<NSString *> *paragraphs = tokenStrings(NLTokenUnitParagraph, @"A. B.\nC.");
		CHECK([paragraphs count] == 2, "paragraph unit: two paragraphs expected for sentences split across a line");
		NSArray<NSString *> *plainParagraphs = tokenStrings(NLTokenUnitParagraph, @"one paragraph\ntwo paragraph");
		CHECK([plainParagraphs count] == 2, "paragraph unit: two paragraphs expected");

		// Range queries on "Hello world".
		NLTokenizer *tokenizer = [[NLTokenizer alloc] initWithUnit:NLTokenUnitWord];
		tokenizer.string = @"Hello world";
		NSRange at = [tokenizer tokenRangeAtIndex:1];
		CHECK(at.location == 0 && at.length == 5, "tokenRangeAtIndex: wrong range");
		NSRange zero = [tokenizer tokenRangeForRange:NSMakeRange(8, 0)];
		NSRange zeroAt = [tokenizer tokenRangeAtIndex:8];
		CHECK(zero.location == 6 && zero.length == 5, "tokenRangeForRange: zero-length range wrong");
		CHECK(NSEqualRanges(zero, zeroAt), "zero-length tokenRangeForRange: differs from tokenRangeAtIndex:");
		NSRange both = [tokenizer tokenRangeForRange:NSMakeRange(3, 5)];
		CHECK(both.location == 0 && both.length == 11, "tokenRangeForRange: partial overlap does not widen to whole tokens");
		NSArray<NSValue *> *some = [tokenizer tokensForRange:NSMakeRange(7, 2)];
		CHECK([some count] == 1 && NSEqualRanges([[some objectAtIndex:0] rangeValue], NSMakeRange(6, 5)), "tokensForRange: partial overlap");

		__block int calls = 0;
		[tokenizer enumerateTokensInRange:NSMakeRange(0, 11) usingBlock:^(NSRange range, NLTokenizerAttributes flags, BOOL *stop) {
			calls++;
			*stop = YES;
		}];
		CHECK(calls == 1, "enumerate did not stop after the block set *stop");
		calls = 0;
		[tokenizer enumerateTokensInRange:NSMakeRange(0, 11) usingBlock:^(NSRange range, NLTokenizerAttributes flags, BOOL *stop) {
			calls++;
		}];
		CHECK(calls == 2, "enumerate over two words did not call the block twice");

		// Numeric attribute from a token with digits.
		__block BOOL sawNumeric = NO;
		tokenizer.string = @"abc 123";
		[tokenizer enumerateTokensInRange:NSMakeRange(0, 7) usingBlock:^(NSRange range, NLTokenizerAttributes flags, BOOL *stop) {
			if (range.location == 4 && (flags & NLTokenizerAttributeNumeric)) {
				sawNumeric = YES;
			}
		}];
		CHECK(sawNumeric, "the token 123 is not flagged numeric");

		// Ranges that reach past the end find no token (macOS 26.7.3, QUIRK above), and do not raise.
		tokenizer.string = @"Hello world";
		CHECK([[tokenizer tokensForRange:NSMakeRange(4, 100)] count] == 0, "tokensForRange: reaching past the end");
		CHECK([[tokenizer tokensForRange:NSMakeRange(0, NSUIntegerMax)] count] == 0, "tokensForRange: with length NSUIntegerMax");
		CHECK([[tokenizer tokensForRange:NSMakeRange(1, NSUIntegerMax)] count] == 0, "tokensForRange: with location 1 and length NSUIntegerMax");
		[tokenizer release];

		// NLTagger: the tokens are the tokenizer's tokens for the unit, each reported with no tag.
		NLTagger *tagger = [[NLTagger alloc] initWithTagSchemes:@[NLTagSchemeLexicalClass]];
		tagger.string = @"The quick brown fox";
		// Every token that meets the range is reported with no tag; the tokens are the tokenizer's for the unit.
		NLTokenizer *reference = [[NLTokenizer alloc] initWithUnit:NLTokenUnitWord];
		reference.string = tagger.string;
		NSArray<NSValue *> *expected = [reference tokensForRange:NSMakeRange(0, 19)];
		NSArray<NSValue *> *tokenRanges = nil;
		NSArray<NLTag> *tags = [tagger tagsInRange:NSMakeRange(0, 19) unit:NLTokenUnitWord scheme:NLTagSchemeLexicalClass
		                                   options:0 tokenRanges:&tokenRanges];
		CHECK(tags != nil && tokenRanges != nil && [tags count] == [tokenRanges count], "tagsInRange: tags and tokenRanges differ in count");
		CHECK([tokenRanges isEqualToArray:expected], "tagsInRange: tokenRanges are not the tokenizer's tokens");
		for (NLTag entry in tags) {
			CHECK([entry isEqualToString:@""], "tagsInRange: the entry for a token without a tag is not an empty string");
		}
		CHECK([[tagger tagsInRange:NSMakeRange(0, 19) unit:NLTokenUnitWord scheme:NLTagSchemeLexicalClass options:0 tokenRanges:NULL] count] == [expected count], "tagsInRange: with NULL tokenRanges");
		__block NSMutableArray<NSValue *> *seen = [NSMutableArray array];
		__block BOOL allNil = YES;
		[tagger enumerateTagsInRange:NSMakeRange(0, 19) unit:NLTokenUnitWord scheme:NLTagSchemeLexicalClass options:0
		                  usingBlock:^(NLTag t, NSRange range, BOOL *stop) {
			allNil = allNil && t == nil;
			[seen addObject:[NSValue valueWithRange:range]];
		}];
		CHECK(allNil, "enumerateTagsInRange: passed a tag");
		CHECK([seen isEqualToArray:expected], "enumerateTagsInRange: did not visit the tokenizer's tokens");
		__block int tagCalls = 0;
		[tagger enumerateTagsInRange:NSMakeRange(0, 19) unit:NLTokenUnitWord scheme:NLTagSchemeLexicalClass options:0
		                  usingBlock:^(NLTag t, NSRange range, BOOL *stop) {
			tagCalls++;
			*stop = YES;
		}];
		CHECK(tagCalls == ([expected count] > 0 ? 1 : 0), "enumerateTagsInRange: did not stop after *stop was set");
		[tagger enumerateTagsInRange:NSMakeRange(0, 19) unit:NLTokenUnitWord scheme:NLTagSchemeLexicalClass options:0 usingBlock:nil];

		// The omit options leave no whitespace-only and no punctuation-only word token. How a token is classified
		// is a GUESS (a token made only of spaces is whitespace under any reading; one made only of "," or "!"
		// is taken as punctuation).
		tagger.string = @"Hello, big world !";
		NSArray<NSValue *> *kept = nil;
		(void)[tagger tagsInRange:NSMakeRange(0, 18) unit:NLTokenUnitWord scheme:NLTagSchemeLexicalClass
		                  options:NLTaggerOmitWhitespace | NLTaggerOmitPunctuation tokenRanges:&kept];
		for (NSValue *value in kept) {
			NSString *token = [tagger.string substringWithRange:[value rangeValue]];
			CHECK([[token stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] length] > 0, "omitWhitespace left a whitespace token");
			CHECK([[token stringByTrimmingCharactersInSet:[NSCharacterSet punctuationCharacterSet]] length] > 0, "omitPunctuation left a punctuation token");
		}

		// The token range methods ask a tokenizer for the same unit.
		tagger.string = @"Hello world";
		NLTokenizer *sentenceReference = [[NLTokenizer alloc] initWithUnit:NLTokenUnitSentence];
		sentenceReference.string = tagger.string;
		CHECK(NSEqualRanges([tagger tokenRangeAtIndex:2 unit:NLTokenUnitSentence], [sentenceReference tokenRangeAtIndex:2]), "tagger tokenRangeAtIndex:unit:");
		CHECK(NSEqualRanges([tagger tokenRangeForRange:NSMakeRange(2, 3) unit:NLTokenUnitSentence], [sentenceReference tokenRangeForRange:NSMakeRange(2, 3)]), "tagger tokenRangeForRange:unit:");
		[sentenceReference release];
		[reference release];
		[tagger release];
	}
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
