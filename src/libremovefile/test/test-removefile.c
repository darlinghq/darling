#include "removefile.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

static struct timeval tv;
static void start_timer(const char* str) {
	fprintf(stderr, "%s... ", str);
	assert(gettimeofday(&tv, NULL) == 0);
}

static void stop_timer() {
	struct timeval tv2;
	assert(gettimeofday(&tv2, NULL) == 0);
	long sec = tv2.tv_sec - tv.tv_sec;
	long usec;
	if (sec == 0) {
		usec = tv2.tv_usec - tv.tv_usec;
	} else {
		usec = tv2.tv_usec + (1000000 - tv.tv_usec);
	}
	fprintf(stderr, "%d.%03d\n", sec, usec);
}


static int removefile_confirm_callback(removefile_state_t state, const char * path, void * context) {
	assert(context == (void*)1234);
	fprintf(stderr, "confirm callback: %s\n", path);
	return REMOVEFILE_PROCEED;
}

static int removefile_error_callback(removefile_state_t state, const char * path, void * context) {
	assert(context == (void*)4567);
	int err = 0;
	assert(removefile_state_get(state, REMOVEFILE_STATE_ERRNO, &err) == 0);
	fprintf(stderr, "error callback: %s: %s (%d)\n", path, strerror(err), err);
	return REMOVEFILE_PROCEED;
}

static int removefile_status_callback(removefile_state_t state, const char * path, void * context) {
   fprintf(stderr, "status callback: %s\n", path);
   return REMOVEFILE_PROCEED;
}


int mkdirs() {
	start_timer("Creating directory structure");
	assert(mkdir("/tmp/removefile-test", 0755) == 0);
	assert(mkdir("/tmp/removefile-test/foo", 0755) == 0);
	assert(mkdir("/tmp/removefile-test/foo/bar", 0755) == 0);
	assert(mkdir("/tmp/removefile-test/foo/baz", 0755) == 0);
	int fd;
	assert((fd = open("/tmp/removefile-test/foo/baz/woot", O_CREAT | O_TRUNC | O_WRONLY, 0644)) != -1);
	write(fd, "Hello World\n", 12);
	close(fd);
        assert((fd = open("/tmp/removefile-test/foo/baz/wootage", O_CREAT | O_TRUNC | O_WRONLY, 0644)) != -1);
        write(fd, "Hello World\n", 12);
	assert(lseek(fd, 1024*1024*30, SEEK_SET) != -1);
        write(fd, "Goodbye Moon\n", 13);
        close(fd);
	stop_timer();
}

void* threadproc(void* state) {
	sleep(1);
	fprintf(stderr, "cancelling...\n");
	assert(removefile_cancel(state) == 0);
	return NULL;
}

int main(int argc, char *argv[]) {
	removefile_state_t state = NULL;
	removefile_callback_t callback = NULL;
	pthread_t thread = NULL;
	int err = 0;

	mkdirs();
	start_timer("removefile(NULL)");
	assert(removefile("/tmp/removefile-test", NULL, REMOVEFILE_SECURE_1_PASS | REMOVEFILE_RECURSIVE) == 0);
	stop_timer();


	mkdirs();
	assert((state = removefile_state_alloc()) != NULL);
	assert(pthread_create(&thread, NULL, threadproc, state) == 0);
	start_timer("removefile(state) with cancel");
        assert(removefile_state_set(state, REMOVEFILE_STATE_ERROR_CALLBACK, removefile_error_callback) == 0);
        assert(removefile_state_set(state, REMOVEFILE_STATE_ERROR_CONTEXT, (void*)4567) == 0);
	assert(removefile("/tmp/removefile-test", state, REMOVEFILE_SECURE_1_PASS | REMOVEFILE_RECURSIVE) == -1 && errno == ECANCELED);
	stop_timer();

        start_timer("removefile(NULL)");
        assert(removefile("/tmp/removefile-test", NULL, REMOVEFILE_SECURE_1_PASS | REMOVEFILE_RECURSIVE) == 0);
        stop_timer();

	mkdirs();
	assert(removefile_state_set(state, 1234567, (void*)1234567) == -1 && errno == EINVAL);

	assert(removefile_state_set(state, REMOVEFILE_STATE_CONFIRM_CALLBACK, removefile_confirm_callback) == 0);
	assert(removefile_state_get(state, REMOVEFILE_STATE_CONFIRM_CALLBACK, &callback) == 0);
	assert(callback == removefile_confirm_callback);
	assert(removefile_state_set(state, REMOVEFILE_STATE_CONFIRM_CONTEXT, (void*)1234) == 0);

	assert(removefile_state_set(state, REMOVEFILE_STATE_ERROR_CALLBACK, removefile_error_callback) == 0);
	assert(removefile_state_get(state, REMOVEFILE_STATE_ERROR_CALLBACK, &callback) == 0);
	assert(callback == removefile_error_callback);
	assert(removefile_state_set(state, REMOVEFILE_STATE_ERROR_CONTEXT, (void*)4567) == 0);

	assert(removefile_state_set(state, REMOVEFILE_STATE_STATUS_CALLBACK, removefile_status_callback) == 0);
	assert(removefile_state_get(state, REMOVEFILE_STATE_STATUS_CALLBACK, &callback) == 0);
	assert(callback == removefile_status_callback);
	assert(removefile_state_set(state, REMOVEFILE_STATE_STATUS_CONTEXT, (void*)5678) == 0);

	start_timer("removefile(state)");
	assert(removefile("/tmp/removefile-test", state, REMOVEFILE_SECURE_1_PASS | REMOVEFILE_RECURSIVE) == 0);
	stop_timer();

	return 0;
}
