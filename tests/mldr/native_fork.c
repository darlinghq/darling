// Host-side test of mldr's native fork callback: a failed fork is reported as a
// negative errno, the child-side resets run only in the child, and the child never
// gets a copy of the socket bitmap taken while another thread was changing it.
#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include "socket_bitmap.h"
#include "elfcalls/native_fork.h"

#define HIGHEST 1023
#define STRESS_FORKS 200

static socket_bitmap_t bitmap = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.highest = HIGHEST,
};

static int thread_resets;

// The same hooks mldr.c provides for its own bitmap.
void __mldr_socket_bitmap_prefork(void) { socket_bitmap_fork_prepare(&bitmap); }
void __mldr_socket_bitmap_postfork_parent(void) { socket_bitmap_fork_parent(&bitmap); }
void __mldr_socket_bitmap_postfork_child(void) { socket_bitmap_fork_child(&bitmap); }
void __mldr_thread_postfork_child(void) { thread_resets++; }

static void fail(const char* message)
{
	printf("FAIL: %s\n", message);
	exit(1);
}

static int bitmap_unlocked(void)
{
	if (pthread_mutex_trylock(&bitmap.mutex) != 0)
		return 0;
	pthread_mutex_unlock(&bitmap.mutex);
	return 1;
}

static int wait_for(pid_t pid)
{
	int status;

	if (waitpid(pid, &status, 0) != pid)
		fail("waitpid");
	return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
}

static void test_success(void)
{
	pid_t pid = __mldr_native_fork();

	if (pid < 0)
		fail("fork failed");
	if (pid == 0)
		_exit(thread_resets == 1 && bitmap_unlocked() ? 0 : 1);

	if (wait_for(pid) != 0)
		fail("the child was not reset, or inherited a locked bitmap");
	if (thread_resets != 0)
		fail("the child reset ran in the parent");
	if (!bitmap_unlocked())
		fail("the parent's bitmap stayed locked");
}

static void test_failure(void)
{
	pid_t pid = fork();

	if (pid < 0)
		fail("fork failed");
	if (pid == 0)
	{
		// Make every way of creating a process fail with EAGAIN, in this disposable child only.
		struct sock_filter filters[] = {
			BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
#ifdef __NR_fork
			BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_fork, 0, 1),
			BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | EAGAIN),
#endif
			BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_clone, 0, 1),
			BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | EAGAIN),
#ifdef __NR_clone3
			BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_clone3, 0, 1),
			BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | EAGAIN),
#endif
			BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
		};
		struct sock_fprog program = { sizeof(filters) / sizeof(filters[0]), filters };

		if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0 || prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &program) != 0)
			_exit(77);

		int result = __mldr_native_fork();
		if (result != -EAGAIN)
		{
			printf("native fork returned %d instead of %d\n", result, -EAGAIN);
			_exit(1);
		}
		_exit(thread_resets == 0 && bitmap_unlocked() ? 0 : 1);
	}

	int status = wait_for(pid);
	if (status == 77)
		printf("SKIP: cannot install a seccomp filter, failed fork not tested\n");
	else if (status != 0)
		fail("a failed fork was not reported as -EAGAIN with the bitmap unlocked");
}

static int ready_pipe[2];

static void* update_slowly(void* arg)
{
	(void) arg;

	pthread_mutex_lock(&bitmap.mutex);
	bitmap.next_index = 1;
	if (write(ready_pipe[1], "x", 1) != 1)
		fail("write");
	usleep(200 * 1000);
	bitmap.next_index = 0;
	pthread_mutex_unlock(&bitmap.mutex);
	return NULL;
}

// A fork that starts while another thread is changing the bitmap must wait for it to finish.
static void test_waits_for_update(void)
{
	pthread_t thread;
	char byte;

	if (pipe(ready_pipe) != 0)
		fail("pipe");
	if (pthread_create(&thread, NULL, update_slowly, NULL) != 0)
		fail("pthread_create");
	if (read(ready_pipe[0], &byte, 1) != 1)
		fail("read");

	pid_t pid = __mldr_native_fork();
	if (pid < 0)
		fail("fork failed");
	if (pid == 0)
		_exit(bitmap.next_index == 0 && bitmap_unlocked() ? 0 : 1);

	if (wait_for(pid) != 0)
		fail("the child got a copy of the bitmap in the middle of an update");
	pthread_join(thread, NULL);
	close(ready_pipe[0]);
	close(ready_pipe[1]);
}

static volatile int stop_churning;

static void* churn(void* arg)
{
	(void) arg;

	while (!stop_churning)
	{
		int fds[8];
		for (int i = 0; i < 8; i++)
			fds[i] = socket_bitmap_get(&bitmap);
		for (int i = 0; i < 8; i++)
			if (fds[i] >= 0)
				socket_bitmap_put(&bitmap, fds[i]);
	}
	return NULL;
}

// Forks while two threads keep taking and releasing descriptors. Each child must be able
// to take a descriptor itself; with an inherited locked mutex it would hang until the alarm.
static void test_stress(void)
{
	pthread_t threads[2];

	for (int i = 0; i < 2; i++)
		if (pthread_create(&threads[i], NULL, churn, NULL) != 0)
			fail("pthread_create");

	for (int i = 0; i < STRESS_FORKS; i++)
	{
		pid_t pid = __mldr_native_fork();
		if (pid < 0)
			fail("fork failed");
		if (pid == 0)
		{
			alarm(5);
			int fd = socket_bitmap_get(&bitmap);
			_exit(fd >= 0 && fd <= HIGHEST ? 0 : 1);
		}
		if (wait_for(pid) != 0)
			fail("a child could not use the socket bitmap after fork");
	}

	stop_churning = 1;
	for (int i = 0; i < 2; i++)
		pthread_join(threads[i], NULL);
}

static int ptrace_scope(void)
{
	FILE* f = fopen("/proc/sys/kernel/yama/ptrace_scope", "r");
	int scope = -1;

	if (f == NULL)
		return -1;
	if (fscanf(f, "%d", &scope) != 1)
		scope = -1;
	fclose(f);
	return scope;
}

// With Yama's ptrace_scope 1, only an ancestor or a process the target named
// as its ptracer may read its memory. darlingserver is neither, so the child
// of a native fork must name every process as its ptracer, as mldr does at
// startup. A sibling (not an ancestor) checks that it can read the child.
static void test_ptracer(void)
{
	static volatile int marker = 0x5a5aa5a5;
	int ready[2], done[2];
	char byte;

	if (ptrace_scope() != 1)
	{
		printf("SKIP: Yama ptrace_scope is not 1, ptracer renewal not tested\n");
		return;
	}
	if (pipe(ready) != 0 || pipe(done) != 0)
		fail("pipe");

	pid_t child = __mldr_native_fork();
	if (child < 0)
		fail("fork failed");
	if (child == 0)
	{
		if (write(ready[1], "x", 1) != 1 || read(done[0], &byte, 1) != 1)
			_exit(1);
		_exit(0);
	}
	if (read(ready[0], &byte, 1) != 1)
		fail("read");

	pid_t sibling = fork();
	if (sibling < 0)
		fail("fork failed");
	if (sibling == 0)
	{
		int value = 0;
		struct iovec local = { &value, sizeof(value) };
		struct iovec remote = { (void*) &marker, sizeof(marker) };

		if (process_vm_readv(child, &local, 1, &remote, 1, 0) != sizeof(value))
			_exit(1);
		_exit(value == marker ? 0 : 2);
	}

	int status = wait_for(sibling);
	if (write(done[1], "x", 1) != 1)
		fail("write");
	wait_for(child);
	if (status != 0)
		fail("a non-ancestor could not read the memory of a native fork child");
}

int main(void)
{
	test_success();
	test_failure();
	test_waits_for_update();
	test_stress();
	test_ptracer();

	printf("PASS\n");
	return 0;
}
