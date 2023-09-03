#ifndef XTRACE_BSD_TRACE
#define XTRACE_BSD_TRACE

#include "base.h"
#include "string.h"

#ifdef XTRACE_CPP
extern void print_open_flags(xtrace::String* log, void* arg);
extern void xtrace_print_string_literal(xtrace::String* log, const char* str);
#endif

#endif // XTRACE_BSD_TRACE
