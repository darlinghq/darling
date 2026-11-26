// TASK_DYLD_INFO in a forked child, as observed on macOS 26.7.3 x86_64: the child, which runs the parent's image
// in a copy of its address space, reports the same dyld image information for its own task as the parent does.
// Where task_for_pid of the child is allowed (macOS allows it only to privileged callers, so this part was not
// observed there), the test also checks that the parent gets that information for the child's task.
#include <mach/mach.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static int failures = 0;

static void check(int ok, const char *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static kern_return_t dyld_info(task_t task, struct task_dyld_info *info)
{
	mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
	return task_info(task, TASK_DYLD_INFO, (task_info_t)info, &count);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	struct task_dyld_info parent;
	check(dyld_info(mach_task_self(), &parent) == KERN_SUCCESS && parent.all_image_info_addr != 0, "the parent has dyld image information");

	int pipefd[2];
	pipe(pipefd);
	pid_t child = fork();
	if (child == 0) {
		struct task_dyld_info own;
		kern_return_t kr = dyld_info(mach_task_self(), &own);
		int same = kr == KERN_SUCCESS && own.all_image_info_addr == parent.all_image_info_addr
			&& own.all_image_info_size == parent.all_image_info_size && own.all_image_info_format == parent.all_image_info_format;
		write(pipefd[1], &same, sizeof(same));
		pause();
		_exit(0);
	}
	alarm(30);
	int same = 0;
	read(pipefd[0], &same, sizeof(same));
	check(same, "a forked child reports its parent's dyld image information for its own task");

	// asking for another task needs task_for_pid, which macOS allows only to privileged callers
	mach_port_t task = MACH_PORT_NULL;
	if (task_for_pid(mach_task_self(), child, &task) == KERN_SUCCESS) {
		struct task_dyld_info info;
		check(dyld_info(task, &info) == KERN_SUCCESS && info.all_image_info_addr == parent.all_image_info_addr,
			"the parent gets the same dyld image information for the child's task");
	} else {
		printf("SKIP: task_for_pid of the child is not allowed\n");
	}
	kill(child, SIGKILL);
	waitpid(child, NULL, 0);
	printf(failures ? "FAILED\n" : "ALL PASSED\n");
	return failures != 0;
}
