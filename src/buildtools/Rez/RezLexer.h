#ifndef REZLEXER_H
#define REZLEXER_H

#include <memory>
#include <string>

#include "location.hh"

class RezSymbol;
class RezWorld;

class RezLexer
{
    RezWorld& world;
    struct Priv;
    std::unique_ptr<Priv> pImpl;

    std::string curFile;
    yy::location lastLocation;

    class WaveToken;

    bool atEnd();
    WaveToken nextWave();
    WaveToken peekWave();

public:
    RezLexer(RezWorld& world, std::string filename);
    RezLexer(RezWorld& world, std::string filename, const std::string& data);
    ~RezLexer();

    RezSymbol nextToken();

    void addDefine(std::string str);
    void addIncludePath(std::string path);
};

#endif // REZLEXER_H
