/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DTC_TEST_CHECK_H
#define DTC_TEST_CHECK_H

#include <stdio.h>
#include <stdlib.h>

static int check_failures;
static int check_count;

#define CHECK(cond)                                                       \
	do {                                                              \
		check_count++;                                            \
		if (!(cond)) {                                            \
			check_failures++;                                 \
			printf("FAIL %s:%d: %s\n", __FILE__, __LINE__,    \
			       #cond);                                    \
		}                                                         \
	} while (0)

#define CHECK_EQ(a, b)                                                    \
	do {                                                              \
		long long va_ = (long long)(a), vb_ = (long long)(b);     \
		check_count++;                                            \
		if (va_ != vb_) {                                         \
			check_failures++;                                 \
			printf("FAIL %s:%d: %s == %s (%lld vs %lld)\n",   \
			       __FILE__, __LINE__, #a, #b, va_, vb_);     \
		}                                                         \
	} while (0)

#define CHECK_DONE()                                                      \
	do {                                                              \
		printf("%d checks, %d failures\n", check_count,           \
		       check_failures);                                   \
		return check_failures ? 1 : 0;                            \
	} while (0)

#endif
