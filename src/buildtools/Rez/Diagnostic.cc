#include "Diagnostic.h"

Diagnostic::Diagnostic()
{
}

Diagnostic::Diagnostic(Severity sev, std::string msg, yy::location loc)
    : severity(sev), message(msg), location(loc)
{

}


std::ostream &operator<<(std::ostream &out, const Diagnostic &d)
{
    //return out << d.location << ": " << d.message;
    const yy::location& loc = d.location;
    if (loc.begin.filename)
      out << *loc.begin.filename << ':';
    out << loc.begin.line << ':' << loc.begin.column;
    out << ": ";
    if(d.severity >= Diagnostic::error)
        out << "error";
    else
        out << "warning";
    out << ": " << d.message;
    return out;
}
