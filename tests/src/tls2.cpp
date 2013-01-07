#include <stdio.h>
#include <pthread.h>

void* printThread(void* v);

struct SimpleStruct
{
	SimpleStruct();
	~SimpleStruct();

	int val;
};

__thread static SimpleStruct var;

int main()
{
	printf("main: var = %d\n", var.val);
	var.val = 321;
	printf("main: var = %d\n", var.val);

	pthread_t pth;
	pthread_create(&pth, NULL, printThread, NULL);
	pthread_join(pth, NULL);

	return 0;
}

SimpleStruct::SimpleStruct()
{
	puts("Constructor called");
	val = 123;
}

SimpleStruct::~SimpleStruct()
{
	puts("Destructor called");
}

void* printThread(void* v)
{
	printf("thread: var = %d <- this seems to be broken on all platforms\n", var.val);
	return NULL;
}

