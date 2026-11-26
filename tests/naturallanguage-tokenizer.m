// NLTokenizer (NaturalLanguage, documented in Apple's NLTokenizer reference):
// word tokens are the words of the string, without the whitespace and
// punctuation between them; tokenRangeAtIndex: is the token holding the index.
#import <NaturalLanguage/NaturalLanguage.h>
#include <stdio.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s\n", #cond); failures++; } else printf("PASS: %s\n", #cond); } while (0)

int main(void) {
    @autoreleasepool {
        NLTokenizer *tokenizer = [[[NLTokenizer alloc] initWithUnit: NLTokenUnitWord] autorelease];
        CHECK(tokenizer.unit == NLTokenUnitWord);
        tokenizer.string = @"Hello, brave new world! 42";
        NSArray<NSValue *> *tokens = [tokenizer tokensForRange: NSMakeRange(0, tokenizer.string.length)];
        NSMutableArray<NSString *> *words = [NSMutableArray array];
        for (NSValue *token in tokens)
            [words addObject: [tokenizer.string substringWithRange: token.rangeValue]];
        CHECK([words isEqualToArray: (@[ @"Hello", @"brave", @"new", @"world", @"42" ])]);

        __block NLTokenizerAttributes numberFlags = 0;
        [tokenizer enumerateTokensInRange: NSMakeRange(24, 2) usingBlock: ^(NSRange range, NLTokenizerAttributes flags, BOOL *stop) {
            numberFlags = flags;
        }];
        CHECK(numberFlags & NLTokenizerAttributeNumeric);

        NSRange brave = [tokenizer tokenRangeAtIndex: 9];
        CHECK(NSEqualRanges(brave, NSMakeRange(7, 5)));
        NSRange span = [tokenizer tokenRangeForRange: NSMakeRange(8, 6)];
        CHECK(NSEqualRanges(span, NSMakeRange(7, 9)));

        __block NSUInteger count = 0;
        [tokenizer enumerateTokensInRange: NSMakeRange(0, tokenizer.string.length) usingBlock: ^(NSRange range, NLTokenizerAttributes flags, BOOL *stop) {
            if (++count == 2)
                *stop = YES;
        }];
        CHECK(count == 2);

        NLTokenizer *document = [[[NLTokenizer alloc] initWithUnit: NLTokenUnitDocument] autorelease];
        document.string = @"One. Two.";
        CHECK([document tokensForRange: NSMakeRange(0, 9)].count == 1);
    }
    printf("%s\n", failures ? "FAILED" : "ALL PASSED");
    return failures != 0;
}
