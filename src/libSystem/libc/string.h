#ifndef LIBC_STRING_H
#define LIBC_STRING_H
#include <cstring>

extern "C"
{

// size_t strlcpy(char* dst, const char* src, size_t size); // in libbsd
// size_t strlcat(char* dst, const char* src, size_t size);
void memset_pattern4(char* b, const char* pattern4, size_t len);
void memset_pattern8(char* b, const char* pattern4, size_t len);
void memset_pattern16(char* b, const char* pattern4, size_t len);

int __mb_cur_max();
int ___mb_cur_max();
int __toupper(int c);
int __tolower(int c);


}

#endif
