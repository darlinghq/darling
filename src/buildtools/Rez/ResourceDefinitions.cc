#include "ResourceDefinitions.h"
#include <ostream>
#include <cassert>

#include "ResourceCompiler.h"
#include "Diagnostic.h"


std::ostream &operator<<(std::ostream &out, TypeSpec ts)
{
    out << ts.getType();
    if(ts.hasID())
        out << " (" << ts.getID() << ")";
    return out;
}


FieldList::~FieldList()
{

}

void FieldList::addField(FieldPtr field, yy::location loc)
{
    field->location = loc;
    fields.push_back(field);
}

void FieldList::addLabel(std::string name, yy::location loc)
{
    addField(std::make_shared<LabelField>(name), loc);
}

void FieldList::compile(ExprPtr expr, ResourceCompiler *compiler, bool prePass)
{
    CompoundExprPtr compound = std::dynamic_pointer_cast<CompoundExpr>(expr);
    assert(compound);

    int i = 0;
    for(FieldPtr f : fields)
    {
        if(f->needsValue())
        {
            if(i >= compound->size())
                compiler->problem(Diagnostic(Diagnostic::error,"not enough values specified", compound->location));
            else
                f->compile(compound->getItem(i++), compiler, prePass);
        }
        else
            f->compile(nullptr, compiler, prePass);
    }
    if(i < compound->size())
    {
        compiler->problem(Diagnostic(Diagnostic::error,"extra value specified",
                                     compound->getItem(i)->location));
    }
}




void SimpleField::addNamedValue(std::string n)
{
    if(lastNamedValue)
        addNamedValue(n, std::make_shared<BinaryExpr>(
            BinaryOp::PLUS, lastNamedValue, std::make_shared<IntExpr>(1, yy::location()), yy::location()));
    else
        addNamedValue(n, std::make_shared<IntExpr>(0, yy::location()));
}

void SimpleField::addNamedValue(std::string n, ExprPtr val)
{
    namedValues[n] = val;
    lastNamedValue = val;
}

ExprPtr SimpleField::lookupNamedValue(std::string n)
{
    auto p = namedValues.find(n);
    if(p != namedValues.end())
        return p->second;
    else
        return nullptr;
}

bool SimpleField::needsValue()
{
    return !value;
}

void SimpleField::compile(ExprPtr expr, ResourceCompiler *compiler, bool prePass)
{
    switch(type)
    {
        case Type::bitstring:
        case Type::boolean:
        case Type::byte:
        case Type::integer:
        case Type::longint:
            compileInt(expr, compiler, prePass);
            break;
        case Type::string:
        case Type::wstring:
        case Type::pstring:
        case Type::char_:
            compileString(expr, compiler, prePass);
            break;

        case Type::rect:
        case Type::point:
            compileCompound(expr, compiler, prePass);
            break;

    }
}

void SimpleField::compileString(ExprPtr expr, ResourceCompiler *compiler, bool prePass)
{
    std::string str;
    {
        ResourceCompiler::FieldScope scope(compiler, this);
        str = (value ? value : expr)->evaluateString(compiler);
    }

    if(arrayCount || type == Type::char_)
    {
        unsigned requestedSize = type == Type::char_ ? 1 : arrayCount->evaluateInt(compiler);
        if(requestedSize < str.size())
            str.erase(str.begin() + requestedSize, str.end());
        else if(requestedSize > str.size())
            str.insert(str.end(),requestedSize - str.size(), '\0');
    }

    int count = str.size();

    if(type == Type::pstring)
    {
        if(count > 255)
        {
            str.erase(str.begin() + 255, str.end());
            count = 255;
        }
        compiler->write(8, count);
    }
    else if(type == Type::wstring)
    {
        if(count > 65535)
        {
            str.erase(str.begin() + 65535, str.end());
            count = 65535;
        }
        compiler->write(16, count);
    }

    for(char c : str)
        compiler->write(8, c);
}

void SimpleField::compileInt(ExprPtr expr, ResourceCompiler *compiler, bool prePass)
{
    int bitSize = 0;

    switch(type)
    {
        case Type::bitstring:
            bitSize = arrayCount->evaluateInt(compiler);
            break;
        case Type::boolean:
            bitSize = 1;
            break;
        case Type::byte:
            bitSize = 8;
            break;
        case Type::integer:
            bitSize = 16;
            break;
        case Type::longint:
            bitSize = 32;
            break;
        default:
            assert(false);
    }

    int actualValue = 0;
    ResourceCompiler::FieldScope scope(compiler, this);
    actualValue = (value ? value : expr)->evaluateInt(compiler);

    compiler->write(bitSize, actualValue);
}

void SimpleField::compileCompound(ExprPtr expr, ResourceCompiler *compiler, bool prePass)
{
    ExprPtr val = value ? value : expr;
    if(IdentifierExprPtr id = std::dynamic_pointer_cast<IdentifierExpr>(val))
    {
        ResourceCompiler::FieldScope scope(compiler, this);
        val = id->lookup(compiler);
    }

    int count = 0;
    switch(type)
    {
        case Type::rect:
            count = 4;
            break;
        case Type::point:
            count = 2;
            break;
        default:
            assert(false);
    }

    CompoundExprPtr compound = std::dynamic_pointer_cast<CompoundExpr>(val);
    if(!compound || compound->size() != count)
    {
        expr->error(compiler, std::string("expected ") + (type == Type::rect ? "rect {t,l,b,r}." : "point {v,h}."));
        return;
    }
    assert(compound);
    assert(compound->size() == count);

    for(int i = 0; i < count; i++)
    {
        int x = compound->getItem(i)->evaluateInt(compiler);
        compiler->write(16, x);
    }
}


ArrayField::ArrayField(std::string name, ExprPtr count)
    : name(name), arrayCount(count)
{
}

void ArrayField::compile(ExprPtr expr, ResourceCompiler *compiler, bool prePass)
{
    CompoundExprPtr compound = std::dynamic_pointer_cast<CompoundExpr>(expr);
    assert(compound);


    int i = 0;
    int n = compound->size();

    int iterations = 0;
    while(i < n)
    {
        ++iterations;
        ResourceCompiler::ArrayScope scope(compiler, name, iterations);
        for(FieldPtr f : fields)
        {
            if(f->needsValue())
            {
                assert(i < n);
                f->compile(compound->getItem(i++), compiler, prePass);
            }
            else
                f->compile(nullptr, compiler, prePass);
        }
    }

    if(!prePass && arrayCount)
    {
        int expected = arrayCount->evaluateInt(compiler);
        assert(expected == iterations);
    }
}


LabelField::LabelField(std::string name)
    : name(name)
{
}

bool LabelField::needsValue()
{
    return false;
}

void LabelField::compile(ExprPtr expr, ResourceCompiler *compiler, bool prePass)
{
    compiler->defineLabel(name);
}


void SwitchField::addCase(const std::string name, FieldListPtr alternative)
{
    cases[name] = alternative;
}

void SwitchField::compile(ExprPtr expr, ResourceCompiler *compiler, bool prePass)
{
    CaseExprPtr caseExpr = std::dynamic_pointer_cast<CaseExpr>(expr);
    assert(caseExpr);

    FieldListPtr caseDefinition = cases[caseExpr->tag];
    assert(caseDefinition);

    caseDefinition->compile(caseExpr->expr, compiler, prePass);
}


FillAlignField::FillAlignField(FillAlignField::Type type, bool isAlign, ExprPtr count)
    : type(type), count(count), isAlign(isAlign)
{

}

bool FillAlignField::needsValue()
{
    return false;
}

void FillAlignField::compile(ExprPtr expr, ResourceCompiler *compiler, bool prePass)
{
    int bitSize;
    switch(type)
    {
        case Type::bit:        bitSize = 1; break;
        case Type::nibble:    bitSize = 4; break;
        case Type::byte:    bitSize = 8; break;
        case Type::word:    bitSize = 16; break;
        case Type::long_:    bitSize = 32; break;
    }

    int actualCount = 1;
    if(count)
        actualCount = count->evaluateInt(compiler);

    for(int i = 0; i < actualCount; i++)
    {
        int n;
        if(isAlign)
        {
            int mask = bitSize - 1;
            int pos = compiler->tell();
            n = ((pos + mask) & ~mask) - pos;
        }
        else
            n = bitSize;
        compiler->write(n, 0);
    }
}
