#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <string>
#include <iosfwd>
#include "location.hh"

class Diagnostic
{
public:
    enum Severity
    {
        warning,
        error,
        fatalError
    };

    Diagnostic();
    Diagnostic(Severity sev, std::string msg, yy::location loc);

private:
    Severity severity;
    std::string message;
    yy::location location;

    friend std::ostream& operator<<(std::ostream&, const Diagnostic&);
};

std::ostream& operator<<(std::ostream&, const Diagnostic&);

#endif // DIAGNOSTIC_H
