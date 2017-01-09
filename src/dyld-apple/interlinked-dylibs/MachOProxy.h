//
//  DylibProxy.h
//  dyld
//
//  Created by Louis Gerbarg on 1/27/16.
//
//

#ifndef MachOProxy_h
#define MachOProxy_h

#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstdint>

#include <sys/stat.h>

struct MachOProxy {
	MachOProxy(const std::string& p, ino_t i, time_t t, uint32_t o, uint32_t s, bool r) :
            path(p), fatFileOffset(o), fileSize(s), lastModTime(t), inode(i), installNameOffsetInTEXT(0), rootlessProtected(r)   {
        bzero( &uuid, sizeof( uuid_t ) );
    }

    struct Segment {
        std::string name;
        uint64_t    size;
        uint64_t    sizeOfSections;
        uint32_t    diskSize;
        uint32_t    fileOffset;
        uint8_t     p2align;
        uint8_t     protection;
    };

    const std::string     path;
    const uint32_t        fatFileOffset;
    const uint32_t        fileSize;
    const time_t          lastModTime;
    const ino_t           inode;
    const bool            rootlessProtected;
    std::string           installName;
    std::set<std::string> installNameAliases;
    uint32_t              installNameOffsetInTEXT;
    std::set<std::string> dependencies;
    std::set<std::string> dependents;
    uuid_t                uuid;
    std::vector<Segment>  segments;

    const uint8_t* getBuffer();
    const bool     isDylib();
    const bool     isExecutable();
    bool addAlias(const std::string& alias);

    static std::map<std::string, MachOProxy*> findDylibInfo(const std::string& path, bool warnOnProblems=false, bool ignoreUncacheableDylibsInExecutables=false);

private:
    bool _dylib;
    bool _executable;

    template <typename P>
    std::string machoParser(bool ignoreUncacheableDylibsInExecutables);
};


#endif /* MachOProxy_h */
