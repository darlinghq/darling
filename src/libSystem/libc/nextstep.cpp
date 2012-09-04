#include "config.h"
#include "nextstep.h"
#include <cstring>
#include <unistd.h>

char*** _NSGetEnviron()
{
	return &environ;
}
