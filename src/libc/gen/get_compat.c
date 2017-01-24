/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>

/* Catastrophic errors only, including "out of memory" */
static bool parse_error = false;
static bool bootstrapping = false;
static bool parsed = false;
#define UNIX2003_DEFAULT_MODE true
static bool unix2003_mode = UNIX2003_DEFAULT_MODE;

static pthread_once_t threadsafe = PTHREAD_ONCE_INIT;

/* There was once a lot of parsing and the ability to set diffrent commands
  to diffrent modes.  That is gone, but some of the scaffolding remains */

static void check_env_var(void) {
	char *mode = getenv("COMMAND_MODE");

	if (mode) {
		if (!strcasecmp(mode, "legacy")) {
			unix2003_mode = false;
		} else {
			if (!strcasecmp(mode, "unix2003")) {
				unix2003_mode = true;
			} else {
				parse_error = true;
				unix2003_mode = UNIX2003_DEFAULT_MODE;
			}
		}
	}
}

/* Function is expected to be something like libc/malloc for a libc call,
or bin/date for command line utilities.  Modes are currently:
  Legacy - pre-tiger behaviour, presumably UNIX2003 incompatable
  UNIX2003 - Unix 2003 spec compliant
  Bootstrap - only seen by (parts of) libc.  The compat_mode system is 
    still starting up.  This will be seen while compat_mode parses it's 
	config file, or reads the cache file, and only by libc functions it calls.
  Error - the conf file could not be parsed, either due to a severe
    syntax error, an I/O error, or an out of memory condition

  mode names are case insensitatave.  You can use | ^ & and even !
  but no () yet, and that stuff hasn't been tested much yet, nor
  has it been optimised.
*/

bool
compat_mode(const char *function, const char *mode) {
	if (!parsed && !bootstrapping) {
		pthread_once(&threadsafe, check_env_var);
		parsed = true;
	}

	bool want2003 = !strcasecmp("unix2003", mode);

	if (want2003) {
		return unix2003_mode;
	}

	bool want_legacy = !strcasecmp("legacy", mode);

	if (want_legacy) {
		return !unix2003_mode;
	}

	bool want_bootstrap = !strcasecmp("bootstrap", mode);

	if (want_bootstrap) {
		return bootstrapping;
	}

	bool want_error = !strcasecmp("error", mode);

	if (want_error) {
		return parse_error;
	}

	char *op = NULL;

	if ((op = strpbrk(mode, "!^&|"))) {
		if (*op == '!') {
			if (op != mode) goto syn_error;
			return !compat_mode(function, mode +1);
		}

		/* XXX char tmp[] would be better for left_arg, but
		  we are not sure what the max size should be...  is
		  alloca(3) frowned on? */
		size_t left_sz = 1 + (op - mode);
		char *left_arg = malloc(left_sz);
		strlcpy(left_arg, mode, left_sz);
		bool left = compat_mode(function, left_arg);
		free(left_arg);
		bool right = compat_mode(function, op +1);

		/* XXX check leftOPright syntax errors */

		switch(*op) {
			case '^':
				return left ^ right;
			case '&':
				return left && right;
			case '|':
				return left || right;
			default:
				goto syn_error;
		}
	}

syn_error:
	fprintf(stderr, "invalid mode %s (while checking for %s)\n",
	  mode, function);

	return false;
}

#ifdef SELF_TEST_COMPAT_MODE

#include <assert.h>

#define NEXPECTED 3

typedef struct {
	char *mode;
	/* [0] is unix2003 mode, [1] is legacy, [2] is Invalid_Mode */
	bool expected[NEXPECTED];
} testcase;

testcase cases[] = {
	{ "unix2003", {true, false, true}},
	{ "legacy", {false, true, false}},
	{ "bootstrap", {false, false, false}},
	{ "unix2003|legacy", {true, true, true}},
	{ "unix2003&legacy", {false, false, false}},
	{ "unix2003|legacy|bootstrap", {true, true, true}},
	{ "unix2003&legacy&bootstrap", {false, false, false}},
	{ "!unix2003", {false, true, false}},
	{ "!legacy", {true, false, true}},
	/* XXX ! with compound statments */
	{ "unix2003^bootstrap", {true, false, true}},
	{ "unix2003^legacy", {true, true, true}},
	{ "&unix2003", {false, false, false}},
	{ "unix2003&", {false, false, false}},
	{ "unix2003!legacy", {false, false, false}},
	{ "unix2003&error", {false, false, true}},
	{ "error", {false, false, true}},
	{ "error|unix2003", {true, false, true}},
	{ "error|legacy", {false, true, true}},
	{ "error&legacy", {false, false, false}},
};

int ncases = sizeof(cases)/sizeof(testcase);

int
main(int argc, char *argv[]) {
	int i, j;
	int failures = 0;
	char *settings[] = { "unix2003", "legacy", "Invalid Mode"};

	assert(sizeof(settings) / sizeof(char *) == NEXPECTED);

	for(i = 0; i < NEXPECTED; ++i) {
		setenv("COMMAND_MODE", settings[i], 1);
		char *compat_env = getenv("COMMAND_MODE");
		printf("$COMMAND_MODE = %s\n", compat_env);
		if (i != 0) {
			/* here we force COMMAND_MODE to be checked again, which
			  is normally impossiable because check_env_var() is a static
			  function, but really nothing except the test cases wants to
			  try it anyway... */
			check_env_var();
		}
		for(j = 0; j < ncases; ++j) {
			bool ret = compat_mode("bin/compat_mode", cases[j].mode);
			bool expect = cases[j].expected[i];

			if (expect != ret) {
				failures++;
			}

			printf("Case %s got %d expected %d%s\n",
			  cases[j].mode, ret, expect, (ret == expect) ? "" : " FAILED");
		}
	}

	/* We have ncases entries in cases[], but each is run multiple
	  times (once per entry in the settings array) thus the multiply */
	printf("Passed %d of %d cases\n",
	  NEXPECTED*ncases - failures, NEXPECTED*ncases);

	return failures ? 1 : 0;
}

#endif
