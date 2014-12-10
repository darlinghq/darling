#ifndef LIBC_MULTIBYTE_H
#define LIBC_MULTIBYTE_H
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

extern "C" {

size_t mbstowcs_l(wchar_t* pwcs, const char* s, size_t n, locale_t l);
size_t wcswidth_l(wchar_t* pwcs, size_t n, locale_t l);
int mbtowc_l(wchar_t * pwc, const char * s, size_t n, locale_t l);
size_t mbrtowc_l(wchar_t * pwc, const char * s, size_t n, mbstate_t * ps, locale_t l);
int mblen_l(const char *s, size_t n, locale_t l);
size_t wcrtomb_l(char * s, wchar_t wc, mbstate_t * ps, locale_t l);

int iswalnum_l(wint_t wc, locale_t loc) throw();
int iswalpha_l(wint_t wc, locale_t loc) throw();
int iswascii_l(wint_t wc, locale_t loc) throw();
int iswblank_l(wint_t wc, locale_t loc) throw();
int iswcntrl_l(wint_t wc, locale_t loc) throw();
int iswdigit_l(wint_t wc, locale_t loc) throw();
int iswgraph_l(wint_t wc, locale_t loc) throw();
int iswhexnumber_l(wint_t wc, locale_t loc) throw();
int iswideogram_l(wint_t wc, locale_t loc) throw();
int iswlower_l(wint_t wc, locale_t loc) throw();
int iswnumber_l(wint_t wc, locale_t loc) throw();
int iswphonogram_l(wint_t wc, locale_t loc) throw();
int iswprint_l(wint_t wc, locale_t loc) throw();
int iswpunct_l(wint_t wc, locale_t loc) throw();
int iswrune_l(wint_t wc, locale_t loc) throw();
int iswspace_l(wint_t wc, locale_t loc) throw();
int iswspecial_l(wint_t wc, locale_t loc) throw();
int iswupper_l(wint_t wc, locale_t loc) throw();
int iswxdigit_l(wint_t wc, locale_t loc) throw();
int iswnumber(wint_t wc) throw();

int iswctype_l(wint_t wc, wctype_t charclass, locale_t loc) throw();
wctype_t wctype_l(const char *property, locale_t loc) throw();

int ___mb_cur_max_l(locale_t loc);
int ___mb_cur_max(void);


/*
TODO:

<ctype.h>   digittoint_l(3), isalnum_l(3), isalpha_l(3), isblank_l(3), iscntrl_l(3), isdigit_l(3), isgraph_l(3), ishexnumber_l(3), isideogram_l(3), islower_l(3), isnumber_l(3),
            isphonogram_l(3), isprint_l(3), ispunct_l(3), isrune_l(3), isspace_l(3), isspecial_l(3), isupper_l(3), isxdigit_l(3), tolower_l(3), toupper_l(3)

<inttypes.h>
            strtoimax_l(3), strtoumax_l(3), wcstoimax_l(3), wcstoumax_l(3)

<langinfo.h>
            nl_langinfo_l(3)

<monetary.h>
            strfmon_l(3)

<stdio.h>   asprintf_l(3), fprintf_l(3), fscanf_l(3), printf_l(3), scanf_l(3), snprintf_l(3), sprintf_l(3), sscanf_l(3), vasprintf_l(3), vfprintf_l(3), vfscanf_l(3), vprintf_l(3),
            vscanf_l(3), vsnprintf_l(3), vsprintf_l(3), vsscanf_l(3)

<stdlib.h>  atof_l(3), atoi_l(3), atol_l(3), atoll_l(3), strtod_l(3), strtof_l(3), strtol_l(3), strtold_l(3), strtoll_l(3), strtoq_l(3),
            strtoul_l(3), strtoull_l(3), strtouq_l(3), wcstombs_l(3), wctomb_l(3)

<string.h>  strcoll_l(3), strxfrm_l(3), strcasecmp_l(3), strcasestr_l(3), strncasecmp_l(3)

<time.h>    strftime_l(3), strptime_l(3)

<wchar.h>   btowc_l(3), fgetwc_l(3), *fgetws_l(3), fputwc_l(3), fputws_l(3), fwprintf_l(3), fwscanf_l(3), getwc_l(3), getwchar_l(3), mbrlen_l(3), mbsinit_l(3),
            mbsnrtowcs_l(3), mbsrtowcs_l(3), putwc_l(3), putwchar_l(3), swprintf_l(3), swscanf_l(3), ungetwc_l(3), vfwprintf_l(3), vfwscanf_l(3), vswprintf_l(3), vswscanf_l(3),
            vwprintf_l(3), vwscanf_l(3), wcscoll_l(3), wcsftime_l(3), wcsnrtombs_l(3), wcsrtombs_l(3), wcstod_l(3), wcstof_l(3), wcstol_l(3), wcstold_l(3),
            wcstoll_l(3), wcstoul_l(3), wcstoull_l(3), wcswidth_l(3), wcsxfrm_l(3), wctob_l(3), wcwidth_l(3), wprintf_l(3), wscanf_l(3)

<wctype.h>  iswblank_l(3), iswhexnumber_l(3), iswideogram_l(3), iswnumber_l(3), iswphonogram_l(3), iswrune_l(3), iswspecial_l(3), nextwctype_l(3), towctrans_l(3), wctrans_l(3)

<xlocale.h>
            localeconv_l(3)

*/

}

#endif

