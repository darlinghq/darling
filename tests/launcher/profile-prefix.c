#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/startup/profile-prefix.h"

static void expectPath(const char* explicitPrefix, const char* profile,
	const char* home, const char* expected)
{
	enum darling_prefix_error error;
	char* result = darlingSelectPrefix(explicitPrefix, profile, home, &error);
	assert(error == DARLING_PREFIX_OK);
	assert(result);
	assert(!strcmp(result, expected));
	free(result);
}

static void expectError(const char* explicitPrefix, const char* profile,
	const char* home, enum darling_prefix_error expected)
{
	enum darling_prefix_error error;
	char* result = darlingSelectPrefix(explicitPrefix, profile, home, &error);
	assert(!result);
	assert(error == expected);
}

int main(void)
{
	expectPath(NULL, NULL, "/home/tester", "/home/tester/.darling");
	expectPath("/tmp/diagnostic-prefix", NULL, "/ignored", "/tmp/diagnostic-prefix");
	expectPath(NULL, "ruby-3.3_test", "/home/tester", "/home/tester/.darling.ruby-3.3_test");
	expectError("/tmp/a", "ruby", "/home/tester", DARLING_PREFIX_CONFLICT);
	expectError("", NULL, "/home/tester", DARLING_PREFIX_EMPTY);
	expectError(NULL, "", "/home/tester", DARLING_PREFIX_INVALID_PROFILE);
	expectError(NULL, ".", "/home/tester", DARLING_PREFIX_INVALID_PROFILE);
	expectError(NULL, "..", "/home/tester", DARLING_PREFIX_INVALID_PROFILE);
	expectError(NULL, "../viewer", "/home/tester", DARLING_PREFIX_INVALID_PROFILE);
	expectError(NULL, "ruby/viewer", "/home/tester", DARLING_PREFIX_INVALID_PROFILE);
	expectError(NULL, "ruby viewer", "/home/tester", DARLING_PREFIX_INVALID_PROFILE);
	expectError(NULL, NULL, NULL, DARLING_PREFIX_NO_HOME);

	char longProfile[66];
	memset(longProfile, 'a', sizeof(longProfile) - 1);
	longProfile[sizeof(longProfile) - 1] = '\0';
	expectError(NULL, longProfile, "/home/tester", DARLING_PREFIX_INVALID_PROFILE);

	char longPrefix[257];
	memset(longPrefix, 'a', sizeof(longPrefix) - 1);
	longPrefix[sizeof(longPrefix) - 1] = '\0';
	expectError(longPrefix, NULL, "/home/tester", DARLING_PREFIX_TOO_LONG);

	puts("PASS: default, explicit, and named profile selection are isolated and invalid aliases are rejected");
	return 0;
}
