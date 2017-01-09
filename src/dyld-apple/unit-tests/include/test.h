#include <stdarg.h>
#include <stdio.h>

#define DEFINE_TEST_FUNC(name) \
  static \
  inline \
  void \
  name(const char *format, ...) \
  { \
    va_list args; \
    va_start(args, format); \
    common(stdout, #name, format, args); \
    va_end(args); \
    return; \
  }

static
inline
void
common(FILE *file, const char *prefix, const char *format, va_list args)
{
  fprintf(file, "%s \"", prefix);
  vfprintf(file, format, args);
  fprintf(file, "\"\n"); // should check for trailing newline
  return;
}

DEFINE_TEST_FUNC(PASS);
DEFINE_TEST_FUNC(XPASS);
DEFINE_TEST_FUNC(FAIL);
DEFINE_TEST_FUNC(XFAIL);

DEFINE_TEST_FUNC(UNTESTED);
DEFINE_TEST_FUNC(UNSUPPORTED);
DEFINE_TEST_FUNC(UNRESOLVED);
