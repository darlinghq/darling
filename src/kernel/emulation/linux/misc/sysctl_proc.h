#ifndef _SYSCTL_PROC_H
#define _SYSCTL_PROC_H
#include <stdbool.h>

// Utility functions for parsing Linux procfs /proc/XX/stat
bool read_string(const char* path, char* dst, int maxlen);
const char* next_stat_elem(char** buf);
void skip_stat_elems(char** buf, int count);

#endif

