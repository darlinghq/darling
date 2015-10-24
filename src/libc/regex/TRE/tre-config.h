/* lib/tre-config.h.  Generated from tre-config.h.in by configure.  */
/* tre-config.h.in.  This file has all definitions that are needed in
   `tre.h'.  Note that this file must contain only the bare minimum
   of definitions without the TRE_ prefix to avoid conflicts between
   definitions here and definitions included from somewhere else. */

/* Define to 1 if you have the <libutf8.h> header file. */
/* #undef HAVE_LIBUTF8_H */

/* Define to 1 if the system has the type `reg_errcode_t'. */
/* #undef HAVE_REG_ERRCODE_T */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <wchar.h> header file. */
#define HAVE_WCHAR_H 1

/* Define if you want to enable approximate matching functionality. */
/* #undef TRE_APPROX */

/* Define to enable multibyte character set support. */
#define TRE_MULTIBYTE 1

/* Define to the absolute path to the system tre.h */
#define TRE_SYSTEM_REGEX_H_PATH <regex.h>

/* Define to include the system regex.h from tre.h */
#define TRE_USE_SYSTEM_REGEX_H 1

/* Define to enable wide character (wchar_t) support. */
#define TRE_WCHAR 1

/* TRE version string. */
#define TRE_VERSION "0.8.0"

/* TRE version level 1. */
#define TRE_VERSION_1 0

/* TRE version level 2. */
#define TRE_VERSION_2 8

/* TRE version level 3. */
#define TRE_VERSION_3 0
