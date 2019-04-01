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

typedef float __attribute__((ext_vector_type(4))) simd_float4;
typedef struct { simd_float4 columns[4]; } simd_float4x4;

// TODO
simd_float4x4 matrix_identity_float4x4;
