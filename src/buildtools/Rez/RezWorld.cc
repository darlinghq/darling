#include "RezWorld.h"
#include "ResourceCompiler.h"
#include "ResourceFork.h"

#include <iostream>

#include "Diagnostic.h"

RezWorld::RezWorld()
    : verboseFlag(false), hadErrors(false)
{
}

void RezWorld::addTypeDefinition(TypeSpec spec, TypeDefinitionPtr type)
{
    if(!type)
        return;
    types[spec] = type;
}

TypeDefinitionPtr RezWorld::getTypeDefinition(ResType type, int id, yy::location loc)
{
    auto p = types.find(TypeSpec(type, id));
    if(p != types.end())
        return p->second;
    p = types.find(TypeSpec(type));
    if(p != types.end())
        return p->second;
    problem(Diagnostic(Diagnostic::Severity::error, "Can't find type definition for '" + std::string(type) + "'", loc));

    return nullptr;
}

void RezWorld::addResource(ResSpec spec, CompoundExprPtr body, yy::location loc)
{
    if(verboseFlag)
        std::cout << "RESOURCE " << spec.type() << "(" << spec.id() << ", " << "\"" << spec.name() << "\"" << spec.attr() << ")" << std::endl;
    TypeDefinitionPtr def = getTypeDefinition(spec.type(), spec.id(), loc);
    if(!def)
        return;
    ResourceCompiler compiler(*this, def, body, verboseFlag);
    compiler.compile();

    resources.addResource(Resource(spec.type(), spec.id(), compiler.resourceData(), spec.name(), spec.attr()));
}

void RezWorld::addData(ResSpec spec, const std::string &data, yy::location loc)
{
    if(verboseFlag)
        std::cout << "DATA " << spec.type() << "(" << spec.id() << ", " << "\"" << spec.name() << "\"" << spec.attr() << ")" << std::endl;
    resources.addResource(Resource(spec.type(), spec.id(), data, spec.name(), spec.attr()));
}

void RezWorld::problem(Diagnostic d)
{
    hadErrors = true;
    std::cerr << d << std::endl;
}
