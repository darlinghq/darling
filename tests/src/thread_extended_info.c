// Copyright (C) 2026 Lubos Dolezel
//
// thread_info(THREAD_EXTENDED_INFO) and THREAD_IDENTIFIER_INFO for the calling thread and for a
// second, sleeping, named thread. Expectations are those of the public <mach/thread_info.h>
// contract and macOS 26: success with the full count, a run state, positive priorities,
// pth_name equal to the pthread_setname_np name, and KERN_INVALID_ARGUMENT for a too-small count.
#include <mach/mach.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int failures;
#define CHECK(cond, what) do { if (!(cond)) { printf("FAIL %s: %s\n", what, #cond); failures++; } else printf("PASS %s\n", what); } while (0)

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int ready, quit;

static void* worker(void* arg)
{
	pthread_setname_np("ext-info-worker");
	pthread_mutex_lock(&lock);
	ready = 1;
	pthread_cond_broadcast(&cond);
	while (!quit)
		pthread_cond_wait(&cond, &lock);
	pthread_mutex_unlock(&lock);
	return arg;
}

static void check_extended(mach_port_t t, const char* who, const char* name, int sleeping)
{
	char msg[96];
	thread_extended_info_data_t info;
	mach_msg_type_number_t count = THREAD_EXTENDED_INFO_COUNT;
	memset(&info, 0x5a, sizeof(info));
	kern_return_t kr = thread_info(t, THREAD_EXTENDED_INFO, (thread_info_t)&info, &count);
	snprintf(msg, sizeof(msg), "%s: THREAD_EXTENDED_INFO succeeds", who);
	CHECK(kr == KERN_SUCCESS, msg);
	snprintf(msg, sizeof(msg), "%s: count", who);
	CHECK(count == THREAD_EXTENDED_INFO_COUNT, msg);
	if (kr != KERN_SUCCESS)
		return;
	snprintf(msg, sizeof(msg), "%s: pth_name", who);
	CHECK(strcmp(info.pth_name, name) == 0, msg);
	snprintf(msg, sizeof(msg), "%s: run state", who);
	if (sleeping)
		CHECK(info.pth_run_state == TH_STATE_WAITING, msg);
	else
		CHECK(info.pth_run_state == TH_STATE_RUNNING, msg);
	snprintf(msg, sizeof(msg), "%s: priorities", who);
	CHECK(info.pth_curpri > 0 && info.pth_priority > 0 && info.pth_maxpriority >= info.pth_priority, msg);
	snprintf(msg, sizeof(msg), "%s: fields written", who);
	CHECK(info.pth_cpu_usage != 0x5a5a5a5a && info.pth_policy != 0x5a5a5a5a && info.pth_flags != 0x5a5a5a5a
	    && info.pth_sleep_time != 0x5a5a5a5a, msg);

	count = THREAD_EXTENDED_INFO_COUNT - 1;
	kr = thread_info(t, THREAD_EXTENDED_INFO, (thread_info_t)&info, &count);
	snprintf(msg, sizeof(msg), "%s: small count rejected", who);
	CHECK(kr == KERN_INVALID_ARGUMENT, msg);
}

static void check_identifier(mach_port_t t, const char* who)
{
	char msg[96];
	thread_identifier_info_data_t id;
	mach_msg_type_number_t count = THREAD_IDENTIFIER_INFO_COUNT;
	kern_return_t kr = thread_info(t, THREAD_IDENTIFIER_INFO, (thread_info_t)&id, &count);
	snprintf(msg, sizeof(msg), "%s: THREAD_IDENTIFIER_INFO succeeds", who);
	CHECK(kr == KERN_SUCCESS && count == THREAD_IDENTIFIER_INFO_COUNT, msg);
	snprintf(msg, sizeof(msg), "%s: thread id", who);
	CHECK(kr == KERN_SUCCESS && id.thread_id != 0 && id.thread_handle != 0, msg);
	count = THREAD_IDENTIFIER_INFO_COUNT - 1;
	snprintf(msg, sizeof(msg), "%s: identifier small count rejected", who);
	CHECK(thread_info(t, THREAD_IDENTIFIER_INFO, (thread_info_t)&id, &count) == KERN_INVALID_ARGUMENT, msg);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	pthread_t th;
	pthread_create(&th, NULL, worker, NULL);
	pthread_mutex_lock(&lock);
	while (!ready)
		pthread_cond_wait(&cond, &lock);
	pthread_mutex_unlock(&lock);
	usleep(200000);

	mach_port_t other = pthread_mach_thread_np(th);
	pthread_setname_np("ext-info-main");

	check_extended(mach_thread_self(), "self", "ext-info-main", 0);
	check_extended(other, "other", "ext-info-worker", 1);
	check_identifier(mach_thread_self(), "self");
	check_identifier(other, "other");

	pthread_mutex_lock(&lock);
	quit = 1;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&lock);
	pthread_join(th, NULL);
	printf(failures ? "FAILED\n" : "OK\n");
	return failures != 0;
}
