#include "Expression.h"
#include "ResourceCompiler.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include "Diagnostic.h"

int Expression::evaluateInt(ResourceCompiler *ctx)
{
    error(ctx, "Expected an integer or integer expression here.");
    return 0;
}

std::string Expression::evaluateString(ResourceCompiler *ctx)
{
    error(ctx, "Expected a string or string expression here.");
    return "";
}

Expression::~Expression()
{
}

void Expression::error(ResourceCompiler *ctx, std::string err)
{
    ctx->problem(Diagnostic(Diagnostic::Severity::error, err, location));
}


StringExpr::~StringExpr()
{
}

std::string StringExpr::evaluateString(ResourceCompiler *ctx)
{
    return str;
}


IntExpr::~IntExpr()
{
}

int IntExpr::evaluateInt(ResourceCompiler *ctx)
{
    return val;
}


void CompoundExpr::addItem(ExprPtr item)
{
    items.push_back(item);
}

CompoundExpr::~CompoundExpr()
{
}


BinaryExpr::~BinaryExpr()
{
}

int BinaryExpr::evaluateInt(ResourceCompiler *ctx)
{
    switch(op)
    {
        case BinaryOp::XOR:
            return a->evaluateInt(ctx) ^ b->evaluateInt(ctx);
        case BinaryOp::OR:
            return a->evaluateInt(ctx) | b->evaluateInt(ctx);
        case BinaryOp::AND:
            return a->evaluateInt(ctx) & b->evaluateInt(ctx);
        case BinaryOp::SHIFTLEFT:
            return a->evaluateInt(ctx) << b->evaluateInt(ctx);
        case BinaryOp::SHIFTRIGHT:
            return a->evaluateInt(ctx) >> b->evaluateInt(ctx);
        case BinaryOp::EQUAL:
            return a->evaluateInt(ctx) == b->evaluateInt(ctx);
        case BinaryOp::NOTEQUAL:
            return a->evaluateInt(ctx) != b->evaluateInt(ctx);
        case BinaryOp::PLUS:
            return a->evaluateInt(ctx) + b->evaluateInt(ctx);
        case BinaryOp::MINUS:
            return a->evaluateInt(ctx) - b->evaluateInt(ctx);
        case BinaryOp::MULTIPLY:
            return a->evaluateInt(ctx) * b->evaluateInt(ctx);
        case BinaryOp::DIVIDE:
            return a->evaluateInt(ctx) / b->evaluateInt(ctx);
        default:
            error(ctx, "Expected an integer or integer expression here.");
            return 0;
    }
}

std::string BinaryExpr::evaluateString(ResourceCompiler *ctx)
{
    switch(op)
    {
        case BinaryOp::CONCAT:
            return a->evaluateString(ctx) + b->evaluateString(ctx);
        default:
            error(ctx, "Expected a string or string expression here.");
            return "";
    }
}


UnaryExpr::~UnaryExpr()
{
}

int UnaryExpr::evaluateInt(ResourceCompiler *ctx)
{
    switch(op)
    {
        case UnaryOp::MINUS:
            return -a->evaluateInt(ctx);
        case UnaryOp::COMPLEMENT:
            return ~a->evaluateInt(ctx);
        default:
            error(ctx, "Expected an integer or integer expression here.");
            return 0;
    }
}


IdentifierExpr::IdentifierExpr(std::string id, yy::location loc)
    : Expression(loc), id(id)
{
}

void IdentifierExpr::addArgument(ExprPtr e)
{
    arguments.push_back(e);
}

ExprPtr IdentifierExpr::lookup(ResourceCompiler *ctx)
{
    Subscripts sub;
    for(auto arg : arguments)
        sub.addSubscript(arg->evaluateInt(ctx));
    ExprPtr val = ctx->lookupIdentifier(id, sub);
    if(!val)
        error(ctx, "Identifier \"" + id + "\" is not defined.");
    return val;
}

int IdentifierExpr::evaluateInt(ResourceCompiler *ctx)
{
    if(ctx->isPrePass())
        return 0;
    if(ExprPtr e = lookup(ctx))
        return e->evaluateInt(ctx);
    else
        return 0;
}

std::string IdentifierExpr::evaluateString(ResourceCompiler *ctx)
{
    if(ExprPtr e = lookup(ctx))
        return e->evaluateString(ctx);
    else
        return "";
}


CaseExpr::CaseExpr(const std::string &tag, CompoundExprPtr expr, yy::location loc)
    : Expression(loc), tag(tag), expr(expr)
{
}


int CountOfExpr::evaluateInt(ResourceCompiler *ctx)
{
    assert(arg->arguments.size() == 0);
    return ctx->getArrayCount(arg->id);
}


int ArrayIndexExpr::evaluateInt(ResourceCompiler *ctx)
{
    assert(arg->arguments.size() == 0);
    return ctx->getArrayIndex(arg->id);
}


std::string ReadExpr::evaluateString(ResourceCompiler *ctx)
{
    std::string filename = arg->evaluateString(ctx);
    std::ifstream instream(filename);
    if(!instream)
    {
        ctx->problem(Diagnostic(Diagnostic::Severity::error, "could not $$read file " + filename, location));
    }
    return std::string(std::istreambuf_iterator<char>(instream.rdbuf()),
                       std::istreambuf_iterator<char>());
}


int UnimplementedExpr::evaluateInt(ResourceCompiler *ctx)
{
    std::cerr << msg << std::endl;
    return 0;
}

std::string UnimplementedExpr::evaluateString(ResourceCompiler *ctx)
{
    std::cerr << msg << std::endl;
    return "";
}


PeekExpr::PeekExpr(ExprPtr addr, ExprPtr offset, ExprPtr size, yy::location loc)
    : Expression(loc), addr(addr), offset(offset), size(size)
{
}

PeekExpr::PeekExpr(ExprPtr addr, int size, yy::location loc)
    : Expression(loc),
      addr(addr),
      offset(std::make_shared<IntExpr>(0,loc)),
      size(std::make_shared<IntExpr>(size,loc))
{
}

int PeekExpr::evaluateInt(ResourceCompiler *ctx)
{
    int p = addr->evaluateInt(ctx) + offset->evaluateInt(ctx);
    int s = size->evaluateInt(ctx);

    return ctx->peek(p, s);
}
