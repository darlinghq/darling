// Copyright (C) 2026 Lubos Dolezel
//
// auditon(2) A_GETSINFO_ADDR: the audit session information for a session id. A process can
// ask for its own session (the one getaudit_addr reports); an id that names no session, a
// wrong length and a NULL buffer are errors.
//
// Expected results were observed with the same calls on macOS 26.7.3 and 10.14.4 (identical).
#include <bsm/audit.h>
#include <bsm/libbsm.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

int main(void)
{
	auditinfo_addr_t own, q;
	int r;

	memset(&own, 0, sizeof(own));
	check("getaudit_addr succeeds", getaudit_addr(&own, sizeof(own)) == 0);

	memset(&q, 0, sizeof(q));
	q.ai_asid = own.ai_asid;
	r = auditon(A_GETSINFO_ADDR, &q, sizeof(q));
	check("own session: succeeds", r == 0);
	check("own session: same audit user", q.ai_auid == own.ai_auid);
	check("own session: same session id", q.ai_asid == own.ai_asid);
	check("own session: same flags", q.ai_flags == own.ai_flags);

	memset(&q, 0, sizeof(q));
	q.ai_asid = 0x7fffff12;
	errno = 0;
	r = auditon(A_GETSINFO_ADDR, &q, sizeof(q));
	check("unknown session id: EINVAL", r == -1 && errno == EINVAL);

	memset(&q, 0, sizeof(q));
	q.ai_asid = own.ai_asid;
	errno = 0;
	r = auditon(A_GETSINFO_ADDR, &q, sizeof(q) - 4);
	check("short length: EINVAL", r == -1 && errno == EINVAL);
	errno = 0;
	r = auditon(A_GETSINFO_ADDR, &q, sizeof(q) + 8);
	check("long length: EINVAL", r == -1 && errno == EINVAL);
	errno = 0;
	r = auditon(A_GETSINFO_ADDR, NULL, sizeof(q));
	check("NULL buffer: EFAULT", r == -1 && errno == EFAULT);

	printf("%s\n", failures ? "FAILED" : "PASSED");
	return failures != 0;
}
