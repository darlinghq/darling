#include <syslog.h>
#include <stdarg.h>

void syslog$DARWIN_EXTSN(int priority, const char *arg1, ...)
{
	const char *format;
	va_list args;
	va_start(args, arg1);
	vsyslog(priority, arg1, args);
	va_end(args);
}
