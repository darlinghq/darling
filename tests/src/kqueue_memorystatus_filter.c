/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Cristian Magherusan-Stanciu
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * The memory pressure dispatch source (DISPATCH_SOURCE_TYPE_MEMORYPRESSURE,
 * documented in <dispatch/source.h>) must be creatable, activatable and
 * cancellable on every system: it reports changes of the memory pressure
 * condition, so a source whose condition does not change is simply quiet.
 * It sits on the kqueue filter EVFILT_MEMORYSTATUS, which <sys/event.h>
 * declares for internal use only (value -14, flags in the same header).
 *
 * The kevent(2) contract checked on that filter, where every change is
 * submitted with EV_RECEIPT so that the outcome of each one is returned as
 * an EV_ERROR event whose data is the error number (zero for success):
 *
 * - adding a registration, with the flags libdispatch's source uses
 *   (EV_DISPATCH | EV_UDATA_SPECIFIC) succeeds;
 * - EV_UDATA_SPECIFIC makes one registration per udata value, so two
 *   registrations with the same ident and different udata can be deleted
 *   independently, and a later poll of the queue succeeds;
 * - a registration can be disabled and enabled again;
 * - deleting a registration that does not exist fails with ENOENT;
 * - an unknown filter fails with EINVAL.
 *
 * The test makes no claim about when the filter reports events. Exact
 * values are not asserted beyond those listed above.
 *
 * Prints PASS and exits 0.
 */
#include <dispatch/dispatch.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>

/* EVFILT_MEMORYSTATUS and its NOTE_ flags are not visible without PRIVATE. */
#define TEST_EVFILT_MEMORYSTATUS (-14)
#define TEST_NOTE_PRESSURE_NORMAL 0x1
#define TEST_NOTE_PRESSURE_WARN 0x2
#define TEST_NOTE_PRESSURE_CRITICAL 0x4
#define TEST_EVFILT_UNKNOWN (-100)

static int failures;

#define CHECK(cond, ...) do { \
	if (!(cond)) { \
		failures++; \
		printf("FAIL %s:%d: ", __FILE__, __LINE__); \
		printf(__VA_ARGS__); \
		printf("\n"); \
	} \
} while (0)

/*
 * Submits one change with EV_RECEIPT and returns its error number, or -1 if
 * the receipt is missing or malformed.
 */
static int change(int kq, short filter, unsigned short flags, unsigned fflags,
	void *udata)
{
	struct kevent ch, out;
	struct timespec zero = {0, 0};

	EV_SET(&ch, 0, filter, flags | EV_RECEIPT, fflags, 0, udata);
	memset(&out, 0, sizeof(out));
	int n = kevent(kq, &ch, 1, &out, 1, &zero);
	if (n != 1 || !(out.flags & EV_ERROR) || out.filter != filter) {
		printf("  receipt: kevent returned %d, flags 0x%x, filter %d\n",
			n, out.flags, out.filter);
		return -1;
	}
	return (int)out.data;
}

static const unsigned add_flags = EV_ADD | EV_ENABLE | EV_DISPATCH | EV_UDATA_SPECIFIC;
static const unsigned pressure_mask = TEST_NOTE_PRESSURE_NORMAL |
	TEST_NOTE_PRESSURE_WARN | TEST_NOTE_PRESSURE_CRITICAL;

static void test_registration(int kq)
{
	int a = 1, b = 2, r;

	r = change(kq, TEST_EVFILT_MEMORYSTATUS, add_flags, pressure_mask, &a);
	CHECK(r == 0, "add: error %d (%s), expected success", r, strerror(r));
	r = change(kq, TEST_EVFILT_MEMORYSTATUS, add_flags, pressure_mask, &b);
	CHECK(r == 0, "add with another udata: error %d (%s), expected success", r, strerror(r));

	r = change(kq, TEST_EVFILT_MEMORYSTATUS, EV_DISABLE | EV_UDATA_SPECIFIC, 0, &a);
	CHECK(r == 0, "disable: error %d (%s), expected success", r, strerror(r));
	r = change(kq, TEST_EVFILT_MEMORYSTATUS, EV_ENABLE | EV_UDATA_SPECIFIC, 0, &a);
	CHECK(r == 0, "enable: error %d (%s), expected success", r, strerror(r));

	/* Neither registration may take the other one with it. */
	r = change(kq, TEST_EVFILT_MEMORYSTATUS, EV_DELETE | EV_UDATA_SPECIFIC, 0, &a);
	CHECK(r == 0, "delete first: error %d (%s), expected success", r, strerror(r));
	r = change(kq, TEST_EVFILT_MEMORYSTATUS, EV_DELETE | EV_UDATA_SPECIFIC, 0, &b);
	CHECK(r == 0, "delete second: error %d (%s), expected success", r, strerror(r));

	{
		struct kevent out;
		struct timespec zero = {0, 0};
		int n = kevent(kq, NULL, 0, &out, 1, &zero);
		CHECK(n >= 0, "poll after deleting: %s", strerror(errno));
	}
}

static void test_errors(int kq)
{
	int a = 1, r;

	r = change(kq, TEST_EVFILT_MEMORYSTATUS, EV_DELETE | EV_UDATA_SPECIFIC, 0, &a);
	CHECK(r == ENOENT, "delete of a missing registration: error %d (%s), expected ENOENT",
		r, strerror(r));
	r = change(kq, TEST_EVFILT_UNKNOWN, add_flags, 0, &a);
	CHECK(r == EINVAL, "unknown filter: error %d (%s), expected EINVAL", r, strerror(r));
}

static void source_event(void *ctx)
{
	(void)ctx;
}

static void source_cancelled(void *ctx)
{
	dispatch_semaphore_signal((dispatch_semaphore_t)ctx);
}

/* The public entry point of the same filter. */
static void test_dispatch_source(void)
{
	dispatch_queue_t q = dispatch_queue_create("memorypressure-test", NULL);
	dispatch_semaphore_t cancelled = dispatch_semaphore_create(0);
	dispatch_source_t s = dispatch_source_create(DISPATCH_SOURCE_TYPE_MEMORYPRESSURE, 0,
		DISPATCH_MEMORYPRESSURE_NORMAL | DISPATCH_MEMORYPRESSURE_WARN |
		DISPATCH_MEMORYPRESSURE_CRITICAL, q);

	CHECK(s != NULL, "dispatch_source_create returned NULL");
	if (!s)
		return;
	dispatch_set_context(s, cancelled);
	dispatch_source_set_event_handler_f(s, source_event);
	dispatch_source_set_cancel_handler_f(s, source_cancelled);
	dispatch_activate(s);
	/* Let the registration reach the kqueue before cancelling it. */
	usleep(200 * 1000);
	dispatch_source_cancel(s);
	CHECK(dispatch_semaphore_wait(cancelled,
		dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC)) == 0,
		"cancel handler of the memory pressure source did not run");
}

int main(void)
{
	int kq = kqueue();

	if (kq < 0) {
		printf("FAIL kqueue: %s\n", strerror(errno));
		return 1;
	}
	test_registration(kq);
	test_errors(kq);
	close(kq);
	test_dispatch_source();
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
