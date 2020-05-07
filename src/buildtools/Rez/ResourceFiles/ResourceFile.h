#ifndef RESOURCEFILE_H
#define RESOURCEFILE_H

#include <memory>
#include <iosfwd>
#include <string>

#include "ResType.h"
#include "ResourceFork.h"

class ResourceFile
{
public:
    enum class Format
    {
        autodetect,
#ifdef __APPLE__
        real,
#endif
        macbin,
        diskimage,
        basilisk,
        applesingle,
        underscore_appledouble,
        percent_appledouble
    };

    bool read(std::string path, Format f = Format::autodetect);
    bool write(std::string path, Format f = Format::autodetect);
    bool read(std::istream& in, Format f);
    bool write(std::ostream& in, Format f);
    static bool hasPlainDataFork(Format f);
    bool hasPlainDataFork();
    Format getFormat() { return format; }

    static bool isSingleFork(Format f);

    ResType type;
    ResType creator;
    Resources resources;
    std::string data;

private:
    bool assign(std::string path, Format f = Format::autodetect);
    bool read();
    bool write();

    std::string pathstring;
    std::string filename;
    Format format = Format::autodetect;
};

#endif // RESOURCEFILE_H
