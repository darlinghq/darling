#ifndef CONFSTR_H
#define CONFSTR_H
#include <stddef.h>

extern "C" {

#define DARWIN__CS_DARWIN_USER_DIR			65536
#define DARWIN__CS_DARWIN_USER_TEMP_DIR		65537
#define DARWIN__CS_DARWIN_USER_CACHE_DIR	65538

size_t __darwin_confstr(int name, char* buf, size_t len);

}

#endif
