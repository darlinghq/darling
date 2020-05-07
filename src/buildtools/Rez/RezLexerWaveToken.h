#ifndef REZLEXERWAVETOKEN_H
#define REZLEXERWAVETOKEN_H

#include "RezLexer.h"

#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>

class RezLexer::WaveToken : public boost::wave::cpplexer::lex_token<>
{
public:
    WaveToken() = default;
    WaveToken(const boost::wave::cpplexer::lex_token<> & o) : boost::wave::cpplexer::lex_token<>(o) {}
};

#endif // REZLEXERWAVETOKEN_H
