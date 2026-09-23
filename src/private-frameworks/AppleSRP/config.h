/* libsrp build configuration for Darling: OpenSSL math and SHA-1 from LibreSSL. */
#define STDC_HEADERS 1
#define HAVE_UNISTD_H 1
#define HAVE_SYS_TIME_H 1
#define TIME_WITH_SYS_TIME 1
#define OPENSSL 1
#define OPENSSL_SHA 1
#define PEDANTIC_ARGS 1

/* t_math.c calls ENGINE_load_builtin_engines() without including engine.h unless OPENSSL_ENGINE is set. */
#include <openssl/engine.h>
