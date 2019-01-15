#ifndef __MSGTRACER_CLIENT_H__
#define __MSGTRACER_CLIENT_H__

#include <asl.h>

extern void _msgtracer_log_with_keys(char *key1, int value1, char *key2, char *value2, char *key3, char *value3, void *end);

extern void msgtracer_log_with_keys(char *key1, int value1, char *things, ...);

#endif
