// Copyright (C) 2026 Lubos Dolezel
//
// PTHREAD_PROCESS_SHARED mutexes, condition variables and read-write locks
// work both between threads of one process and, in MAP_SHARED memory, between
// processes (after fork, and when the memory is mapped at a different address
// in the other process). A timed wait on an unsignaled condition variable
// returns ETIMEDOUT. Primitives that share a page do not affect each other.
// Every wait is bounded, so a failure is reported instead of hanging.

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define WAIT_SECONDS 5

struct shared {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_cond_t other_cond;
	pthread_rwlock_t rwlock;
	int flag;
	int other_flag;
	int ready;
	int counter;
};

static int failed;

#define CHECK(cond, ...) do { \
	if (!(cond)) { \
		printf("FAIL: " __VA_ARGS__); \
		printf("\n"); \
		failed = 1; \
	} \
} while (0)

static void init_shared(struct shared* s) {
	pthread_mutexattr_t ma;
	pthread_condattr_t ca;
	pthread_rwlockattr_t ra;

	pthread_mutexattr_init(&ma);
	pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&s->mutex, &ma);
	pthread_mutexattr_destroy(&ma);

	pthread_condattr_init(&ca);
	pthread_condattr_setpshared(&ca, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(&s->cond, &ca);
	pthread_cond_init(&s->other_cond, &ca);
	pthread_condattr_destroy(&ca);

	pthread_rwlockattr_init(&ra);
	pthread_rwlockattr_setpshared(&ra, PTHREAD_PROCESS_SHARED);
	pthread_rwlock_init(&s->rwlock, &ra);
	pthread_rwlockattr_destroy(&ra);
}

static struct timespec deadline(long milliseconds) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += milliseconds / 1000;
	ts.tv_nsec += (milliseconds % 1000) * 1000000L;
	if (ts.tv_nsec >= 1000000000L) {
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000L;
	}
	return ts;
}

// waits (with the mutex held) until *flag is set; returns 0 or the wait's error
static int wait_flag(struct shared* s, pthread_cond_t* cond, int* flag) {
	int rc = 0;
	struct timespec ts = deadline(WAIT_SECONDS * 1000);
	while (!__atomic_load_n(flag, __ATOMIC_ACQUIRE) && rc == 0) {
		rc = pthread_cond_timedwait(cond, &s->mutex, &ts);
	}
	return __atomic_load_n(flag, __ATOMIC_ACQUIRE) ? 0 : rc;
}

// runs `fn` in a child process; returns its exit status, or -1
static int run_child(int (*fn)(void*), void* arg) {
	pid_t pid = fork();
	if (pid < 0) {
		return -1;
	}
	if (pid == 0) {
		_exit(fn(arg));
	}
	int status = 0;
	if (waitpid(pid, &status, 0) != pid || !WIFEXITED(status)) {
		return -1;
	}
	return WEXITSTATUS(status);
}

//
// within one process
//

static void* signal_thread(void* arg) {
	struct shared* s = arg;
	usleep(100000);
	pthread_mutex_lock(&s->mutex);
	__atomic_store_n(&s->flag, 1, __ATOMIC_RELEASE);
	pthread_cond_signal(&s->cond);
	pthread_mutex_unlock(&s->mutex);
	return NULL;
}

static void* broadcast_waiter(void* arg) {
	struct shared* s = arg;
	pthread_mutex_lock(&s->mutex);
	s->ready++;
	int rc = wait_flag(s, &s->cond, &s->flag);
	pthread_mutex_unlock(&s->mutex);
	return (void*)(long)rc;
}

static void* mutex_incrementer(void* arg) {
	struct shared* s = arg;
	for (int i = 0; i < 2000; i++) {
		pthread_mutex_lock(&s->mutex);
		s->counter++;
		pthread_mutex_unlock(&s->mutex);
	}
	return NULL;
}

static void* writer_thread(void* arg) {
	struct shared* s = arg;
	pthread_rwlock_wrlock(&s->rwlock);
	__atomic_store_n(&s->flag, 1, __ATOMIC_RELEASE);
	pthread_rwlock_unlock(&s->rwlock);
	return NULL;
}

static void test_threads(struct shared* s) {
	pthread_t threads[4];
	int rc;

	// an unsignaled timed wait times out and returns with the mutex held
	pthread_mutex_lock(&s->mutex);
	struct timespec ts = deadline(200);
	rc = pthread_cond_timedwait(&s->cond, &s->mutex, &ts);
	CHECK(rc == ETIMEDOUT, "threads: timedwait on an unsignaled condition returned %d, expected ETIMEDOUT", rc);
	CHECK(pthread_mutex_trylock(&s->mutex) == EBUSY, "threads: the mutex was not held after a timed out wait");
	pthread_mutex_unlock(&s->mutex);

	// signal wakes a waiter
	s->flag = 0;
	pthread_create(&threads[0], NULL, signal_thread, s);
	pthread_mutex_lock(&s->mutex);
	rc = wait_flag(s, &s->cond, &s->flag);
	pthread_mutex_unlock(&s->mutex);
	pthread_join(threads[0], NULL);
	CHECK(rc == 0, "threads: wait after pthread_cond_signal returned %d", rc);

	// broadcast wakes every waiter
	s->flag = 0;
	s->ready = 0;
	for (int i = 0; i < 3; i++) {
		pthread_create(&threads[i], NULL, broadcast_waiter, s);
	}
	for (int i = 0; i < 100; i++) {
		pthread_mutex_lock(&s->mutex);
		int ready = s->ready;
		pthread_mutex_unlock(&s->mutex);
		if (ready == 3) {
			break;
		}
		usleep(20000);
	}
	usleep(100000);
	pthread_mutex_lock(&s->mutex);
	__atomic_store_n(&s->flag, 1, __ATOMIC_RELEASE);
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->mutex);
	for (int i = 0; i < 3; i++) {
		void* result;
		pthread_join(threads[i], &result);
		CHECK(result == NULL, "threads: broadcast waiter %d returned %ld", i, (long)result);
	}

	// a signal on one condition variable does not wake a waiter on another one in the same page
	s->flag = 1;
	s->other_flag = 0;
	pthread_mutex_lock(&s->mutex);
	pthread_cond_signal(&s->cond);
	ts = deadline(200);
	rc = pthread_cond_timedwait(&s->other_cond, &s->mutex, &ts);
	pthread_mutex_unlock(&s->mutex);
	CHECK(rc == ETIMEDOUT, "threads: neighbouring condition variable returned %d, expected ETIMEDOUT", rc);

	// mutual exclusion
	s->counter = 0;
	for (int i = 0; i < 4; i++) {
		pthread_create(&threads[i], NULL, mutex_incrementer, s);
	}
	for (int i = 0; i < 4; i++) {
		pthread_join(threads[i], NULL);
	}
	CHECK(s->counter == 8000, "threads: counter is %d, expected 8000", s->counter);

	// a locked mutex refuses trylock; an uncontended one gets it
	CHECK(pthread_mutex_trylock(&s->mutex) == 0, "threads: trylock of a free mutex failed");
	pthread_mutex_unlock(&s->mutex);

	// read-write lock: readers share, a writer waits for them
	s->flag = 0;
	CHECK(pthread_rwlock_rdlock(&s->rwlock) == 0, "threads: rdlock failed");
	CHECK(pthread_rwlock_tryrdlock(&s->rwlock) == 0, "threads: second rdlock failed");
	CHECK(pthread_rwlock_trywrlock(&s->rwlock) == EBUSY, "threads: trywrlock succeeded while read-locked");
	pthread_create(&threads[0], NULL, writer_thread, s);
	usleep(200000);
	CHECK(__atomic_load_n(&s->flag, __ATOMIC_ACQUIRE) == 0, "threads: the writer got the lock while readers held it");
	pthread_rwlock_unlock(&s->rwlock);
	pthread_rwlock_unlock(&s->rwlock);
	pthread_join(threads[0], NULL);
	CHECK(__atomic_load_n(&s->flag, __ATOMIC_ACQUIRE) == 1, "threads: the writer did not run after the readers unlocked");
}

//
// between processes
//

static int child_signal(void* arg) {
	struct shared* s = arg;
	usleep(100000);
	if (pthread_mutex_lock(&s->mutex) != 0) {
		return 10;
	}
	__atomic_store_n(&s->flag, 1, __ATOMIC_RELEASE);
	pthread_cond_signal(&s->cond);
	pthread_mutex_unlock(&s->mutex);
	return 0;
}

static int child_wait_broadcast(void* arg) {
	struct shared* s = arg;
	pthread_mutex_lock(&s->mutex);
	s->ready++;
	int rc = wait_flag(s, &s->cond, &s->flag);
	pthread_mutex_unlock(&s->mutex);
	return rc == 0 ? 0 : 11;
}

static int child_increment(void* arg) {
	mutex_incrementer(arg);
	return 0;
}

static int child_writer(void* arg) {
	writer_thread(arg);
	return 0;
}

static int child_timedwait(void* arg) {
	struct shared* s = arg;
	pthread_mutex_lock(&s->mutex);
	struct timespec ts = deadline(200);
	int rc = pthread_cond_timedwait(&s->cond, &s->mutex, &ts);
	pthread_mutex_unlock(&s->mutex);
	return rc == ETIMEDOUT ? 0 : 12;
}

static void test_processes(struct shared* s, const char* what) {
	int status;
	pid_t pids[3];

	// the child's timed wait on an unsignaled condition variable times out
	status = run_child(child_timedwait, s);
	CHECK(status == 0, "%s: child timedwait status %d, expected ETIMEDOUT result", what, status);

	// the parent waits for a signal from the child
	s->flag = 0;
	pid_t pid = fork();
	if (pid == 0) {
		_exit(child_signal(s));
	}
	pthread_mutex_lock(&s->mutex);
	int rc = wait_flag(s, &s->cond, &s->flag);
	pthread_mutex_unlock(&s->mutex);
	waitpid(pid, &status, 0);
	CHECK(rc == 0, "%s: wait for a signal from another process returned %d", what, rc);
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0, "%s: signalling child status %d", what, status);

	// the parent broadcasts to waiters in other processes
	s->flag = 0;
	s->ready = 0;
	for (int i = 0; i < 3; i++) {
		pids[i] = fork();
		if (pids[i] == 0) {
			_exit(child_wait_broadcast(s));
		}
	}
	for (int i = 0; i < 100; i++) {
		pthread_mutex_lock(&s->mutex);
		int ready = s->ready;
		pthread_mutex_unlock(&s->mutex);
		if (ready == 3) {
			break;
		}
		usleep(20000);
	}
	usleep(200000);
	pthread_mutex_lock(&s->mutex);
	__atomic_store_n(&s->flag, 1, __ATOMIC_RELEASE);
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->mutex);
	for (int i = 0; i < 3; i++) {
		waitpid(pids[i], &status, 0);
		CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0, "%s: broadcast waiter process %d status %d", what, i, status);
	}

	// mutual exclusion between processes
	s->counter = 0;
	for (int i = 0; i < 3; i++) {
		pids[i] = fork();
		if (pids[i] == 0) {
			_exit(child_increment(s));
		}
	}
	mutex_incrementer(s);
	for (int i = 0; i < 3; i++) {
		waitpid(pids[i], &status, 0);
	}
	CHECK(s->counter == 8000, "%s: counter is %d, expected 8000", what, s->counter);

	// a writer in another process waits for a reader here
	s->flag = 0;
	pthread_rwlock_rdlock(&s->rwlock);
	pid = fork();
	if (pid == 0) {
		_exit(child_writer(s));
	}
	usleep(300000);
	CHECK(__atomic_load_n(&s->flag, __ATOMIC_ACQUIRE) == 0, "%s: a writer in another process got the lock while a reader held it", what);
	pthread_rwlock_unlock(&s->rwlock);
	waitpid(pid, &status, 0);
	CHECK(__atomic_load_n(&s->flag, __ATOMIC_ACQUIRE) == 1, "%s: the writer did not run after the reader unlocked", what);
}

int main(void) {
	setvbuf(stdout, NULL, _IONBF, 0);

	// 1. process-shared primitives in private memory, between threads
	static struct shared private_shared;
	init_shared(&private_shared);
	test_threads(&private_shared);

	// 2. the same in anonymous shared memory, between threads and processes
	struct shared* anon = mmap(NULL, sizeof(*anon), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
	CHECK(anon != MAP_FAILED, "mmap(MAP_SHARED | MAP_ANON) failed: %s", strerror(errno));
	if (anon != MAP_FAILED) {
		init_shared(anon);
		test_threads(anon);
		test_processes(anon, "anonymous shared memory");
	}

	// 3. a shared memory object mapped by each process on its own, at a different address
	char name[32];
	snprintf(name, sizeof(name), "/pshared.%d", (int)getpid());
	int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
	CHECK(fd >= 0, "shm_open failed: %s", strerror(errno));
	if (fd >= 0) {
		shm_unlink(name);
		CHECK(ftruncate(fd, 4096) == 0, "ftruncate failed: %s", strerror(errno));
		struct shared* object = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		CHECK(object != MAP_FAILED, "mmap of the shared memory object failed: %s", strerror(errno));
		if (object != MAP_FAILED) {
			init_shared(object);

			// the child maps the object again, at another address (a spacer keeps the addresses apart)
			mmap(NULL, 1 << 20, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
			pid_t pid = fork();
			if (pid == 0) {
				struct shared* again = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
				if (again == MAP_FAILED || again == object) {
					_exit(20);
				}
				_exit(child_signal(again));
			}
			pthread_mutex_lock(&object->mutex);
			int rc = wait_flag(object, &object->cond, &object->flag);
			pthread_mutex_unlock(&object->mutex);
			int status;
			waitpid(pid, &status, 0);
			CHECK(rc == 0, "shared memory object: wait for a signal through a second mapping returned %d", rc);
			CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0, "shared memory object: child status %d", status);

			test_processes(object, "shared memory object");
		}
		close(fd);
	}

	printf("%s\n", failed ? "FAIL" : "PASS");
	return failed;
}
