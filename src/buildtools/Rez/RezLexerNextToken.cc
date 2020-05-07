#include "RezLexer.h"
#include "RezLexerWaveToken.h"
#include "RezParser.generated.hh"
#include <unordered_map>

#include <boost/regex.hpp>

using namespace boost::wave;

static int readInt(const char *str, const char *end = NULL, int baseOverride = 0)
{
    int x = 0;

    int base = 10;

    if(baseOverride)
        base = baseOverride;
    else if(*str == '0')
    {
        base = 8;
        ++str;
        if(*str == 'x' || *str == 'X')
        {
            base = 16;
            ++str;
        }
        if(*str == 'b' || *str == 'B')
        {
            base = 2;
            ++str;
        }
    }
    else if(*str == 'b' || *str == 'B')
    {
        base = 2;
        ++str;
    }

    while(str != end && *str)
    {
        x *= base;
        if(*str >= 'a' && *str <= 'z')
            x += *str - 'a' + 10;
        else if(*str >= 'A' && *str <= 'Z')
            x += *str - 'A' + 10;
        else if(*str >= '0' && *str <= '9')
            x += *str - '0';
        str++;
    }

    return x;
}

static int readCharLit(const char *str)
{
    const char *p = str + 1;
    const char *e = str + strlen(str) - 1;

    if(e - p != 4)
        std::cout << "warning: CHAR LITERAL " << str << "\n";

    int x = 0;
    while(p != e)
    {
        x <<= 8;
        x |= (*p) & 0xFF;
        ++p;
    }
    return x;
}

static std::string readStringLit(const char *str)
{
    const char *p = str + 1;
    const char *e = str + strlen(str) - 1;

    std::ostringstream out;

    while(p != e)
    {
        if(*p == '\\')
        {
            ++p;
            if(p != e)
            {
                switch(*p)
                {
                    case 'n':
                        out << '\n'; ++p;
                        break;
                    case 'r':
                        out << '\r'; ++p;
                        break;
                    case 't':
                        out << '\t'; ++p;
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                        if(p + 3 > e)
                            continue;
                        if(p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
                        {
                            if(p + 4 > e)
                                continue;
                            out << (char)readInt(p+2, p+4, 16);
                            p += 4;
                        }
                        else
                        {
                            out << (char)readInt(p, p+3, 8);
                            p += 3;
                        }
                        break;
                    case '$':
                        {
                            if(p + 3 > e)
                                continue;
                            out << (char)readInt(p+1, p+3, 16);
                            p += 3;
                        }
                        break;
                }
            }
        }
        else
        {
            out << *p++;
        }
    }

    return out.str();
}

RezSymbol RezLexer::nextToken()
{
    for(auto tok = nextWave(); tok != T_EOI && tok != T_EOF; tok = nextWave())
    {
        if(IS_CATEGORY(tok, WhiteSpaceTokenType))
            continue;
        else if(IS_CATEGORY(tok, EOLTokenType))
            continue;
        else if(tok == T_PP_LINE)
        {
            while(tok != T_EOI && tok != T_EOF && !IS_CATEGORY(tok, EOLTokenType))
                tok = nextWave();
            continue;
        }
        else
        {
            //std::cout << "{" << std::hex << (token_id)tok << std::dec << "|" << tok.get_value() << "}\n";

            auto pos = tok.get_position();
            curFile = pos.get_file().c_str();
            auto yypos = yy::position(&curFile, pos.get_line(), pos.get_column());
            yy::location loc(yypos);
            lastLocation = loc;

            if(tok == (UnknownTokenType | '"'))
            {
                return RezParser::make_STRINGLIT("Hello, world.", loc);
            }
            else if(IS_CATEGORY(tok, IdentifierTokenType) || IS_CATEGORY(tok, KeywordTokenType) || IS_CATEGORY(tok, BoolLiteralTokenType))
            {
                typedef decltype(&RezParser::make_TYPE) memfun;
#define KEYWORD(upper, lower) \
{ lower, &RezParser::make_ ## upper }

                static std::unordered_map<std::string, memfun> keywords = {
                    KEYWORD(TYPE, "type"),
                    KEYWORD(RESOURCE, "resource"),
                    KEYWORD(DATA, "data"),
                    KEYWORD(READ, "read"),
                    KEYWORD(INCLUDE, "include"),
                    KEYWORD(CHANGE, "change"),
                    KEYWORD(DELETE, "delete"),

                    KEYWORD(ARRAY,"array"),
                    KEYWORD(SWITCH, "switch"),
                    KEYWORD(CASE, "case"),
                    KEYWORD(AS, "as"),
                    KEYWORD(FILL,"fill"),
                    KEYWORD(ALIGN, "align"),
                    KEYWORD(HEX,"hex"),
                    KEYWORD(KEY, "key"),
                    KEYWORD(WIDE,"wide"),
                    KEYWORD(UNSIGNED, "unsigned"),
                    KEYWORD(BINARY, "binary"),
                    KEYWORD(LITERAL, "literal"),
                    KEYWORD(BOOLEAN, "boolean"),
                    KEYWORD(BIT, "bit"),
                    KEYWORD(NIBBLE, "nibble"),
                    KEYWORD(BYTE, "byte"),
                    KEYWORD(CHAR, "char"),
                    KEYWORD(WORD, "word"),
                    KEYWORD(INTEGER, "integer"),
                    KEYWORD(LONG, "long"),
                    KEYWORD(LONGINT, "longint"),
                    KEYWORD(PSTRING, "pstring"),
                    KEYWORD(PSTRING, "wstring"),
                    KEYWORD(STRING, "string"),
                    KEYWORD(POINT, "point"),
                    KEYWORD(RECT, "rect"),
                    KEYWORD(BITSTRING, "bitstring"),

                    KEYWORD(INTEGER, "int"),
                    KEYWORD(DOLLAR, "$"),

                    KEYWORD(FUN_COUNTOF, "$$countof"),
                    KEYWORD(FUN_ARRAYINDEX, "$$arrayindex"),
                    KEYWORD(FUN_READ, "$$read"),
                    KEYWORD(FUN_BITFIELD, "$$bitfield"),
                    KEYWORD(FUN_WORD, "$$word"),
                    KEYWORD(FUN_BYTE, "$$byte"),
                    KEYWORD(FUN_LONG, "$$long"),
                };

                std::string s = tok.get_value().c_str();
                if(s.size() >= 2 && s[0] == '$' && std::all_of(s.begin()+1, s.end(), [](char c) { return isxdigit(c); }))
                    return RezParser::make_INTLIT(readInt(s.c_str()+1, nullptr, 16), loc);

                std::string lower = s;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                auto p = keywords.find(lower);
                if(p == keywords.end())
                {
                    //std::cout << "id: " << s << std::endl;
                    return RezParser::make_IDENTIFIER(lower, loc);
                }
                else
                {
                    //std::cout << "key: " << s << std::endl;
                    return (*p->second)(loc);
                }
            }
            else if(tok == T_INTLIT)
            {
                if(tok.get_value() == "0")
                {
                    auto tok2 = peekWave();
                    while(tok2 != T_EOI && tok2 != T_EOF && IS_CATEGORY(tok2, WhiteSpaceTokenType))
                        nextWave(), tok2 = peekWave();

                    //std::cout << "!" << std::hex << (token_id)tok2 << std::dec << "|" << tok2.get_value() << "!\n";
                    static boost::regex binlit("[bB][01]+");
                    if(tok2 == T_IDENTIFIER && boost::regex_match(tok2.get_value().c_str(), binlit))
                        tok = nextWave();
                }
                return RezParser::make_INTLIT(readInt(tok.get_value().c_str()), loc);
            }
            else
            {
#define NOVAL_TOK(name)    \
case T_ ## name: /*std::cout << #name << std::endl;*/ return RezParser::make_ ## name(loc)
                switch(token_id(tok))
                {
                    case T_INTLIT: return RezParser::make_INTLIT(readInt(tok.get_value().c_str()), loc);

                    case T_CHARLIT: return RezParser::make_CHARLIT(readCharLit(tok.get_value().c_str()), loc);
                    case T_STRINGLIT: return RezParser::make_STRINGLIT(readStringLit(tok.get_value().c_str()), loc);

                    NOVAL_TOK(LEFTBRACE);
                    NOVAL_TOK(RIGHTBRACE);
                    NOVAL_TOK(LEFTBRACKET);
                    NOVAL_TOK(RIGHTBRACKET);
                    NOVAL_TOK(LEFTPAREN);
                    NOVAL_TOK(RIGHTPAREN);
                    NOVAL_TOK(SEMICOLON);
                    NOVAL_TOK(COMMA);
                    NOVAL_TOK(PLUS);
                    NOVAL_TOK(MINUS);
                    NOVAL_TOK(DIVIDE);
                    NOVAL_TOK(STAR);
                    NOVAL_TOK(ASSIGN);
                    NOVAL_TOK(COLON);
                    NOVAL_TOK(SHIFTLEFT);
                    NOVAL_TOK(SHIFTRIGHT);
                    NOVAL_TOK(EQUAL);
                    NOVAL_TOK(NOTEQUAL);
                    NOVAL_TOK(AND);
                    NOVAL_TOK(OR);
                    NOVAL_TOK(XOR);
                    NOVAL_TOK(COMPL);

                    default:

                        return RezParser::make_BADTOKEN(tok.get_value().c_str(), loc);
                }

            }
        }
    }
    return RezParser::symbol_type(RezParser::token_type(0), yy::location());
}
