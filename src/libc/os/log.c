#include <os/log.h>

#include <stdio.h>
#include <stdarg.h>

struct os_log_s
{
	int stub;
};

struct os_log_s _os_log_disabled;

bool os_log_type_enabled(os_log_t oslog, os_log_type_t type)
{
	printf("os_log_enabled stub called\n");
	return 1;
}

void _os_log_impl(void *dso,
		os_log_t log,
		os_log_type_t type,
		const char *format,
		uint8_t *buf,
		uint32_t size)
{
	printf("_os_log_impl called: %s\n", format);
}

void _os_log_error_impl(void *dso, os_log_t log, os_log_type_t type,
	const char *format, uint8_t *buf, uint32_t size)
{
	printf("_os_log_error_impl called: %s\n", format);
}

void _os_log_fault_impl(void *dso, os_log_t log, os_log_type_t type,
        const char *format, uint8_t *buf, uint32_t size)
{
	printf("_os_log_fault_impl called: %s\n", format);
}

void _os_log_internal(void *dso, os_log_t log, os_log_type_t type, const char *message, ...)
{
	va_list ap;
	printf("_os_log_internal called: ");
	va_start(ap, message);
	vprintf(message, ap);
	va_end(ap);
	printf("\n");
}

os_log_t _os_log_create(void *dso, const char *subsystem, const char *category)
{
	printf("_os_log_create called: %s, %s\n", subsystem, category);
	return NULL;
}

bool os_log_is_enabled(os_log_t log)
{
	printf("os_log_is_enabled called\n");
	return 1;
}

bool os_log_is_debug_enabled(os_log_t log)
{
	printf("os_log_is_debug_enabled called\n");
	return 1;
}
