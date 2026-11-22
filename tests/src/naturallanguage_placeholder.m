// CFLAGS: -framework Foundation -framework NaturalLanguage
// Copyright (C) 2026 Darling developers
//
// NaturalLanguage is a placeholder framework. NLTagger reports its tokens with no tags, NLLanguageRecognizer
// recognizes no language (nil, as macOS 26.7.3 does when it finds none), the NLEmbedding factories return nil, and NLTokenizer is built on
// CFStringTokenizer. This test checks only what does not depend on CFStringTokenizer's word, sentence and
// paragraph boundaries: the classes, the constants, the document unit, empty and nil strings, ranges that need
// no tokenizing, and the placeholder results. The boundary expectations are in naturallanguage_tokenizer.m.
// The constant and enum values are not documented; they are pinned to what macOS 26.7.3 and 10.14.4 report
// (black-box observation). Prints OK, or the failures with a non-zero exit status.
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

int main(void)
{
	@autoreleasepool {
		CHECK([NLTokenizer class] != Nil, "NLTokenizer is missing");
		CHECK([NLTagger class] != Nil, "NLTagger is missing");
		CHECK([NLLanguageRecognizer class] != Nil, "NLLanguageRecognizer is missing");
		CHECK([NLEmbedding class] != Nil, "NLEmbedding is missing");
		CHECK([NLTokenizer superclass] == [NSObject class], "NLTokenizer is not an NSObject subclass");
		CHECK([NLTagger superclass] == [NSObject class], "NLTagger is not an NSObject subclass");
		CHECK([NLLanguageRecognizer superclass] == [NSObject class], "NLLanguageRecognizer is not an NSObject subclass");
		CHECK([NLEmbedding superclass] == [NSObject class], "NLEmbedding is not an NSObject subclass");

		// The values are not documented: only presence is checked.
		CHECK(NLLanguageEnglish != nil, "NLLanguageEnglish is nil");
		CHECK(NLLanguageUndetermined != nil, "NLLanguageUndetermined is nil");
		// The values macOS 26.7.3 and 10.14.4 report for the constants and the enums.
		CHECK([NLLanguageEnglish isEqualToString:@"en"] && [NLLanguageUndetermined isEqualToString:@"und"], "language strings");
		CHECK([NLTagSchemeLexicalClass isEqualToString:@"LexicalClass"] &&
		      [NLTagSchemeNameTypeOrLexicalClass isEqualToString:@"NameTypeOrLexicalClass"], "tag scheme strings");
		CHECK([NLTagNoun isEqualToString:@"Noun"] && [NLTagVerb isEqualToString:@"Verb"] &&
		      [NLTagAdjective isEqualToString:@"Adjective"] && [NLTagNumber isEqualToString:@"Number"] &&
		      [NLTagPersonalName isEqualToString:@"PersonalName"] &&
		      [NLTagOrganizationName isEqualToString:@"OrganizationName"] &&
		      [NLTagPlaceName isEqualToString:@"PlaceName"], "tag strings");
		CHECK(NLTokenUnitWord == 0 && NLTokenUnitSentence == 1 && NLTokenUnitParagraph == 2 && NLTokenUnitDocument == 3, "NLTokenUnit values");
		CHECK(NLTokenizerAttributeNumeric == 1 && NLTokenizerAttributeSymbolic == 2 && NLTokenizerAttributeEmoji == 4, "NLTokenizerAttributes values");
		CHECK(NLTaggerOmitWords == 1 && NLTaggerOmitPunctuation == 2 && NLTaggerOmitWhitespace == 4 &&
		      NLTaggerOmitOther == 8 && NLTaggerJoinNames == 16 && NLTaggerJoinContractions == 32, "NLTaggerOptions values");
		CHECK(NLTagSchemeLexicalClass != nil, "NLTagSchemeLexicalClass is nil");
		CHECK(NLTagSchemeNameTypeOrLexicalClass != nil, "NLTagSchemeNameTypeOrLexicalClass is nil");
		CHECK(NLTagNoun != nil, "NLTagNoun is nil");
		CHECK(NLTagVerb != nil, "NLTagVerb is nil");
		CHECK(NLTagAdjective != nil, "NLTagAdjective is nil");
		CHECK(NLTagNumber != nil, "NLTagNumber is nil");
		CHECK(NLTagPersonalName != nil, "NLTagPersonalName is nil");
		CHECK(NLTagOrganizationName != nil, "NLTagOrganizationName is nil");
		CHECK(NLTagPlaceName != nil, "NLTagPlaceName is nil");
		CHECK(![NLLanguageEnglish isEqualToString:NLLanguageUndetermined], "English equals undetermined");
		CHECK(![NLTagNoun isEqualToString:NLTagVerb], "noun equals verb");

		// An empty string has no tokens in any unit, and the document unit is the whole string.
		CHECK([tokenStrings(NLTokenUnitWord, @"") count] == 0, "word unit: empty string has tokens");
		CHECK([tokenStrings(NLTokenUnitSentence, @"") count] == 0, "sentence unit: empty string has tokens");
		CHECK([tokenStrings(NLTokenUnitParagraph, @"") count] == 0, "paragraph unit: empty string has tokens");
		NSString *text = @"Hello world. Bye";
		NSArray<NSString *> *document = tokenStrings(NLTokenUnitDocument, text);
		CHECK([document count] == 1 && [[document objectAtIndex:0] isEqualToString:text], "document unit is not the whole string");
		CHECK([tokenStrings(NLTokenUnitDocument, @"") count] == 0, "document unit: empty string has tokens");

		// Properties, and the strings and ranges that need no tokenizing.
		NLTokenizer *tokenizer = [[NLTokenizer alloc] initWithUnit:NLTokenUnitWord];
		CHECK(tokenizer.unit == NLTokenUnitWord, "unit not stored");
		CHECK(tokenizer.string == nil, "string is not nil initially");
		[tokenizer setLanguage:NLLanguageEnglish];

		// Not documented, only checked not to crash: no string, nil block, ranges outside the string.
		tokenizer.string = nil;
		CHECK([[tokenizer tokensForRange:NSMakeRange(0, 5)] count] == 0, "nil string has tokens");
		(void)[tokenizer tokenRangeAtIndex:0];
		(void)[tokenizer tokenRangeForRange:NSMakeRange(0, 5)];
		tokenizer.string = @"Hello world";
		[tokenizer enumerateTokensInRange:NSMakeRange(0, 5) usingBlock:nil];
		(void)[tokenizer tokensForRange:NSMakeRange(100, 5)];
		(void)[tokenizer tokenRangeAtIndex:100];
		(void)[tokenizer tokenRangeAtIndex:NSUIntegerMax];
		[tokenizer release];

		// The document unit is one token. As for the other units, macOS 26.7.3 finds nothing for a range
		// that ends beyond the string ({0, NSUIntegerMax}), while {1, NSUIntegerMax} (location + length
		// wraps around to 0) selects the token (black-box observation, see NLTokenizerTokensMeetingRange).
		NLTokenizer *docTokenizer = [[NLTokenizer alloc] initWithUnit:NLTokenUnitDocument];
		docTokenizer.string = @"Hello world";
		CHECK([[docTokenizer tokensForRange:NSMakeRange(0, 11)] count] == 1, "document unit: one token expected");
		CHECK([[docTokenizer tokensForRange:NSMakeRange(0, NSUIntegerMax)] count] == 0, "document unit: tokensForRange: with length NSUIntegerMax");
		CHECK([[docTokenizer tokensForRange:NSMakeRange(1, NSUIntegerMax)] count] == 1, "document unit: tokensForRange: with location 1 and length NSUIntegerMax");
		CHECK([[docTokenizer tokensForRange:NSMakeRange(100, NSUIntegerMax)] count] == 0, "document unit: a range beyond the string has tokens");
		(void)[docTokenizer tokenRangeAtIndex:NSUIntegerMax];
		(void)[docTokenizer tokenRangeForRange:NSMakeRange(1, NSUIntegerMax)];
		[docTokenizer release];

		// NLTagger: no tags.
		NLTagger *tagger = [[NLTagger alloc] initWithTagSchemes:@[NLTagSchemeLexicalClass]];
		CHECK([tagger.tagSchemes isEqualToArray:@[NLTagSchemeLexicalClass]], "tagSchemes not stored");
		tagger.string = @"The quick brown fox";
		CHECK([tagger.string isEqualToString:@"The quick brown fox"], "tagger string not stored");
		NSRange tokenRange = NSMakeRange(7, 7);
		NLTag tag = [tagger tagAtIndex:1 unit:NLTokenUnitWord scheme:NLTagSchemeLexicalClass tokenRange:&tokenRange];
		CHECK(tag == nil, "tagAtIndex: returned a tag");
		CHECK(tokenRange.location == 7 && tokenRange.length == 7, "tagAtIndex: changed tokenRange although it returned no tag");
		CHECK([tagger tagAtIndex:1 unit:NLTokenUnitWord scheme:NLTagSchemeLexicalClass tokenRange:NULL] == nil, "tagAtIndex: with NULL tokenRange returned a tag");
		NSArray<NSValue *> *docTokens = nil;
		NSArray<NLTag> *docTags = [tagger tagsInRange:NSMakeRange(0, 19) unit:NLTokenUnitDocument scheme:NLTagSchemeLexicalClass options:0 tokenRanges:&docTokens];
		CHECK([docTags count] == 1 && [[docTags objectAtIndex:0] isEqualToString:@""], "tagsInRange: the entry for a token without a tag is not an empty string");
		CHECK([docTokens count] == 1 && NSEqualRanges([[docTokens objectAtIndex:0] rangeValue], NSMakeRange(0, 19)), "document unit tokens for the tagger");
		CHECK([[tagger tagsInRange:NSMakeRange(0, 19) unit:NLTokenUnitDocument scheme:NLTagSchemeLexicalClass options:0 tokenRanges:NULL] count] == 1, "tagsInRange: with NULL tokenRanges");
		__block int tagCalls = 0;
		__block BOOL allNil = YES;
		[tagger enumerateTagsInRange:NSMakeRange(0, 19) unit:NLTokenUnitDocument scheme:NLTagSchemeLexicalClass options:0
		                  usingBlock:^(NLTag t, NSRange range, BOOL *stop) {
			tagCalls++;
			allNil = allNil && t == nil && NSEqualRanges(range, NSMakeRange(0, 19));
			*stop = YES;
		}];
		CHECK(tagCalls == 1 && allNil, "enumerateTagsInRange: for the document unit");
		[tagger enumerateTagsInRange:NSMakeRange(0, 19) unit:NLTokenUnitDocument scheme:NLTagSchemeLexicalClass options:0 usingBlock:nil];
		CHECK([[tagger tagsInRange:NSMakeRange(100, 5) unit:NLTokenUnitDocument scheme:NLTagSchemeLexicalClass options:0 tokenRanges:NULL] count] == 0, "tagsInRange: beyond the string has entries");
		CHECK(NSEqualRanges([tagger tokenRangeAtIndex:3 unit:NLTokenUnitDocument], NSMakeRange(0, 19)), "tagger tokenRangeAtIndex:unit: for the document unit");
		CHECK([tagger.dominantLanguage isEqualToString:NLLanguageUndetermined], "tagger dominantLanguage is not undetermined");
		[tagger setLanguage:NLLanguageEnglish range:NSMakeRange(0, 19)];
		tagger.string = nil;
		CHECK([NLTagger availableTagSchemesForUnit:NLTokenUnitWord language:NLLanguageEnglish] != nil, "availableTagSchemes is nil");
		[tagger release];
		NLTagger *empty = [[NLTagger alloc] initWithTagSchemes:@[]];
		CHECK([empty tagAtIndex:0 unit:NLTokenUnitWord scheme:NLTagSchemeNameTypeOrLexicalClass tokenRange:NULL] == nil, "empty tagger returned a tag");
		[empty release];

		// NLLanguageRecognizer: no language.
		// macOS 26.7.3 answers nil, not NLLanguageUndetermined, when no language is recognized (an empty
		// string, text without letters, a new recognizer); the placeholder recognizes nothing.
		CHECK([NLLanguageRecognizer dominantLanguageForString:@"This is English text."] == nil, "dominantLanguageForString: is not nil");
		CHECK([NLLanguageRecognizer dominantLanguageForString:@""] == nil, "dominantLanguageForString: of an empty string is not nil");
		CHECK([NLLanguageRecognizer dominantLanguageForString:@"12345 !!!"] == nil, "dominantLanguageForString: of digits and punctuation is not nil");
		CHECK([NLLanguageRecognizer dominantLanguageForString:nil] == nil, "dominantLanguageForString: of nil is not nil");
		NLLanguageRecognizer *recognizer = [[NLLanguageRecognizer alloc] init];
		CHECK([recognizer.languageHints count] == 0, "languageHints not empty initially");
		CHECK([recognizer.languageConstraints count] == 0, "languageConstraints not empty initially");
		[recognizer processString:@"Bonjour tout le monde"];
		[recognizer processString:nil];
		CHECK(recognizer.dominantLanguage == nil, "dominantLanguage is not nil");
		NSDictionary<NLLanguage, NSNumber *> *hypotheses = [recognizer languageHypothesesWithMaximum:3];
		CHECK(hypotheses != nil && [hypotheses count] == 0, "languageHypotheses is not an empty dictionary");
		recognizer.languageHints = @{NLLanguageEnglish: @0.5};
		recognizer.languageConstraints = @[NLLanguageEnglish];
		CHECK([recognizer.languageHints count] == 1, "languageHints not stored");
		CHECK([recognizer.languageConstraints isEqualToArray:@[NLLanguageEnglish]], "languageConstraints not stored");
		[recognizer reset];
		CHECK(recognizer.dominantLanguage == nil, "dominantLanguage is not nil after reset");
		// reset keeps the hints and the constraints (observed on macOS 26.7.3).
		CHECK([recognizer.languageHints count] == 1, "reset cleared the languageHints");
		CHECK([recognizer.languageConstraints isEqualToArray:@[NLLanguageEnglish]], "reset cleared the languageConstraints");
		[recognizer release];

		// NLEmbedding: no embedding is available.
		CHECK([NLEmbedding wordEmbeddingForLanguage:NLLanguageEnglish] == nil, "wordEmbedding is not nil");
		CHECK([NLEmbedding wordEmbeddingForLanguage:NLLanguageEnglish revision:1] == nil, "wordEmbedding with revision is not nil");
		CHECK([NLEmbedding sentenceEmbeddingForLanguage:NLLanguageEnglish] == nil, "sentenceEmbedding is not nil");
		CHECK([NLEmbedding sentenceEmbeddingForLanguage:NLLanguageEnglish revision:1] == nil, "sentenceEmbedding with revision is not nil");
		CHECK([NLEmbedding wordEmbeddingForLanguage:nil] == nil, "wordEmbedding for nil is not nil");
		CHECK([NLEmbedding sentenceEmbeddingForLanguage:nil] == nil, "sentenceEmbedding for nil is not nil");
	}
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
