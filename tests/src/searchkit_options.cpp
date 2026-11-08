// Copyright (C) 2026 Lubos Dolezel
//
// SKSearchOptions is an integer type and the kSKSearchOption constants are
// plain integer constants, so options combined with | can be passed and
// stored without a cast, in C++ as in C. With SKSearchOptions as an enum type
// this file does not compile.
#include <CoreServices/CoreServices.h>
#include <stdio.h>

static_assert(sizeof(SKSearchOptions) == sizeof(UInt32), "SKSearchOptions is 32 bits");

static SKSearchOptions combined(void)
{
	SKSearchOptions options = kSKSearchOptionNoRelevanceScores | kSKSearchOptionSpaceMeansOR;
	options |= kSKSearchOptionFindSimilar;
	return options;
}

int main(void)
{
	SKSearchOptions options = combined();
	int ok = options == 7 && kSKSearchOptionDefault == 0;

	printf("%s: kSKSearchOption values combine to %u\n", ok ? "PASS" : "FAIL", (unsigned) options);
	return ok ? 0 : 1;
}
