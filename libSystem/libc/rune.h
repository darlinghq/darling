#ifndef RUNE_H
#define RUNE_H
#include <runetype.h>

typedef __darwin_rune_t rune_t;
typedef int __darwin_ct_rune_t;     /* ct_rune_t */

#define _INVALID_RUNE _DefaultRuneLocale.__invalid_rune

rune_t _none_sgetrune(const char *string, size_t n, char const **result);
int _none_sputrune(rune_t c, char *string, size_t n, char** result);


#endif
