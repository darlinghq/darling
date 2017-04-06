#include <stdio.h>
#include <stdlib.h>


extern int foo();
extern int bar();

static __attribute__((constructor)) void main_init()
{
    printf("main_init\n");
}

static __attribute__((destructor)) void main_term()
{
    printf("main_term\n");
}


int main()
{
    printf("main\n");
	foo();
	return 0;
}

