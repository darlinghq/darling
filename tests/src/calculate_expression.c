// Copyright (C) 2026 Lubos Dolezel
//
// CalculateExpression() in the Calculate private framework evaluates an
// arithmetic expression and writes the result as text.
//
// Covers: basic arithmetic and precedence, right-associative '^', unary signs,
// functions, factorial, hexadecimal and octal output, error returns (division
// by zero, malformed input, NULL arguments, overflowing output base), and that
// very long nested or unary input is rejected instead of overflowing the stack.
// Darling-specific: the evaluator's semantics are Darling's own.
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

typedef bool (*calc_fn)(const char *, int, int, char *, int);
static calc_fn calc;

static void expect(const char *expr, int base, const char *want)
{
	char buf[64] = "";
	bool ok = calc(expr, base, 0, buf, sizeof buf);
	if (!ok || strcmp(buf, want) != 0) {
		printf("FAIL: \"%s\" base %d -> %s \"%s\", want \"%s\"\n", expr, base, ok ? "ok" : "error", buf, want);
		g_failures++;
	}
}

static void expect_error(const char *expr)
{
	char buf[64];
	if (calc(expr, 10, 0, buf, sizeof buf)) {
		printf("FAIL: \"%s\" should be an error, got \"%s\"\n", expr, buf);
		g_failures++;
	}
}

int main(void)
{
	void *h = dlopen("/System/Library/PrivateFrameworks/Calculate.framework/Calculate", RTLD_NOW);
	if (!h) {
		printf("FAIL: dlopen: %s\n", dlerror());
		return 1;
	}
	calc = (calc_fn) dlsym(h, "CalculateExpression");
	if (!calc) {
		printf("FAIL: CalculateExpression not found\n");
		return 1;
	}

	expect("1+2", 10, "3");
	expect("2+3*4", 10, "14");
	expect("(2+3)*4", 10, "20");
	expect("10-4-3", 10, "3");
	expect("7/2", 10, "3.5");
	expect("7%4", 10, "3");
	expect("2^3^2", 10, "512");
	expect("-3+5", 10, "2");
	expect("--3", 10, "3");
	expect("  1 +  1 ", 10, "2");
	expect("sqrt(16)", 10, "4");
	expect("abs(-2.5)", 10, "2.5");
	expect("5!", 10, "120");
	expect("(3)!", 10, "6");
	expect("0.1+0.2", 10, "0.3");
	expect("255", 16, "FF");
	expect("8", 8, "10");

	expect_error("1/0");
	expect_error("1%0");
	expect_error("1+");
	expect_error("(1+2");
	expect_error("1 2");
	expect_error("foo(1)");
	expect_error("sqrt(-1)");
	expect_error("");

	char buf[64];
	CHECK(!calc(NULL, 10, 0, buf, sizeof buf));
	CHECK(!calc("1", 10, 0, NULL, sizeof buf));
	CHECK(!calc("1", 10, 0, buf, 0));
	CHECK(!calc("1e30", 16, 0, buf, sizeof buf));

	// Pathological input must fail, not crash.
	size_t n = 200000;
	char *deep = malloc(2 * n + 2);
	memset(deep, '(', n);
	deep[n] = '1';
	memset(deep + n + 1, ')', n);
	deep[2 * n + 1] = 0;
	CHECK(!calc(deep, 10, 0, buf, sizeof buf));
	memset(deep, '-', n);
	deep[n] = '1';
	deep[n + 1] = 0;
	CHECK(!calc(deep, 10, 0, buf, sizeof buf));
	free(deep);
	char *pw = malloc(4 * n + 2);
	for (size_t i = 0; i < n; i++) memcpy(pw + 2 * i, "1^", 2);
	pw[2 * n] = '1';
	pw[2 * n + 1] = 0;
	CHECK(!calc(pw, 10, 0, buf, sizeof buf));
	free(pw);

	if (g_failures) {
		printf("FAILED: %d\n", g_failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
