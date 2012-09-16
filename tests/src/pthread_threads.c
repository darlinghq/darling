// CFLAGS: -lpthread
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void* mythread(void*);

int main()
{
	pthread_t th, th2;
	void* rv;
	pthread_create(&th, 0, mythread, 0);
	sleep(1);
	pthread_create(&th2, 0, mythread, 0);
	printf("Waiting for thread...\n");
	pthread_join(th, &rv);
	pthread_join(th2, &rv);
	printf("Thread joined!\n");
	return 0;
}

void* mythread(void* p)
{
	sleep(2);
}

