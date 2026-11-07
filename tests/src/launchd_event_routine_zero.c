// Copyright (C) 2026 Lubos Dolezel
//
// launchd's XPC event routines are numbered by xpc_event_t in
// <xpc/launchd_defs.h>, and the first one, XPC_EVENT_GET_NAME, is 0.
// launchd has to answer it like any other event routine. It used to treat
// opcode 0 as "not an event routine" and dropped the request without a
// reply.
//
// The test sends XPC_EVENT_GET_NAME for a stream that has no events, plus
// XPC_EVENT_COPY as a control, over the bootstrap port. Both must come back
// with a reply that carries a launchd error code (the stream and token do
// not exist), and that code must not be EDOM, which launchd uses for an
// unknown opcode. If the control gets no reply either, pipe routines to
// launchd do not work at all, which is a libxpc problem, not this one.
//
// The process routines have the same numbering: XPC_PROCESS_JETSAM_SET_BAND
// (xpc_jetsam_t) is 0. It is sent without a label, so launchd must answer
// with an error under XPC_PROCESS_ROUTINE_KEY_ERROR (not EDOM).
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <xpc/launchd_defs.h>
#include <servers/bootstrap.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define STREAM "org.darlinghq.tests.event-routine-zero"

static int g_failures = 0;
static const char* g_step = "start";

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: no reply from launchd to %s%s\n", g_step,
		strcmp(g_step, "XPC_EVENT_COPY") == 0 ? " (control: XPC pipe routines to launchd do not work)" : "");
	finish(1);
}

static void send_routine(xpc_pipe_t pipe, xpc_object_t msg, const char* what, const char* error_key)
{
	g_step = what;

	xpc_object_t reply = NULL;
	int err = xpc_pipe_routine(pipe, msg, &reply);
	xpc_release(msg);

	if (err != 0 || reply == NULL)
	{
		printf("FAIL: %s: xpc_pipe_routine returned %d (%s), launchd did not reply\n",
			what, err, strerror(err));
		g_failures++;
		return;
	}

	xpc_object_t xerr = xpc_dictionary_get_value(reply, error_key);
	if (xerr == NULL || xpc_get_type(xerr) != XPC_TYPE_UINT64)
	{
		printf("FAIL: %s: the reply has no error code\n", what);
		g_failures++;
	}
	else
	{
		uint64_t code = xpc_uint64_get_value(xerr);
		if (code == EDOM)
		{
			printf("FAIL: %s: launchd rejected the opcode (EDOM)\n", what);
			g_failures++;
		}
		else
			printf("ok: %s: launchd replied with error %llu\n", what, (unsigned long long) code);
	}
	xpc_release(reply);
}

static void event_routine(xpc_pipe_t pipe, uint64_t op, const char* what, bool with_token)
{
	xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_uint64(msg, XPC_EVENT_ROUTINE_KEY_OP, op);
	xpc_dictionary_set_string(msg, XPC_EVENT_ROUTINE_KEY_STREAM, STREAM);
	if (with_token)
		xpc_dictionary_set_uint64(msg, XPC_EVENT_ROUTINE_KEY_TOKEN, 1);
	else
		xpc_dictionary_set_string(msg, XPC_EVENT_ROUTINE_KEY_NAME, "none");
	send_routine(pipe, msg, what, XPC_EVENT_ROUTINE_KEY_ERROR);
}

static void process_routine(xpc_pipe_t pipe, uint64_t op, const char* what)
{
	xpc_object_t msg = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_uint64(msg, XPC_PROCESS_ROUTINE_KEY_OP, op);
	send_routine(pipe, msg, what, XPC_PROCESS_ROUTINE_KEY_ERROR);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(15);

	xpc_pipe_t pipe = xpc_pipe_create_from_port(bootstrap_port, 0);
	if (pipe == NULL)
	{
		printf("FAIL: cannot create a pipe to launchd\n");
		finish(1);
	}

	event_routine(pipe, XPC_EVENT_COPY, "XPC_EVENT_COPY", false);
	if (g_failures)
	{
		printf("FAIL: XPC pipe routines to launchd do not work (control)\n");
		finish(1);
	}
	event_routine(pipe, XPC_EVENT_GET_NAME, "XPC_EVENT_GET_NAME", true);
	process_routine(pipe, XPC_PROCESS_JETSAM_SET_BAND, "XPC_PROCESS_JETSAM_SET_BAND");

	xpc_release(pipe);

	printf("%s\n", g_failures ? "FAIL" : "PASS");
	finish(g_failures ? 1 : 0);
}
