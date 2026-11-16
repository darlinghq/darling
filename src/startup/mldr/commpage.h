#ifndef _COMMPAGE_H
#define _COMMPAGE_H
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void commpage_setup(bool _64bit);
void commpage_setup_arch(bool _64bit, bool x86_64_on_arm64);
unsigned long commpage_address(bool _64bit);
unsigned long commpage_address_arch(bool _64bit, bool x86_64_on_arm64);
unsigned long commpage_length(bool _64bit);
unsigned long commpage_length_arch(bool _64bit, bool x86_64_on_arm64);

#ifdef __cplusplus
}
#endif

#endif

