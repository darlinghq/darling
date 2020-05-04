#include "ResourceCompiler.h"
#include <iostream>
#include "ResourceDefinitions.h"
#include "RezWorld.h"
#include "Diagnostic.h"

ResourceCompiler::ResourceCompiler(
        RezWorld& world, TypeDefinitionPtr type, CompoundExprPtr body, bool verboseFlag)
    : world(world),
      typeDefinition(type),
      body(body),
      currentField(nullptr)
{
    this->verboseFlag = verboseFlag;
}

BinaryOutput::BinaryOutput()
    : verboseFlag(false)
{
    reset(true);
}

void BinaryOutput::reset(bool prePass)
{
    currentOffset = 0;
    if(!prePass)
        prePassData = std::move(data);
    data.clear();
    this->prePass = prePass;
}

std::string BinaryOutput::resourceData()
{
    return std::string(data.begin(), data.end());
}

void BinaryOutput::write(int nBits, int value)
{
    if(verboseFlag)
        std::cout << "[" << nBits << " bits] = " << std::hex << value << std::dec << std::endl;

    unsigned mask = 1 << (nBits-1);

    for(int i = 0; i < nBits; i++)
    {
        bool bit = (value & mask) != 0;

        if(currentOffset % 8 == 0)
            data.push_back(bit ? 0x80 : 0);
        else if(bit)
            data.back() |= (0x80 >> (currentOffset % 8));
        ++currentOffset;

        mask >>= 1;
    }

    //currentOffset += nBits;
}

int BinaryOutput::peek(int bitPos, int size)
{
    unsigned bytePos = bitPos / 8;
    unsigned endBytePos = (bitPos + size - 1) / 8 + 1;

    unsigned bitPosInByte = bitPos % 8;
    unsigned outPos = 32 - size;

    unsigned val = 0;

    for(unsigned i = bytePos; i != endBytePos; ++i)
    {
        unsigned byte;
        if(i < data.size())
            byte = data[i];
        else if(i < prePassData.size())
            byte = prePassData[i];
        else
            byte = 0;

        unsigned read = byte << (bitPosInByte + 24);
        val |= (read >> outPos);

        outPos += 8 - bitPosInByte;

        bitPosInByte = 0;
    }

    return val;
}

ExprPtr ResourceCompiler::lookupIdentifier(std::string name, const Subscripts &sub)
{
    if(currentField)
    {
        if(ExprPtr val = currentField->lookupNamedValue(name))
        {
            return val;
        }
    }

    auto p = labelValues.find(std::make_pair(name, sub));
    if(p != labelValues.end())
        return p->second;

    //std::cerr << "ID lookup failed: " << name << std::endl;

    return nullptr;
}

void ResourceCompiler::defineLabel(const std::string &name)
{
    labelValues[std::make_pair(name,currentSubscripts)] = std::make_shared<IntExpr>(currentOffset, yy::location());
}

void ResourceCompiler::compile()
{
    if(verboseFlag) std::cout << "(first pass)\n";
    reset(true);
    typeDefinition->compile(body, this, true);
    if(verboseFlag) std::cout << "(second pass)\n";

    reset(false);
    typeDefinition->compile(body, this, false);
    if(verboseFlag) std::cout << "(done)\n";
}

int ResourceCompiler::getArrayCount(const std::string &name)
{
    Subscripts sub = currentSubscripts;
    for(;;)
    {
        auto p = arrayCounts.find(std::make_pair(name, sub));
        if(p != arrayCounts.end())
            return p->second;


        if(sub.empty())
            return 0; /* ### */
        sub.popSubscript();
    }
}

int ResourceCompiler::getArrayIndex(const std::string &arrayName)
{
    return curArrayIndices[arrayName];
}

void ResourceCompiler::problem(Diagnostic d)
{
    if(!prePass)
        world.problem(d);
}

void ResourceCompiler::beginArrayScope(std::string &arrayName, int index)
{
    if(arrayName != "")
    {
        curArrayIndices[arrayName] = index;
        int& count = arrayCounts[std::make_pair(arrayName, currentSubscripts)];
        if(count < index)
            count = index;
        arrayCounts[std::make_pair(arrayName, Subscripts())] = count;
        //std::cout << "count for " << arrayName << " is " << count << std::endl;
    }
    currentSubscripts.addSubscript(index);
}

Subscripts::Subscripts()
{
}

Subscripts::~Subscripts()
{
}

void Subscripts::addSubscript(int x)
{
    subscripts.push_back(x);
}

void Subscripts::popSubscript()
{
    subscripts.pop_back();
}

bool Subscripts::operator<(const Subscripts &other) const
{
    if(subscripts.size() < other.subscripts.size())
        return true;
    if(other.subscripts.size() < subscripts.size())
        return false;
    for(int i = 0, n = subscripts.size(); i < n; i++)
    {
        if(subscripts[i] < other.subscripts[i])
            return true;
        else if(subscripts[i] > other.subscripts[i])
            return false;
    }
    return false;
}
