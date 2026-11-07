#ifndef _COMMPAGE_H
#define _COMMPAGE_H
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void commpage_setup(bool _64bit);
unsigned long commpage_address(bool _64bit);
unsigned long commpage_length(bool _64bit);

#ifdef __cplusplus
}
#endif

#endif

