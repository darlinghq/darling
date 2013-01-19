#include <libproc.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void TEST(const char* name, int r);
void test1();
void test2();
void test3();

int main()
{
	test1();
	test2();
	test3();

	return 0;
}

void TEST(const char* name, int r)
{
	printf("%s: %s\n", name, r ? "[OK]" : "[FAIL]");
}

void test1()
{
	char name[1024];
	int rv = proc_name(getpid(), name, sizeof name);

	TEST("proc_name() retval", rv != 0);
	TEST("proc_name() strstr", strstr(name, "libproc") != NULL);
}

void test2()
{
	pid_t pids[4096];
	int rv;
	int found = 0;

	rv = proc_listpids(PROC_UID_ONLY, getuid(), pids, sizeof pids);
	TEST("proc_listpids() retval", rv != 0);

	for (int i = 0; i < rv; i++)
	{
		if (pids[i] == getpid())
		{
			found = 1;
			break;
		}
	}

	TEST("proc_listpids() PROC_UID_ONLY", found);
}

void test3()
{
	char testfile[] = "/tmp/testcaseXXXXXX";
	FILE* file;
	pid_t pids[4096];
	int rv;
	int found = 0, i = 0;

	mkstemp(testfile);
	file = fopen(testfile, "w");

	rv = proc_listpidspath(PROC_ALL_PIDS, 0, testfile, 0, pids, sizeof pids);
	TEST("proc_listpidspath retval", rv > 0);
	TEST("proc_listpidspath pid", pids[0] == getpid());

	fclose(file);
	unlink(testfile);
}

