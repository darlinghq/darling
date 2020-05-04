#include "BinaryIO.h"
#include <iostream>
#include <cassert>

#include "ResType.h"

void byte(std::ostream& out, int byte)
{
    out.put((unsigned char)byte);
}
void word(std::ostream& out, int word)
{
    byte(out,(word >> 8) & 0xFF);
    byte(out,word & 0xFF);
}
void ostype(std::ostream& out, ResType type)
{
    longword(out, type);
}
void longword(std::ostream& out, int longword)
{
    byte(out,(longword >> 24) & 0xFF);
    byte(out,(longword >> 16) & 0xFF);
    byte(out,(longword >> 8) & 0xFF);
    byte(out,longword & 0xFF);
}

int byte(std::istream& in)
{
    return in.get() & 0xFF;
}
int word(std::istream& in)
{
    int a = byte(in);
    int b = byte(in);
    return (a << 8) | b;
}
ResType ostype(std::istream& in)
{
    return longword(in);
}
int longword(std::istream& in)
{
    int a = byte(in);
    int b = byte(in);
    int c = byte(in);
    int d = byte(in);
    return (a << 24) | (b << 16) | (c << 8) | d;
}

