#include "ResourceFork.h"
#include "BinaryIO.h"

#include <iostream>
#include <map>
#include <vector>

void Resources::addResources(const Resources& res)
{
    for(auto& rr : res.resources)
        addResource(rr.second);
}

void Resources::writeFork(std::ostream& out) const
{
    std::streampos start = out.tellp();
    longword(out,0x100);
    longword(out,0);
    longword(out,0);
    longword(out,0);
    for(int i = 0; i < 0x100 - 16; i++)
        byte(out, 0);

    std::map< ResType, std::map<int, int> > resourceInfos;
    std::streampos datastart = out.tellp();
    for(auto& rr : resources)
    {
        const Resource& r = rr.second;
        const std::string& data = r.getData();
        unsigned offset = out.tellp() - datastart;
        offset = (r.getAttr() << 24) | (offset & 0xFFFFFF);
        resourceInfos[ r.getType() ][ r.getID() ] = (int)offset;
        longword(out, data.size());
        out << data;
    }
    std::streampos dataend = out.tellp();
//   while(out.tellp() % 0x100)
//      out.put(0);
    std::streampos resmap = out.tellp();
    for(int i = 0; i < 16+4+2+2; i++)
        byte(out, 0);
    word(out,16+4+2+2+2+2); // offset to resource type list
    std::streampos resnameOffset = out.tellp();
    word(out,0);
    std::streampos typelist = out.tellp();
    word(out,resourceInfos.size() - 1);
    for(std::map< ResType, std::map<int, int> >::iterator p = resourceInfos.begin();
            p != resourceInfos.end(); ++p)
    {
        if(p->second.size())
        {
            ostype(out,p->first);
            word(out,p->second.size()-1);
            word(out,0); // replaced later
        }
    }
    int typeIndex = 0;
    int nameOffset = 0;
    for(std::map< ResType, std::map<int, int> >::iterator p = resourceInfos.begin();
            p != resourceInfos.end(); ++p)
    {
        if(p->second.size())
        {
            std::streampos pos = out.tellp();
            out.seekp((int)typelist + 2 + 8 * typeIndex + 6);
            word(out, pos - typelist);
            out.seekp(pos);
            typeIndex++;

            for(std::map<int,int>::iterator q = p->second.begin(); q != p->second.end(); ++q)
            {
                std::string name = resources.find(ResRef(p->first, q->first))->second.getName();
                word(out,q->first);
                if(name.size() == 0)
                    word(out,-1);
                else
                {
                    word(out, nameOffset);
                    nameOffset += (name.size() > 255 ? 255 : name.size()) + 1;
                }
                longword(out,q->second);
                longword(out,0);
            }
        }
    }
    std::streampos resnames = out.tellp();
    out.seekp(resnameOffset);
    word(out, resnames - resmap);
    out.seekp(resnames);

    for(std::map< ResType, std::map<int, int> >::iterator p = resourceInfos.begin();
            p != resourceInfos.end(); ++p)
    {
        for(std::map<int,int>::iterator q = p->second.begin(); q != p->second.end(); ++q)
        {
            std::string name = resources.find(ResRef(p->first, q->first))->second.getName();
            if(name.size() > 0)
            {
                int sz = name.size() > 255 ? 255 : name.size();
                byte(out, sz);
                for(int i = 0; i < sz; i++)
                    byte(out, name[i]);
            }
        }
    }

    std::streampos end = out.tellp();
    out.seekp(start + std::streampos(4));
    longword(out, resmap - start);
    longword(out, dataend - start - std::streampos(0x100));
    longword(out, end - resmap);
    out.seekp(end);
}

Resources::Resources(std::istream &in)
{
    std::streampos start = in.tellg();
    int resdataOffset = longword(in);
    int resmapOffset = longword(in);

    in.seekg(start + std::streampos(resmapOffset + 16 + 4 + 2 + 2));
    int typeListOffset = word(in);
    int nameListOffset = word(in);
    int nTypes = (word(in) + 1) & 0xFFFF;

    for(int i = 0; i < nTypes; i++)
    {
        in.seekg(start + std::streampos(resmapOffset + typeListOffset + 2 + i * 8));
        std::string type = ostype(in);
        int nRes = (word(in) + 1) & 0xFFFF;
        int refListOffset = word(in);

        for(int j = 0; j < nRes; j++)
        {
            in.seekg(start + std::streampos(resmapOffset + typeListOffset + refListOffset + j * 12));
            int id = (short) word(in);
            int nameOffset = word(in);
            int attr = byte(in);
            int off1 = byte(in);
            int off2 = byte(in);
            int off3 = byte(in);
            int offset = (off1 << 16) | (off2 << 8) | off3;
            std::string name;
            if(nameOffset != 0xFFFF)
            {
                in.seekg(start + std::streampos(resmapOffset + nameListOffset + nameOffset));
                int nameLen = byte(in);
                char buf[256];
                in.read(buf, nameLen);
                name = std::string(buf, nameLen);
            }

            in.seekg(start + std::streampos(resdataOffset + offset));
            int size = longword(in);
            std::vector<char> tmp(size);
            in.read(tmp.data(), size);
            std::string data(tmp.data(), size);

            addResource(Resource(type, id, data, name, attr));
        }
    }
}
