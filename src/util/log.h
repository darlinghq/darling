#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#include <cassert>
#include <cstdio>
#include <iostream>

extern "C" bool g_loggingEnabled;

#define LOG  if (g_loggingEnabled) std::cerr
#define LOGF(...) if (g_loggingEnabled) fprintf(stderr, __VA_ARGS__)

#endif
