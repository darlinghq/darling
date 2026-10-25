#include <stdint.h>
#include <stdio.h>
extern void fixture_prepare(const void *, unsigned, unsigned);
extern int fixture_status(int);
extern uint64_t fixture_wide(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t,
                             uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
#define UPLOAD(name)                                                           \
  extern void name(unsigned, int, int, int, int, int, int, int, unsigned,      \
                   unsigned, const void *);
#define COMPRESSED(name)                                                       \
  extern void name(unsigned, int, int, int, int, int, int, int, unsigned, int, \
                   const void *);
UPLOAD(glTexSubImage3D)
UPLOAD(glTexSubImage3DEXT)
COMPRESSED(glCompressedTexSubImage3D)
COMPRESSED(glCompressedTexSubImage3DARB)
int main(void) {
  unsigned char bytes[32] = {0};
  int calls = 0;
  for (int round = 0; round < 5; round++) {
    const void *pointer = bytes + round;
    fixture_prepare(pointer, 1, 0x1401);
    glTexSubImage3D(1, 2, 3, 4, 5, 6, 7, 8, 0x1908, 0x1401, pointer);
    calls++;
    fixture_prepare(pointer, 2, 0x1401);
    glTexSubImage3DEXT(1, 2, 3, 4, 5, 6, 7, 8, 0x1908, 0x1401, pointer);
    calls++;
    fixture_prepare(pointer, 3, (unsigned)-101);
    glCompressedTexSubImage3D(1, 2, 3, 4, 5, 6, 7, 8, 0x1908, -101, pointer);
    calls++;
    fixture_prepare(pointer, 4, 32);
    glCompressedTexSubImage3DARB(1, 2, 3, 4, 5, 6, 7, 8, 0x1908, 32, pointer);
    calls++;
    if (fixture_status(calls)) {
      fprintf(stderr, "GL trailing arguments corrupted at round %d\n", round);
      return 1;
    }
  }
  uint64_t wide = UINT64_C(0x1122334455667788);
  if (fixture_wide(1, 2, 3, 4, 5, 6, 7, 8, wide, wide + 1) !=
      (UINT64_C(1) ^ 2 ^ 3 ^ 4 ^ 5 ^ 6 ^ 7 ^ 8 ^ wide ^ (wide + 1)))
    return 2;
  puts("Four GL signatures preserve register/stack arguments and pointers on "
       "cold/warm calls; unrelated 64-bit stack arguments pass");
  return 0;
}
