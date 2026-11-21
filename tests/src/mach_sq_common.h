// Copyright (C) 2026 Lubos Dolezel
//
// Helpers shared by the mach_sq_* tests: simple Mach messages to ports of the
// test's own task, which Darling may queue in memory the task shares with
// darlingserver instead of in the server. Every test also passes with
// DARLING_MACH_SERVER=1, which sends every message through the server.

#ifndef MACH_SQ_COMMON_H
#define MACH_SQ_COMMON_H

#include <mach/mach.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#define SQ_CHECK(cond, ...) do { \
		if (!(cond)) { \
			printf("FAIL %s:%d: ", __FILE__, __LINE__); \
			printf(__VA_ARGS__); \
			printf("\n"); \
			fflush(stdout); \
			exit(1); \
		} \
	} while (0)

#define SQ_KR(kr, what) SQ_CHECK((kr) == KERN_SUCCESS, "%s: 0x%x", what, (unsigned) (kr))

typedef struct
{
	mach_msg_header_t header;
	uint8_t body[1024];
} sq_buffer_t;

static inline double sq_now(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1e6;
}

// a receive right, with a send right under the same name if `send`
static inline mach_port_t sq_port(bool send)
{
	mach_port_t port;
	SQ_KR(mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port), "mach_port_allocate");
	if (send)
		SQ_KR(mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND), "insert_right");
	return port;
}

static inline void sq_destroy(mach_port_t port)
{
	mach_port_destroy(mach_task_self(), port);
}

static inline void sq_set_qlimit(mach_port_t port, mach_port_msgcount_t qlimit)
{
	mach_port_limits_t limits = { .mpl_qlimit = qlimit };
	SQ_KR(mach_port_set_attributes(mach_task_self(), port, MACH_PORT_LIMITS_INFO,
		(mach_port_info_t) &limits, MACH_PORT_LIMITS_INFO_COUNT), "set qlimit");
}

// a simple message of `size` bytes (header included) with id `id` and a body pattern from `id`
static inline mach_msg_return_t sq_send_ex(mach_port_t dest, mach_msg_type_name_t disposition, mach_msg_size_t size,
		mach_msg_id_t id, mach_msg_option_t options, mach_msg_timeout_t timeout)
{
	sq_buffer_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.header.msgh_bits = MACH_MSGH_BITS(disposition, 0);
	msg.header.msgh_size = size;
	msg.header.msgh_remote_port = dest;
	msg.header.msgh_local_port = MACH_PORT_NULL;
	msg.header.msgh_id = id;
	for (mach_msg_size_t i = 0; i + sizeof(mach_msg_header_t) < size && i < sizeof(msg.body); i++)
		msg.body[i] = (uint8_t) (id * 7 + i);
	return mach_msg(&msg.header, MACH_SEND_MSG | options, size, 0, MACH_PORT_NULL, timeout, MACH_PORT_NULL);
}

static inline mach_msg_return_t sq_send(mach_port_t dest, mach_msg_id_t id)
{
	return sq_send_ex(dest, MACH_MSG_TYPE_COPY_SEND, sizeof(mach_msg_header_t) + 8, id, 0, 0);
}

static inline mach_msg_return_t sq_receive_ex(mach_port_t port, sq_buffer_t* buffer, mach_msg_size_t size,
		mach_msg_option_t options, mach_msg_timeout_t timeout)
{
	return mach_msg(&buffer->header, MACH_RCV_MSG | options, 0, size, port, timeout, MACH_PORT_NULL);
}

static inline mach_msg_return_t sq_receive(mach_port_t port, sq_buffer_t* buffer, mach_msg_timeout_t timeout)
{
	return sq_receive_ex(port, buffer, sizeof(*buffer), timeout ? MACH_RCV_TIMEOUT : 0, timeout);
}

// whether this process has its shared message queue region mapped (see /proc/self/maps):
// 1 yes, 0 no, -1 can't tell
static inline int sq_region_mapped(void)
{
	char buffer[65536];
	int fd = open("/proc/self/maps", O_RDONLY);
	if (fd < 0)
		return -1;
	ssize_t total = 0, n;
	while (total < (ssize_t) sizeof(buffer) - 1 && (n = read(fd, buffer + total, sizeof(buffer) - 1 - total)) > 0)
		total += n;
	close(fd);
	buffer[total] = '\0';
	return strstr(buffer, "darling-mach-sq") != NULL;
}

static inline bool sq_server_only(void)
{
	const char* value = getenv("DARLING_MACH_SERVER");
	return value != NULL && strcmp(value, "1") == 0;
}

// fails the test if it runs for longer than `seconds` (a hang)
static void sq_watchdog_fired(int sig)
{
	(void) sig;
	static const char text[] = "FAIL: watchdog (the test hangs)\n";
	write(1, text, sizeof(text) - 1);
	_exit(1);
}

static inline void sq_watchdog(unsigned seconds)
{
	signal(SIGALRM, sq_watchdog_fired);
	alarm(seconds);
}

#endif // MACH_SQ_COMMON_H
