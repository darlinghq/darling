#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * Parent and child handlers are called in the order they were registered
 * prepare handlers are called in reverse order. The non-commutative
 * operations will ensure that we are calling them in the proper order.
 */

static int parentval = 0;
static int childval = 0;

static void prepare1(void)
{
	parentval *= 2;
}

static void prepare2(void)
{
	parentval = 3;
}

static void parent1(void)
{
	parentval += 4;
}

static void parent2(void)
{
	parentval *= 3;
}

static void child1(void)
{
	childval = 5;
}

static void child2(void)
{
	childval *= 3;
}

int
main(void)
{
	pid_t pid, child;
	int status;

	assert(!pthread_atfork(prepare1, parent1, child1));
	assert(!pthread_atfork(prepare2, parent2, child2));
	pid = fork();
	assert(pid >= 0);
	if (pid == 0) {
		_exit(childval);
	} else {
		child = waitpid(pid, &status, 0);
		assert(child == pid);
		assert(WIFEXITED(status));
		assert(WEXITSTATUS(status) == 15);
		assert(parentval == 30);
	}
	return 0;
}
