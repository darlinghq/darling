#include <stdbool.h> 



extern bool bar1();


__attribute__((weak)) bool bar1()
{
	return false;
}

__attribute__((weak)) bool bar2()
{
	return true;
}


bool foo1()
{
	return bar1();
}

bool foo2()
{
	return bar2();
}


