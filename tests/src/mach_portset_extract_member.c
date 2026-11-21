/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
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
 * mach_port_insert_member and mach_port_extract_member on a receive right that
 * is a member of several port sets.
 *
 * mach_port_extract_member removes the port from one named set and returns
 * KERN_SUCCESS when it was a member of that set; KERN_NOT_IN_SET when it was
 * not; and mach_port_insert_member returns KERN_ALREADY_IN_SET when the port is
 * already in the set. The tests below check the return codes and, with a
 * message sent to the port, that the port keeps being received through exactly
 * the sets it is still a member of, whichever of its sets is removed first.
 *
 * Runs on macOS and in Darling; prints PASS and exits 0.
 */
#include <mach/mach.h>
#include <stdio.h>
#include <string.h>

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL %d: %s\n", __LINE__, #cond); \
		failures++; \
	} \
} while (0)

#define CHECK_KR(call, expected) do { \
	kern_return_t _kr = (call); \
	if (_kr != (expected)) { \
		printf("FAIL %d: %s returned %d, expected %d\n", __LINE__, #call, _kr, (int) (expected)); \
		failures++; \
	} \
} while (0)

static int failures;

typedef struct {
	mach_msg_header_t head;
	int serial;
} send_msg_t;

typedef struct {
	mach_msg_header_t head;
	int serial;
	mach_msg_trailer_t trailer;
} recv_msg_t;

static mach_port_t newPort(void)
{
	mach_port_t port = MACH_PORT_NULL;
	mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
	mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);
	return port;
}

static mach_port_t newSet(void)
{
	mach_port_t set = MACH_PORT_NULL;
	mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET, &set);
	return set;
}

static void post(mach_port_t port, int serial)
{
	send_msg_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.head.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
	msg.head.msgh_size = sizeof(msg);
	msg.head.msgh_remote_port = port;
	msg.serial = serial;
	CHECK_KR(mach_msg(&msg.head, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof(msg), 0, MACH_PORT_NULL, 0, MACH_PORT_NULL), KERN_SUCCESS);
}

// Receives from a set (or port) without blocking for long; returns the serial
// of the message, or -1 when none arrives.
static int take(mach_port_t from)
{
	recv_msg_t msg;
	memset(&msg, 0, sizeof(msg));
	if (mach_msg(&msg.head, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(msg), from, 100, MACH_PORT_NULL) != MACH_MSG_SUCCESS)
		return -1;
	return msg.serial;
}

// Whether a message posted to `port` can be received through `set`. A message
// that is not received stays queued on the port while the port is in some other
// set; it is taken from the port itself when that is possible (a port in no set)
// so that queued messages do not accumulate. A port that is a member of a set
// always has a message to receive through that set while one is queued, so a
// leftover message never changes the answer.
static int seenThrough(mach_port_t port, mach_port_t set, int serial)
{
	post(port, serial);
	if (take(set) != -1)
		return 1;
	take(port);
	return 0;
}

// A port that is in two sets can be removed from either one first.
static void twoSets(int removeFirst)
{
	mach_port_t port = newPort(), set[2] = { newSet(), newSet() };

	CHECK_KR(mach_port_insert_member(mach_task_self(), port, set[0]), KERN_SUCCESS);
	CHECK_KR(mach_port_insert_member(mach_task_self(), port, set[1]), KERN_SUCCESS);
	CHECK(seenThrough(port, set[0], 1));
	CHECK(seenThrough(port, set[1], 2));

	CHECK_KR(mach_port_extract_member(mach_task_self(), port, set[removeFirst]), KERN_SUCCESS);
	CHECK(!seenThrough(port, set[removeFirst], 3));
	CHECK(seenThrough(port, set[!removeFirst], 4));
	CHECK_KR(mach_port_extract_member(mach_task_self(), port, set[removeFirst]), KERN_NOT_IN_SET);

	CHECK_KR(mach_port_extract_member(mach_task_self(), port, set[!removeFirst]), KERN_SUCCESS);
	CHECK(!seenThrough(port, set[!removeFirst], 5));
	CHECK_KR(mach_port_extract_member(mach_task_self(), port, set[!removeFirst]), KERN_NOT_IN_SET);

	mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
	mach_port_deallocate(mach_task_self(), port);
	mach_port_mod_refs(mach_task_self(), set[0], MACH_PORT_RIGHT_PORT_SET, -1);
	mach_port_mod_refs(mach_task_self(), set[1], MACH_PORT_RIGHT_PORT_SET, -1);
}

// A port in three sets can be removed from them in any order.
static void threeSets(const int order[3])
{
	mach_port_t port = newPort(), set[3] = { newSet(), newSet(), newSet() };
	int in[3] = { 1, 1, 1 };

	for (int i = 0; i < 3; i++)
		CHECK_KR(mach_port_insert_member(mach_task_self(), port, set[i]), KERN_SUCCESS);

	for (int step = 0; step < 3; step++) {
		int removed = order[step];
		CHECK_KR(mach_port_extract_member(mach_task_self(), port, set[removed]), KERN_SUCCESS);
		in[removed] = 0;
		for (int i = 0; i < 3; i++)
			CHECK(seenThrough(port, set[i], 10 + step) == in[i]);
		CHECK_KR(mach_port_extract_member(mach_task_self(), port, set[removed]), KERN_NOT_IN_SET);
	}

	mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
	mach_port_deallocate(mach_task_self(), port);
	for (int i = 0; i < 3; i++)
		mach_port_mod_refs(mach_task_self(), set[i], MACH_PORT_RIGHT_PORT_SET, -1);
}

// Inserting a port into a set it is already in fails and leaves every
// membership of the port as it was.
static void alreadyInSet(void)
{
	mach_port_t a = newPort(), b = newPort(), set[2] = { newSet(), newSet() };

	CHECK_KR(mach_port_insert_member(mach_task_self(), a, set[0]), KERN_SUCCESS);
	CHECK_KR(mach_port_insert_member(mach_task_self(), b, set[0]), KERN_SUCCESS);
	CHECK_KR(mach_port_insert_member(mach_task_self(), b, set[0]), KERN_ALREADY_IN_SET);
	CHECK_KR(mach_port_insert_member(mach_task_self(), a, set[1]), KERN_SUCCESS);
	CHECK_KR(mach_port_insert_member(mach_task_self(), a, set[1]), KERN_ALREADY_IN_SET);

	CHECK(seenThrough(a, set[0], 20));
	CHECK(seenThrough(a, set[1], 21));
	CHECK(seenThrough(b, set[0], 22));

	CHECK_KR(mach_port_extract_member(mach_task_self(), a, set[0]), KERN_SUCCESS);
	CHECK_KR(mach_port_extract_member(mach_task_self(), b, set[0]), KERN_SUCCESS);
	CHECK_KR(mach_port_extract_member(mach_task_self(), a, set[1]), KERN_SUCCESS);
	CHECK_KR(mach_port_extract_member(mach_task_self(), a, set[1]), KERN_NOT_IN_SET);
	CHECK_KR(mach_port_extract_member(mach_task_self(), b, set[1]), KERN_NOT_IN_SET);
}

int main(void)
{
	static const int orders[6][3] = {
		{ 0, 1, 2 }, { 0, 2, 1 }, { 1, 0, 2 }, { 1, 2, 0 }, { 2, 0, 1 }, { 2, 1, 0 },
	};

	twoSets(0);
	twoSets(1);

	for (int i = 0; i < 6; i++)
		threeSets(orders[i]);

	alreadyInSet();

	if (failures == 0)
		printf("PASS\n");
	return failures == 0 ? 0 : 1;
}
