#ifndef RUNE_H
#define RUNE_H
#include <runetype.h>

typedef __darwin_rune_t rune_t;
typedef int __darwin_ct_rune_t;     /* ct_rune_t */

#define _INVALID_RUNE _DefaultRuneLocale.__invalid_rune

rune_t _none_sgetrune(const char *string, size_t n, char const **result);
int _none_sputrune(rune_t c, char *string, size_t n, char** result);

#define	_RUNETYPE_A	0x00000100
#define	_RUNETYPE_C	0x00000200
#define	_RUNETYPE_D	0x00000400
#define	_RUNETYPE_G	0x00000800
#define	_RUNETYPE_L	0x00001000
#define	_RUNETYPE_P	0x00002000
#define	_RUNETYPE_S	0x00004000
#define	_RUNETYPE_U	0x00008000
#define	_RUNETYPE_X	0x00010000
#define	_RUNETYPE_B	0x00020000
#define	_RUNETYPE_R	0x00040000
#define	_RUNETYPE_I	0x00080000
#define	_RUNETYPE_T	0x00100000
#define	_RUNETYPE_Q	0x00200000
#define _RUNETYPE_SW1 0x40000000

#endif
