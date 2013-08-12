// http://stackoverflow.com/a/452955
#define _GLIBCXX_VISIBILITY(x) __attribute((visibility(#x)))
#define _GLIBCXX_BEGIN_NAMESPACE_VERSION
#define _GLIBCXX_END_NAMESPACE_VERSION
#include "basic_file_stdio.cc"
#include "fstream"
#include "fstream.tcc"

template class std::__basic_file<char>;
template class std::basic_filebuf<char, std::char_traits<char> >;
template class std::basic_streambuf<char, std::char_traits<char> >;
template class std::basic_ios<char, std::char_traits<char> >;
template class std::basic_ostream<char, std::char_traits<char> >;

