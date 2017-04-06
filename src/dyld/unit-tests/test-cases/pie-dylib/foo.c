#include <stdio.h>
#include <stdbool.h>

void foo()
{
	printf("&foo=%p\n", &foo);
}
