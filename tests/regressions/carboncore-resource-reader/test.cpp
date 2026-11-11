// SPDX-License-Identifier: GPL-3.0-or-later
// Reader contracts adapted from fr4nck/d4rling 8030c9d (GPL-3.0-or-later).
// Original synthetic fixtures; real ResourcesImpl.cpp, test-only platform adapters.
// Format: https://developer.apple.com/library/archive/documentation/mac/MoreToolbox/MoreToolbox-99.html
#include "ResourcesImpl.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <unistd.h>

static int passed = 0, failed = 0;
#define CHECK(x) do { if (x) ++passed; else { ++failed; std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); } } while (0)
static std::map<Handle, size_t> live;
static bool failNextHandle = false;
extern "C" OSErr PtrToHand(const void* data, Handle* out, long length) {
    if (!out || length < 0 || (length && !data)) return -50;
    if (failNextHandle) { failNextHandle=false; return -108; }
    auto handle = static_cast<Handle>(std::malloc(sizeof(Ptr)));
    if (!handle) return -108;
    *handle = static_cast<Ptr>(std::malloc(length ? size_t(length) : 1));
    if (!*handle) { std::free(handle); return -108; }
    if (length) std::memcpy(*handle, data, length);
    *out = handle;
    live[handle] = size_t(length);
    return noErr;
}
extern "C" void DisposeHandle(Handle h) {
    if (!h) return;
    if (live.erase(h)) { std::free(*h); std::free(h); }
}
extern "C" long GetHandleSize(Handle h) {
    auto it = live.find(h);
    return it == live.end() ? -1 : long(it->second);
}

static void put16(std::vector<uint8_t>& f, size_t at, uint16_t v) {
    if (at+2 > f.size()) throw std::runtime_error("put16");
    f[at] = uint8_t(v>>8); f[at+1] = uint8_t(v);
}
static void put32(std::vector<uint8_t>& f, size_t at, uint32_t v) {
    if (at+4 > f.size()) throw std::runtime_error("put32");
    f[at] = uint8_t(v>>24); f[at+1] = uint8_t(v>>16);
    f[at+2] = uint8_t(v>>8); f[at+3] = uint8_t(v);
}
static std::vector<uint8_t> makeFork(size_t nameLength=4, bool emptyMap=false) {
    const size_t d=256, m=512;
    const size_t n=emptyMap ? 30 : 50+1+nameLength;
    std::vector<uint8_t> f(m+n, 0);
    put32(f,0,uint32_t(d)); put32(f,4,uint32_t(m));
    put32(f,8,7); put32(f,12,uint32_t(n));
    put32(f,d,3); f[d+4]='A'; f[d+5]='B'; f[d+6]='C';
    put16(f,m+22,0x0123); put16(f,m+24,28);
    put16(f,m+26,emptyMap ? 30 : 50);
    if (emptyMap) { put16(f,m+28,0xffff); return f; }
    put16(f,m+28,0); // one type
    put32(f,m+30,0x53545220); // STR<space>
    put16(f,m+34,0); // one resource
    put16(f,m+36,10); // reference list relative to type-list count
    put16(f,m+38,128); put16(f,m+40,0); // resource id and name offset
    f[m+42]=0x20; // resource attributes
    f[m+43]=0; f[m+44]=0; f[m+45]=0; // resource data offset 0
    f[m+50]=uint8_t(nameLength);
    for (size_t i=0; i<nameLength; ++i) f[m+51+i] = char('a'+i%26);
    return f;
}

// Two resources of the same type verify reference-list offsets, independent
// data offsets, and the resource map's first-match id/name indexes.
static std::vector<uint8_t> makeTwo() {
    auto f=makeFork();
    f.resize(512+74, 0);
    put32(f,8,15); put32(f,12,74);
    put32(f,256+7,4); f[256+11]='W'; f[256+12]='X';
    f[256+13]='Y'; f[256+14]='Z';
    put16(f,512+34,1); // two resources, same 'STR ' type
    put16(f,512+26,62); // names now follow two reference records
    put16(f,512+50,129); put16(f,512+52,5);
    f[512+54]=0x08;
    f[512+55]=0; f[512+56]=0; f[512+57]=7;
    f[512+62]=4;
    std::memcpy(f.data()+512+63, "abcd", 4);
    f[512+67]=6;
    std::memcpy(f.data()+512+68, "second", 6);
    return f;
}

static std::string makeFile(const std::vector<uint8_t>& f) {
    const char* tmp = std::getenv("TMPDIR");
    if (!tmp) tmp = "/tmp";
    std::string pattern=std::string(tmp)+"/d4rl-rsrc-r1-XXXXXX";
    std::vector<char> name(pattern.begin(), pattern.end());
    name.push_back('\0');
    int fd = mkstemp(name.data());
    if (fd < 0) throw std::runtime_error("mkstemp");
    size_t written = 0;
    while (written < f.size()) {
        ssize_t n = ::write(fd, f.data()+written, f.size()-written);
        if (n <= 0) { close(fd); unlink(name.data()); throw std::runtime_error("write"); }
        written += size_t(n);
    }
    close(fd);
    return name.data();
}
static bool rejects(const std::vector<uint8_t>& bytes) {
    std::string path=makeFile(bytes);
    try { Resources resource(path.c_str(), 1, false); unlink(path.c_str()); return false; }
    catch (OSErr e) { unlink(path.c_str()); return e==eofErr; }
    catch (...) { unlink(path.c_str()); throw; }
}

template<class Check> static void withFork(const std::vector<uint8_t>& bytes, Check check) {
    std::string path=makeFile(bytes);
    try {
        Resources res(path.c_str(),1,false);
        unlink(path.c_str());
        check(res);
    } catch (...) { unlink(path.c_str()); throw; }
}

static std::vector<uint8_t> makeShiftedTypes() {
    auto original=makeFork();
    std::vector<uint8_t> f(512+67,0);
    std::copy_n(original.begin(),16,f.begin());
    std::copy_n(original.begin()+256,7,f.begin()+256);
    put32(f,12,67);
    put16(f,512+22,0x0123);
    put16(f,512+24,40); put16(f,512+26,62);
    put16(f,512+28,0xfffe); // Not the count: the type list begins at 40.
    put16(f,512+40,0);
    put32(f,512+42,0x53545220); put16(f,512+46,0);
    put16(f,512+48,10);
    put16(f,512+50,128); put16(f,512+52,0);
    f[512+54]=0x20;
    f[512+62]=4; std::memcpy(f.data()+512+63,"abcd",4);
    return f;
}

static std::vector<uint8_t> makeTwoTypes() {
    auto original=makeTwo();
    std::vector<uint8_t> f(512+82,0);
    std::copy_n(original.begin(),16,f.begin());
    std::copy_n(original.begin()+256,15,f.begin()+256);
    put32(f,12,82);
    put16(f,512+22,0x0123); put16(f,512+24,28); put16(f,512+26,70);
    put16(f,512+28,1);
    put32(f,512+30,0x53545220); put16(f,512+34,0); put16(f,512+36,18);
    put32(f,512+38,0x54455854); put16(f,512+42,0); put16(f,512+44,30);
    put16(f,512+46,128); put16(f,512+48,0); f[512+50]=0x20;
    put16(f,512+58,129); put16(f,512+60,5); f[512+62]=0x08;
    f[512+65]=7;
    f[512+70]=4; std::memcpy(f.data()+512+71,"abcd",4);
    f[512+75]=6; std::memcpy(f.data()+512+76,"second",6);
    return f;
}

static std::vector<uint8_t> makeUnaligned() {
    auto original=makeFork();
    std::vector<uint8_t> f(original.size()+1,0);
    std::copy_n(original.begin(),16,f.begin());
    std::copy_n(original.begin()+256,7,f.begin()+257);
    std::copy(original.begin()+512,original.end(),f.begin()+513);
    put32(f,0,257); put32(f,4,513);
    return f;
}

static std::vector<uint8_t> makeOffsetFFFF() {
    auto original=makeFork();
    const size_t map=66000, offset=0xffff;
    std::vector<uint8_t> f(map+55,0);
    std::copy_n(original.begin(),16,f.begin());
    std::copy(original.begin()+512,original.end(),f.begin()+map);
    put32(f,4,uint32_t(map)); put32(f,8,uint32_t(offset+7));
    put32(f,256+offset,3); std::memcpy(f.data()+256+offset+4,"ABC",3);
    f[map+43]=0; f[map+44]=0xff; f[map+45]=0xff;
    return f;
}

int main(int argc, char** argv) {
    try {
        const bool validOnly=argc==2 && std::strcmp(argv[1],"--valid-only")==0;
        const size_t baseline=live.size();
        auto fork=makeFork();
        withFork(fork,[](Resources& res) {
            CHECK(res.attributes()==0x0123);
            CHECK(res.data().size()==1);
            auto& group=res.data().at(0x53545220);
            CHECK(group.type==0x53545220);
            CHECK(group.resources.size()==1);
            auto* entry=group.resources.front().get();
            CHECK(entry->id==128);
            CHECK(entry->type==0x53545220);
            CHECK(entry->name=="abcd");
            CHECK(entry->attributes==0x20);
            CHECK(entry->dataBacking==std::vector<uint8_t>({'A','B','C'}));
            CHECK(group.idMap.at(128)==entry);
            CHECK(group.nameMap.count("abcd")==1);
            if (group.nameMap.count("abcd")) CHECK(group.nameMap.at("abcd")==entry);
            Handle h=entry->getHandle();
            CHECK(h && *h && GetHandleSize(h)==3);
            if (h && *h) CHECK(std::memcmp(*h,"ABC",3)==0);
            CHECK(res.dataByHandle(h)==entry);
        });
        CHECK(live.size()==baseline);
        withFork(makeTwo(),[](Resources& res) {
            auto& group=res.data().at(0x53545220);
            CHECK(group.resources.size()==2);
            CHECK(group.idMap.size()==2);
            CHECK(group.nameMap.size()==2);
            auto* second=group.idMap.at(129);
            CHECK(second->id==129 && second->type==0x53545220);
            CHECK(second->name=="second" && second->attributes==0x08);
            CHECK(second->dataBacking==std::vector<uint8_t>({'W','X','Y','Z'}));
            CHECK(group.nameMap.count("second")==1);
        });
        withFork(makeTwoTypes(),[](Resources& res) {
            CHECK(res.data().size()==2);
            auto* first=res.data().at(0x53545220).resources.front().get();
            auto* second=res.data().at(0x54455854).resources.front().get();
            CHECK(first->type==0x53545220 && second->type==0x54455854);
            CHECK(first->name=="abcd" && second->name=="second");
            CHECK(first->dataBacking==std::vector<uint8_t>({'A','B','C'}));
            CHECK(second->dataBacking==std::vector<uint8_t>({'W','X','Y','Z'}));
            CHECK(res.handleMap().size()==2);
        });
        withFork(makeShiftedTypes(),[](Resources& res) {
            CHECK(res.data().size()==1);
            auto* entry=res.data().at(0x53545220).resources.front().get();
            CHECK(entry->id==128 && entry->name=="abcd");
            CHECK(entry->dataBacking==std::vector<uint8_t>({'A','B','C'}));
        });
        withFork(makeUnaligned(),[](Resources& res) {
            auto* entry=res.data().at(0x53545220).resources.front().get();
            CHECK(entry->name=="abcd");
            CHECK(entry->dataBacking==std::vector<uint8_t>({'A','B','C'}));
        });
        withFork(makeOffsetFFFF(),[](Resources& res) {
            CHECK(res.data().at(0x53545220).resources.front()->dataBacking==
                  std::vector<uint8_t>({'A','B','C'}));
        });
        auto a=makeFork(); put16(a,512+38,0xffff);
        withFork(a,[](Resources& res) {
            CHECK(res.data().at(0x53545220).resources.front()->id==-1);
        });
        for (size_t length : {size_t(0),size_t(200),size_t(255)}) {
            withFork(makeFork(length),[length](Resources& res) {
                const auto& name=res.data().at(0x53545220).resources.front()->name;
                CHECK(name.size()==length);
                if (length) CHECK(name.back()==char('a'+(length-1)%26));
            });
        }
        a=makeFork(); put16(a,512+40,0xffff);
        withFork(a,[](Resources& res) {
            CHECK(res.data().at(0x53545220).resources.front()->name.empty());
        });
        a=makeFork(); put32(a,256,0); put32(a,8,4);
        withFork(a,[](Resources& res) {
            auto* entry=res.data().at(0x53545220).resources.front().get();
            CHECK(entry->dataBacking.empty());
            CHECK(entry->getHandle() && GetHandleSize(entry->getHandle())==0);
        });
        withFork(makeFork(4,true),[](Resources& res) {
            CHECK(res.data().empty()); CHECK(res.attributes()==0x0123);
        });
        // Preserve existing duplicate-index policy; this is not a macOS
        // compatibility claim for malformed maps. Discarded types must not
        // leave orphan handles or dangling entries in the handle index.
        a=makeTwoTypes(); put32(a,512+38,0x53545220);
        withFork(a,[](Resources& res) {
            CHECK(res.data().size()==1);
            CHECK(res.handleMap().size()==1);
            CHECK(live.size()==1);
            auto* entry=res.data().at(0x53545220).resources.front().get();
            CHECK(res.dataByHandle(entry->getHandle())==entry);
        });
        a=makeTwo(); put16(a,512+50,128);
        withFork(a,[](Resources& res) {
            const auto& group=res.data().at(0x53545220);
            CHECK(group.resources.size()==2 && group.idMap.size()==1);
        });
        CHECK(live.size()==baseline);
        if (!validOnly) {
            CHECK(rejects({}));
            CHECK(rejects(std::vector<uint8_t>(15,0)));
            a=fork; put32(a,4,0xffffffff); CHECK(rejects(a));
            a=fork; put32(a,12,0xffffffff); CHECK(rejects(a));
            a=fork; put32(a,0,0xffffffff); CHECK(rejects(a));
            a=fork; put32(a,8,0xffffffff); CHECK(rejects(a));
            a=fork; put32(a,0,0xfffffff0); put32(a,8,0x50); CHECK(rejects(a));
            a=fork; put32(a,4,0xfffffff0); put32(a,12,0x50); CHECK(rejects(a));
            a=fork; put32(a,12,29); CHECK(rejects(a));
            a=fork; put16(a,512+24,0xffff); CHECK(rejects(a));
            a=fork; put16(a,512+26,0xffff); CHECK(rejects(a));
            a=fork; put16(a,512+28,0xfffe); CHECK(rejects(a));
            a=fork; put16(a,512+36,0xffff); CHECK(rejects(a));
            a=fork; put16(a,512+34,0xffff); CHECK(rejects(a));
            a=fork; put16(a,512+40,0xfffe); CHECK(rejects(a));
            a=fork; put16(a,512+40,4); CHECK(rejects(a));
            a=fork; a[512+50]=250; CHECK(rejects(a));
            a=fork; a[512+43]=0xff; a[512+44]=0xff; a[512+45]=0xff; CHECK(rejects(a));
            a=fork; put32(a,256,0xffffffff); CHECK(rejects(a));
            a=fork; put32(a,256,8); CHECK(rejects(a)); // In the file, outside the data section.
            a=fork; put32(a,8,3); CHECK(rejects(a));
            a=makeTwo(); a[512+57]=100; CHECK(rejects(a));
            a=makeTwo(); put32(a,12,61); CHECK(rejects(a));
            a=makeTwoTypes(); put16(a,512+44,0xffff); CHECK(rejects(a));
            a=makeTwoTypes(); put16(a,512+60,0xffff-1); CHECK(rejects(a));
            a=makeShiftedTypes(); put16(a,512+40,1); CHECK(rejects(a));
            a=fork; a.resize(a.size()-1); CHECK(rejects(a));
            try { Resources missing("/d4rl-resource-reader-missing-ephemeral-xyz",1,false); CHECK(false); }
            catch (OSErr e) { CHECK(e==fnfErr); }
        }
        CHECK(live.size()==baseline);
        std::printf("CARBONCORE_RESOURCE_READER_HOST: %d PASS, %d FAIL\n",passed,failed);
        return failed?1:0;
    } catch (const std::exception& e) {
        std::fprintf(stderr,"FAIL unexpected exception: %s\n",e.what());
    } catch (OSErr e) {
        std::fprintf(stderr,"FAIL unexpected OSErr: %d\n",int(e));
    }
    return 1;
}
