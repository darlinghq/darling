#include <syslog.h>
#include <stdarg.h>

typedef float __attribute__((ext_vector_type(4))) simd_float4;
typedef struct { simd_float4 columns[4]; } simd_float4x4;

// TODO
simd_float4x4 matrix_identity_float4x4;
