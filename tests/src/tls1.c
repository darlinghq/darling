#include <stdio.h>
#include <pthread.h>

__thread static int var = 123;

void* printThread(void* v);

int main()
{
	printf("main: var = %d\n", var);
	var = 321;
	printf("main: var = %d\n", var);

	pthread_t pth;
	pthread_create(&pth, NULL, printThread, NULL);
	pthread_join(pth, NULL);

	return 0;
}

void* printThread(void* v)
{
	printf("thread: var = %d\n", var);
	return NULL;
}

