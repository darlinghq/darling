#ifndef CALCULATE_H
#define CALCULATE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool CalculateExpression(const char *expr, int base, int flags, char *outBuffer, int bufferSize);

#ifdef __cplusplus
}
#endif

#endif
