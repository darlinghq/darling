#ifndef RESOURCEFORK_H
#define RESOURCEFORK_H

#include <string>
#include <map>
#include "ResType.h"

class Resource
{
    ResType type;
    short id;
    std::string name;
    std::string data;
    int attr;
public:
    Resource() {}
    Resource(ResType type, int id, std::string data, std::string name = "", int attr = 0)
        : type(type), id(id), name(name), data(data), attr(attr) {}

    const std::string& getData() const { return data; }
    inline ResType getType() const { return type; }
    inline int getID() const { return id; }
    inline ResRef getTypeAndID() const { return ResRef(type, id); }
    std::string getName() const { return name; }
    int getAttr() const { return attr; }
};

class Fork
{
public:
     virtual void writeFork(std::ostream& out) const { }
     virtual ~Fork() {}
};

class Resources : public Fork
{
public:
    std::map<ResRef, Resource> resources;

    Resources() {}
    Resources(std::istream& in);
    void writeFork(std::ostream& out) const;
    void addResource(Resource res) { resources[res.getTypeAndID()] = res; }
    void addResources(const Resources& res);

    unsigned countResources() const { return resources.size(); }
};

#endif // RESOURCEFORK_H
