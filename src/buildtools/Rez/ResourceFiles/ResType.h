#ifndef RESTYPE_H
#define RESTYPE_H

#include <string>
#include <iosfwd>

class ResType
{
    int x;
public:
    ResType() : x(0) {}
    ResType(int x) : x(x) {}
    ResType(const std::string& s);
    ResType(const char* s);

    operator int() const { return x; }
    bool operator<(ResType y) const { return x < y.x; }

    operator std::string();
};

std::ostream& operator<<(std::ostream& out, ResType t);

struct ResRef : public std::pair<ResType, short>
{
    ResRef() : std::pair<ResType, short>(ResType(), 0) {}
    ResRef(ResType t, int id) : std::pair<ResType, short>(t,id) {}

    ResType& type() { return first; }
    ResType type() const { return first; }
    short& id() { return second; }
    short id() const { return second; }
};

#endif // RESTYPE_H
