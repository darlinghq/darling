#ifndef REZSPEC_H
#define REZSPEC_H

#include "ResType.h"
#include <string>

class ResSpec : public ResRef
{
    int attr_;
    std::string name_;

public:
    ResSpec() : attr_(0) {}
    ResSpec(ResType type, int id, int attr = 0, std::string name = "")
        : ResRef(type, id), attr_(attr), name_(name)
    {}

    int& attr() { return attr_; }
    int attr() const { return attr_; }
    std::string& name() { return name_; }
    const std::string& name() const { return name_; }
};

#endif // REZSPEC_H
