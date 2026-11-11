/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Guest test: libScreenReader reports that no screen reader runs and that
 * the system is not in recovery mode. Run inside Darling; prints PASS or
 * FAIL and exits non-zero on failure.
 */
#include <stdbool.h>
#include <stdio.h>

bool _ScreenReaderIsRunning(void);
bool _ScreenReaderIsInRecoveryMode(void);

int main(void)
{
	int failures = 0;
	if (_ScreenReaderIsRunning()) {
		fprintf(stderr, "FAIL: screen reader reported running\n");
		failures++;
	}
	if (_ScreenReaderIsInRecoveryMode()) {
		fprintf(stderr, "FAIL: recovery mode reported\n");
		failures++;
	}
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}
