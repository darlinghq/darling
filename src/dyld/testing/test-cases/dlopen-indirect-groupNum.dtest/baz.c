
#include <stdio.h>
#include <string.h>

extern void barInDylib();

void bazInDylib()
{
	return barInDylib();
}
