// Copyright (C) 2026 Lubos Dolezel
//
// A job removed with LAUNCH_KEY_REMOVEJOB and submitted again under the same label must have
// working MachServices as soon as the removal has been reported complete (GetJob no longer finds
// the label): the removed job's service names are free again and the new job's names are served.
//
// Each of 50 rounds submits a job whose MachServices entry is checked in by the program itself
// (this executable, started with --child), removes it, polls GetJob until the label is gone and
// immediately submits the next round's job under the same label and service name. Every round
// looks the service up with bootstrap_look_up and sends it a message; the child answers it on the
// reply port. Even rounds ping the job (so it is running when it is removed), odd rounds remove
// the job without it ever having run.
//
// Build: clang <target flags> launchd_resubmit_mach_services.c -o launchd_resubmit_mach_services
#include <launch.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <crt_externs.h>
#include <errno.h>
#include <mach-o/dyld.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ROUNDS 50
#define PING_ID 7001
#define PONG_ID 7002

typedef struct {
	mach_msg_header_t header;
	mach_msg_trailer_t trailer;
} message_t;

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static char g_label[128];
static char g_service[128];
static char g_self[PATH_MAX];

static int child_main(const char *service)
{
	mach_port_t port = MACH_PORT_NULL;
	if (bootstrap_check_in(bootstrap_port, service, &port) != KERN_SUCCESS)
		return 2;
	for (;;) {
		message_t m;
		memset(&m, 0, sizeof(m));
		if (mach_msg(&m.header, MACH_RCV_MSG, 0, sizeof(m), port, MACH_MSG_TIMEOUT_NONE,
				MACH_PORT_NULL) != KERN_SUCCESS)
			return 3;
		if (m.header.msgh_id != PING_ID || m.header.msgh_remote_port == MACH_PORT_NULL)
			continue;
		mach_msg_header_t reply = { 0 };
		reply.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_MOVE_SEND_ONCE, 0);
		reply.msgh_size = sizeof(reply);
		reply.msgh_remote_port = m.header.msgh_remote_port;
		reply.msgh_id = PONG_ID;
		(void) mach_msg(&reply, MACH_SEND_MSG, sizeof(reply), 0, MACH_PORT_NULL, 2000,
			MACH_PORT_NULL);
	}
}

// ---- launchd requests ----

static launch_data_t request(const char *key, launch_data_t value)
{
	launch_data_t msg = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
	launch_data_dict_insert(msg, value, key);
	launch_data_t reply = launch_msg(msg);
	launch_data_free(msg);
	return reply;
}

// 0 on success, otherwise the errno launchd reported
static int errno_request(const char *key, launch_data_t value)
{
	launch_data_t reply = request(key, value);
	if (!reply)
		return errno ? errno : -1;
	int rc = launch_data_get_type(reply) == LAUNCH_DATA_ERRNO ? launch_data_get_errno(reply) : -1;
	launch_data_free(reply);
	return rc;
}

static int job_exists(void)
{
	launch_data_t reply = request(LAUNCH_KEY_GETJOB, launch_data_new_string(g_label));
	if (!reply)
		return -1;
	int exists = launch_data_get_type(reply) == LAUNCH_DATA_DICTIONARY;
	launch_data_free(reply);
	return exists;
}

static int submit_job(void)
{
	launch_data_t job = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
	launch_data_dict_insert(job, launch_data_new_string(g_label), LAUNCH_JOBKEY_LABEL);
	launch_data_t args = launch_data_alloc(LAUNCH_DATA_ARRAY);
	launch_data_array_set_index(args, launch_data_new_string(g_self), 0);
	launch_data_array_set_index(args, launch_data_new_string("--child"), 1);
	launch_data_array_set_index(args, launch_data_new_string(g_service), 2);
	launch_data_dict_insert(job, args, LAUNCH_JOBKEY_PROGRAMARGUMENTS);
	launch_data_t services = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
	launch_data_dict_insert(services, launch_data_new_bool(true), g_service);
	launch_data_dict_insert(job, services, LAUNCH_JOBKEY_MACHSERVICES);
	launch_data_dict_insert(job, launch_data_new_bool(false), LAUNCH_JOBKEY_KEEPALIVE);
	return errno_request(LAUNCH_KEY_SUBMITJOB, job);
}

// Removes the job and waits until GetJob no longer reports it.
static int remove_job_and_wait(void)
{
	int rc = errno_request(LAUNCH_KEY_REMOVEJOB, launch_data_new_string(g_label));
	// A job whose process is still running is removed once the process has exited; launchd then
	// may answer EINPROGRESS (macOS does), which only says that the removal has started.
	if (rc && rc != EINPROGRESS)
		return rc;
	for (int i = 0; i < 20000; i++) {
		if (job_exists() == 0)
			return 0;
		usleep(500);
	}
	return ETIMEDOUT;
}

// Looks the service up and exchanges one message with it; 0 when the answer arrived.
static int ping_service(void)
{
	mach_port_t service = MACH_PORT_NULL;
	kern_return_t kr = bootstrap_look_up(bootstrap_port, g_service, &service);
	if (kr != KERN_SUCCESS)
		return 1;
	mach_port_t reply_port = MACH_PORT_NULL;
	if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &reply_port) != KERN_SUCCESS)
		return 2;
	mach_msg_header_t ping = { 0 };
	ping.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);
	ping.msgh_size = sizeof(ping);
	ping.msgh_remote_port = service;
	ping.msgh_local_port = reply_port;
	ping.msgh_id = PING_ID;
	int rc = 0;
	if (mach_msg(&ping, MACH_SEND_MSG | MACH_SEND_TIMEOUT, sizeof(ping), 0, MACH_PORT_NULL, 5000,
			MACH_PORT_NULL) != KERN_SUCCESS) {
		rc = 3;
	} else {
		message_t m;
		memset(&m, 0, sizeof(m));
		kr = mach_msg(&m.header, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof(m), reply_port, 5000,
			MACH_PORT_NULL);
		if (kr != KERN_SUCCESS || m.header.msgh_id != PONG_ID)
			rc = 4;
	}
	mach_port_mod_refs(mach_task_self(), reply_port, MACH_PORT_RIGHT_RECEIVE, -1);
	mach_port_deallocate(mach_task_self(), service);
	return rc;
}

int main(int argc, char **argv)
{
	if (argc >= 3 && !strcmp(argv[1], "--child"))
		return child_main(argv[2]);

	uint32_t size = sizeof(g_self);
	if (_NSGetExecutablePath(g_self, &size) != 0) {
		printf("FAIL: executable path\n");
		return 1;
	}
	snprintf(g_label, sizeof(g_label), "org.darlinghq.tests.resubmit.%d", (int) getpid());
	snprintf(g_service, sizeof(g_service), "org.darlinghq.tests.resubmit.%d.service", (int) getpid());

	int submit_fail = 0, lookup_fail = 0, remove_fail = 0;
	for (int round = 0; round < ROUNDS; round++) {
		int rc = submit_job();
		if (rc) {
			printf("round %d: submit failed: %d\n", round, rc);
			submit_fail++;
			break;
		}
		// Even rounds run the job before it is removed; odd rounds remove it unstarted.
		if (round % 2 == 0 && (rc = ping_service())) {
			printf("round %d: service does not answer (step %d)\n", round, rc);
			lookup_fail++;
		}
		if ((rc = remove_job_and_wait())) {
			printf("round %d: remove failed: %d\n", round, rc);
			remove_fail++;
			break;
		}
		// The label is reported gone: submit it again at once, its service must work.
		if ((rc = submit_job())) {
			printf("round %d: resubmit failed: %d\n", round, rc);
			submit_fail++;
			break;
		}
		if ((rc = ping_service())) {
			printf("round %d: resubmitted job's service does not answer (step %d)\n", round, rc);
			lookup_fail++;
		}
		if ((rc = remove_job_and_wait())) {
			printf("round %d: second remove failed: %d\n", round, rc);
			remove_fail++;
			break;
		}
	}
	CHECK(submit_fail == 0);
	CHECK(remove_fail == 0);
	CHECK(lookup_fail == 0);
	if (g_failures) {
		printf("FAIL (%d failures)\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
