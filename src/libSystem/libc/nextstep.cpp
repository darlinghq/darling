#include "config.h"
#include "nextstep.h"
#include <cstring>
#include <unistd.h>

static char** my_environ = environ;

char*** _NSGetEnviron()
{
	return &my_environ;
}
