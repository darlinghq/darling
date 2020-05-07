#ifndef _OS_CPP_UTIL_H
#define _OS_CPP_UTIL_H

#include <sys/cdefs.h>

#if __has_feature(cxx_nullptr) && __has_feature(cxx_decltype)
# define OS_HAS_NULLPTR 1
#endif

#if __has_feature(cxx_rvalue_references) || __has_extension(cxx_rvalue_references)
# define OS_HAS_RVALUE_REFERENCES 1
#endif

namespace os {
#if OS_HAS_NULLPTR
typedef decltype(nullptr) nullptr_t;
#endif

/*
 * Reference removal
 */

template <class _T> struct remove_reference       {typedef _T type;};
template <class _T> struct remove_reference<_T&>  {typedef _T type;};
template <class _T> struct remove_reference<_T &&> {typedef _T type;};
template <class _T> using remove_reference_t = typename remove_reference<_T>::type;

/*
 * Const removal
 */

template <class _T> struct remove_const           {typedef _T type;};
template <class _T> struct remove_const<const _T> {typedef _T type;};
template <class _T> using remove_const_t = typename remove_const<_T>::type;

/*
 * Move
 */

template <class _T>
inline typename remove_reference<_T>::type &&
move(_T && _t)
{
	typedef typename os::remove_reference<_T>::type _U;
	return static_cast<_U &&>(_t);
}
}

#endif /* _OS_CPP_UTIL_H */
