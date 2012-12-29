#include "xlocale.h"

#define WRAP_TRIVIAL(name, type) int name ## _l (wint_t wc, locale_t l) throw() { return UseLocale<int>(name, l, wc); }

template<typename T, typename Func, typename... Params> T UseLocale(Func func, locale_t l, Params... params)
{
	locale_t oldLoc = uselocale(l);

	T rv = func(params...);

	uselocale(oldLoc);
	return rv;
}

size_t mbstowcs_l(wchar_t* pwcs, const char* s, size_t n, locale_t l)
{
	return UseLocale<size_t>(mbstowcs, l, pwcs, s, n);
}

size_t wcswidth_l(wchar_t* pwcs, size_t n, locale_t l)
{
	return UseLocale<size_t>(wcswidth, l, pwcs, n);
}

int mbtowc_l(wchar_t* pwc, const char* s, size_t n, locale_t l)
{
	return UseLocale<int>(mbtowc, l, pwc, s, n);
}

size_t mbrtowc_l(wchar_t * pwc, const char * s, size_t n, mbstate_t * ps, locale_t l)
{
	return UseLocale<size_t>(mbrtowc, l, pwc, s, n, ps);
}

int mblen_l(const char *s, size_t n, locale_t l)
{
	return UseLocale<int>(mblen, l, s, n);
}

size_t wcrtomb_l(char * s, wchar_t wc, mbstate_t * ps, locale_t l)
{
	return UseLocale<size_t>(wcrtomb, l, s, wc, ps);
}

int iswnumber(wint_t wc) throw()
{
	return iswdigit(wc);
}

WRAP_TRIVIAL(iswalnum, WCTYPE_ALNUM);
WRAP_TRIVIAL(iswalpha, WCTYPE_ALPHA);
// WRAP_TRIVIAL(iswascii);
WRAP_TRIVIAL(iswblank, WCTYPE_BLANK);
WRAP_TRIVIAL(iswcntrl, WCTYPE_CNTRL);
WRAP_TRIVIAL(iswdigit, WCTYPE_DIGIT);
WRAP_TRIVIAL(iswgraph, WCTYPE_GRAPH);
// WRAP_TRIVIAL(iswhexnumber);
// WRAP_TRIVIAL(iswideogram);
WRAP_TRIVIAL(iswlower, WCTYPE_LOWER);
WRAP_TRIVIAL(iswnumber, WCTYPE_DIGIT);
// WRAP_TRIVIAL(iswphonogram);
WRAP_TRIVIAL(iswprint, WCTYPE_PRINT);
WRAP_TRIVIAL(iswpunct, WCTYPE_PUNCT);
// WRAP_TRIVIAL(iswrune);
WRAP_TRIVIAL(iswspace, WCTYPE_SPACE);
// WRAP_TRIVIAL(iswspecial);
WRAP_TRIVIAL(iswupper, WCTYPE_UPPER);
WRAP_TRIVIAL(iswxdigit, WCTYPE_XDIGIT);

int iswctype_l(wint_t wc, wctype_t charclass, locale_t loc) throw()
{
	return UseLocale<int>(iswctype, loc, wc, charclass);
}

wctype_t wctype_l(const char *property, locale_t loc) throw()
{
	return UseLocale<int>(wctype, loc, property);
}

