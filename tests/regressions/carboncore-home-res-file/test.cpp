// SPDX-License-Identifier: GPL-3.0-or-later
// Synthetic fixtures and host adapters; compile the real Resources.cpp and
// ResourcesImpl.cpp, and use the actual public Resources.h declaration.
// Contract: https://developer.apple.com/library/archive/documentation/mac/MoreToolbox/MoreToolbox-48.html
#include <CarbonCore/Resources.h>
#include <CarbonCore/MacMemory.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <unistd.h>

static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { \
    std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); std::exit(1); \
} } while (0)

static std::map<Handle, size_t> live;
static bool failNextHandle;
extern "C" OSErr PtrToHand(const void* bytes, Handle* out, long size)
{
    if (failNextHandle) { failNextHandle = false; *out = nullptr; return -108; }
    if (!out || size < 0 || (size && !bytes)) return paramErr;
    Handle h = static_cast<Handle>(std::malloc(sizeof(Ptr)));
    if (!h) return -108;
    *h = static_cast<Ptr>(std::malloc(size ? size_t(size) : 1));
    if (!*h) { std::free(h); return -108; }
    if (size) std::memcpy(*h, bytes, size_t(size));
    live.emplace(h, size_t(size));
    *out = h;
    return noErr;
}
extern "C" void DisposeHandle(Handle h)
{
    if (!h) return;
    if (live.erase(h) != 1) std::abort();
    std::free(*h);
    std::free(h);
}
extern "C" long GetHandleSize(Handle h)
{
    auto it = live.find(h);
    return it == live.end() ? -1 : long(it->second);
}
extern "C" void SetHandleSize(Handle, long) { std::abort(); }
extern "C" OSErr FSRefMakePath(const FSRef* ref, UInt8* path, UInt32 capacity)
{
    if (!ref || !path || std::strlen(ref->path) + 1 > capacity) return paramErr;
    std::memcpy(path, ref->path, std::strlen(ref->path) + 1);
    return noErr;
}
extern "C" OSErr FSGetDataForkName(HFSUniStr255* name) { name->length = 0; return noErr; }
extern "C" OSErr FSGetResourceForkName(HFSUniStr255*) { std::abort(); }
extern "C" OSErr FSPathMakeRef(const UInt8*, FSRef*, Boolean*) { std::abort(); }

static void put16(std::vector<uint8_t>& bytes, size_t offset, uint16_t value)
{
    bytes.at(offset) = uint8_t(value >> 8);
    bytes.at(offset + 1) = uint8_t(value);
}
static void put32(std::vector<uint8_t>& bytes, size_t offset, uint32_t value)
{
    put16(bytes, offset, uint16_t(value >> 16));
    put16(bytes, offset + 2, uint16_t(value));
}

// One resource per file, with the same type/id but distinct payloads. Ownership
// must be resolved from the handle, not from type/id or the current file.
static FSRef makeFile(char payload)
{
    std::vector<uint8_t> bytes(562, 0);
    put32(bytes, 0, 256); put32(bytes, 4, 512);
    put32(bytes, 8, 5); put32(bytes, 12, 50);
    put32(bytes, 256, 1); bytes.at(260) = uint8_t(payload);
    put16(bytes, 512 + 24, 28); put16(bytes, 512 + 26, 50);
    put16(bytes, 512 + 28, 0); // one resource type
    put32(bytes, 512 + 30, 0x54455354); // TEST
    put16(bytes, 512 + 34, 0); put16(bytes, 512 + 36, 10);
    put16(bytes, 512 + 38, 128); put16(bytes, 512 + 40, 0xffff);
    const char* tmp = std::getenv("TMPDIR");
    std::string pattern = std::string(tmp ? tmp : "/tmp") + "/home-res-file.XXXXXX";
    std::vector<char> path(pattern.begin(), pattern.end()); path.push_back(0);
    int fd = mkstemp(path.data()); CHECK(fd >= 0);
    size_t written = 0;
    while (written < bytes.size()) {
        ssize_t n = write(fd, bytes.data() + written, bytes.size() - written);
        CHECK(n > 0); written += size_t(n);
    }
    CHECK(close(fd) == 0);
    FSRef ref = {};
    CHECK(std::strlen(path.data()) < sizeof(ref.path));
    std::strcpy(ref.path, path.data());
    return ref;
}
static ResFileRefNum openFile(const FSRef& path)
{
    ResFileRefNum ref = kResFileNotOpened;
    CHECK(FSOpenResourceFile(&path, 0, nullptr, fsRdPerm, &ref) == noErr);
    CHECK(ref != kResFileNotOpened);
    return ref;
}
static Handle getResource(ResFileRefNum ref, char payload)
{
    UseResFile(ref); CHECK(ResError() == noErr);
    Handle h = Get1Resource(0x54455354, 128);
    CHECK(h && *h && **h == payload);
    return h;
}
static void home(Handle h, ResFileRefNum expected)
{
    ResFileRefNum current = CurResFile();
    ResFileRefNum actual = HomeResFile(h);
    OSErr error = ResError(); // capture before querying any other API
    CHECK(actual == expected);
    CHECK(error == (expected == kResFileNotOpened ? resNotFound : noErr));
    CHECK(CurResFile() == current);
}

int main()
{
    static_assert(resNotFound == -192 && kResFileNotOpened == -1, "public contract");
    CHECK(CurResFile() == kResFileNotOpened);
    home(nullptr, kResFileNotOpened);
    FSRef pathA = makeFile('A'), pathB = makeFile('B');
    ResFileRefNum a = openFile(pathA), b = openFile(pathB);
    CHECK(a != b);
    Handle ha = getResource(a, 'A'), hb = getResource(b, 'B');
    CHECK(CurResFile() == b);
    home(ha, a); home(hb, b);
    UseResFile(a); home(hb, b); CHECK(CurResFile() == a);

    // A successful lookup must clear an earlier resource-manager error.
    UseResFile(32000); CHECK(ResError() == resFNotFound);
    home(hb, b);
    Ptr unknownData = nullptr;
    home(&unknownData, kResFileNotOpened);
    home(reinterpret_cast<Handle>(uintptr_t(1)), kResFileNotOpened);
    home(nullptr, kResFileNotOpened);
    Handle ordinary = nullptr;
    CHECK(PtrToHand("x", &ordinary, 1) == noErr);
    home(ordinary, kResFileNotOpened); DisposeHandle(ordinary);
    home(ha, a);

    // Ownership is attached to the master pointer, even with purged data.
    std::free(*hb); *hb = nullptr;
    home(hb, b);

    ReleaseResource(ha); // actual upstream lifecycle removes its handle-map entry
    home(ha, kResFileNotOpened);
    home(hb, b);
    ha = getResource(a, 'A'); home(ha, a);
    DetachResource(ha);
    home(ha, kResFileNotOpened);
    CHECK(*ha && **ha == 'A'); DisposeHandle(ha);

    // Failed eager handle creation can leave a null key in today's owner map.
    // A null API argument must still never identify that resource.
    FSRef pathC = makeFile('C');
    failNextHandle = true;
    ResFileRefNum c = openFile(pathC);
    CHECK(!failNextHandle);
    home(nullptr, kResFileNotOpened);
    CloseResFile(c); CHECK(ResError() == noErr);

    CloseResFile(b); CHECK(ResError() == noErr);
    CHECK(CurResFile() == a);
    home(hb, kResFileNotOpened); // the stale value is compared, never dereferenced
    ha = getResource(a, 'A'); home(ha, a);
    CloseResFile(a); CHECK(ResError() == noErr);
    CHECK(CurResFile() == kResFileNotOpened);
    home(ha, kResFileNotOpened);
    home(nullptr, kResFileNotOpened);
    CHECK(live.empty());
    CHECK(unlink(pathA.path) == 0); CHECK(unlink(pathB.path) == 0); CHECK(unlink(pathC.path) == 0);
    std::printf("PASS: %u checks; ownership, errors, current file, release/detach/close, null/unknown handles\n", checks);
    return 0;
}
