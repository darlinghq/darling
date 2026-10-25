#include <stdint.h>
static const void *expected_pointer;
static unsigned expected_kind, expected_second;
static int failed, calls;
void fixture_prepare(const void *pointer, unsigned kind, unsigned second) {
  expected_pointer = pointer;
  expected_kind = kind;
  expected_second = second;
}
static void check(unsigned kind, unsigned a, int b, int c, int d, int e, int f,
                  int g, int h, unsigned format, unsigned second,
                  const void *pointer) {
  calls++;
  if (kind != expected_kind || a != 1 || b != 2 || c != 3 || d != 4 || e != 5 ||
      f != 6 || g != 7 || h != 8 || format != 0x1908 ||
      second != expected_second || pointer != expected_pointer)
    failed = 1;
}
#define UPLOAD(name, kind)                                                     \
  void name(unsigned a, int b, int c, int d, int e, int f, int g, int h,       \
            unsigned format, unsigned type, const void *pointer) {             \
    check(kind, a, b, c, d, e, f, g, h, format, type, pointer);                \
  }
#define COMPRESSED(name, kind)                                                 \
  void name(unsigned a, int b, int c, int d, int e, int f, int g, int h,       \
            unsigned format, int size, const void *pointer) {                  \
    check(kind, a, b, c, d, e, f, g, h, format, (unsigned)size, pointer);      \
  }
UPLOAD(glTexSubImage3D, 1)
UPLOAD(glTexSubImage3DEXT, 2)
COMPRESSED(glCompressedTexSubImage3D, 3)
COMPRESSED(glCompressedTexSubImage3DARB, 4)
int fixture_status(int expected_calls) {
  return failed || calls != expected_calls;
}
uint64_t fixture_wide(uint64_t a, uint64_t b, uint64_t c, uint64_t d,
                      uint64_t e, uint64_t f, uint64_t g, uint64_t h,
                      uint64_t i, uint64_t j) {
  return a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j;
}
