/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <CarbonCore/Resources.h>
#include <CarbonCore/MacMemory.h>
#include <CarbonCore/MacErrors.h>
#include <CoreServices/FileManager.h>
#include <mach-o/loader.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#if !defined(__i386__)
#error The witness must be a Darwin i386 executable.
#endif

static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { \
    fprintf(stderr, "FAIL line=%d check=%s\n", __LINE__, #x); exit(1); \
} } while (0)

static void image(const char* symbol, const void* address)
{
    Dl_info info;
    CHECK(dladdr(address, &info) != 0);
    const struct mach_header* header = (const struct mach_header*)info.dli_fbase;
    CHECK(header && header->magic == MH_MAGIC && header->cputype == 7);
    const struct load_command* command = (const struct load_command*)(header + 1);
    const unsigned char* uuid = NULL;
    for (uint32_t i = 0; i < header->ncmds; ++i) {
        if (command->cmd == LC_UUID) uuid = ((const struct uuid_command*)command)->uuid;
        command = (const struct load_command*)((const char*)command + command->cmdsize);
    }
    CHECK(uuid != NULL);
    printf("IMAGE symbol=%s path=%s uuid=", symbol, info.dli_fname);
    for (unsigned i = 0; i < 16; ++i) printf("%02x", uuid[i]);
    putchar('\n');
}

static void be16(unsigned char* bytes, unsigned offset, uint16_t n)
{
    bytes[offset] = (unsigned char)(n >> 8); bytes[offset + 1] = (unsigned char)n;
}
static void be32(unsigned char* bytes, unsigned offset, uint32_t n)
{
    be16(bytes, offset, (uint16_t)(n >> 16)); be16(bytes, offset + 2, (uint16_t)n);
}
static FSRef fixture(const char* path, char data)
{
    unsigned char bytes[562] = {0};
    be32(bytes, 0, 256); be32(bytes, 4, 512); be32(bytes, 8, 5); be32(bytes, 12, 50);
    be32(bytes, 256, 1); bytes[260] = (unsigned char)data;
    be16(bytes, 536, 28); be16(bytes, 538, 50);
    be16(bytes, 540, 0); be32(bytes, 542, 0x54455354);
    be16(bytes, 546, 0); be16(bytes, 548, 10);
    be16(bytes, 550, 128); be16(bytes, 552, 0xffff);
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    CHECK(fd >= 0);
    size_t offset = 0;
    while (offset < sizeof(bytes)) {
        ssize_t n = write(fd, bytes + offset, sizeof(bytes) - offset);
        CHECK(n > 0); offset += (size_t)n;
    }
    CHECK(close(fd) == 0);
    FSRef ref;
    OSErr error = FSPathMakeRef((const UInt8*)path, &ref, NULL);
    printf("FIXTURE path=%s FSPathMakeRef=%d\n", path, error);
    CHECK(error == noErr);
    return ref;
}
static ResFileRefNum open_file(const FSRef* path)
{
    ResFileRefNum ref = kResFileNotOpened;
    OSErr error = FSOpenResourceFile(path, 0, NULL, fsRdPerm, &ref);
    printf("OPEN error=%d ref=%d\n", error, ref);
    CHECK(error == noErr && ref != kResFileNotOpened);
    return ref;
}
static Handle resource(ResFileRefNum file, char expected)
{
    UseResFile(file); CHECK(ResError() == noErr);
    Handle h = Get1Resource(0x54455354, 128);
    CHECK(h && *h && **h == expected);
    return h;
}
static void home(const char* label, Handle handle, ResFileRefNum expected)
{
    ResFileRefNum before = CurResFile();
    ResFileRefNum actual = HomeResFile(handle);
    OSErr error = ResError();
    ResFileRefNum after = CurResFile();
    printf("CASE %s home=%d expected=%d error=%d current=%d/%d\n", label, actual, expected, error, before, after);
    CHECK(actual == expected);
    CHECK(error == (expected == kResFileNotOpened ? resNotFound : noErr));
    CHECK(after == before);
}

int main(void)
{
    setbuf(stdout, NULL);
    printf("HOME_RES_I386_BEGIN pointer_size=%u\n", (unsigned)sizeof(void*));
    CHECK(sizeof(void*) == 4);
    image("HomeResFile", (const void*)&HomeResFile);
    image("GetHandleSize", (const void*)&GetHandleSize);
    image("FSPathMakeRef", (const void*)&FSPathMakeRef);
    home("null-no-open-file", NULL, kResFileNotOpened);
    FSRef pa = fixture("/tmp/home-res-a.rsrc", 'A');
    FSRef pb = fixture("/tmp/home-res-b.rsrc", 'B');
    ResFileRefNum a = open_file(&pa), b = open_file(&pb);
    CHECK(a != b);
    Handle ha = resource(a, 'A'), hb = resource(b, 'B');
    home("other-open-file-a", ha, a);
    home("current-file-b", hb, b);
    UseResFile(a); home("other-open-file-b", hb, b);
    UseResFile(32767); CHECK(ResError() == resFNotFound);
    home("clear-prior-error", ha, a);
    Handle ordinary = NewHandle(1); CHECK(ordinary && *ordinary);
    home("ordinary-memory-handle", ordinary, kResFileNotOpened);
    DisposeHandle(ordinary);
    char* master = NULL;
    home("unknown-master", &master, kResFileNotOpened);
    home("invalid-address", (Handle)(uintptr_t)1, kResFileNotOpened);
    home("null-with-open-files", NULL, kResFileNotOpened);
    home("success-after-error", hb, b);
    ReleaseResource(ha);
    home("released-resource", ha, kResFileNotOpened);
    home("other-owner-still-open", hb, b);
    ha = resource(a, 'A'); home("reloaded-resource", ha, a);
    DetachResource(ha);
    home("detached-resource", ha, kResFileNotOpened);
    CHECK(*ha && **ha == 'A'); DisposeHandle(ha);
    CloseResFile(b); CHECK(ResError() == noErr && CurResFile() == a);
    home("closed-other-file", hb, kResFileNotOpened);
    ha = resource(a, 'A'); home("remaining-open-file", ha, a);
    CloseResFile(a); CHECK(ResError() == noErr && CurResFile() == kResFileNotOpened);
    home("closed-current-file", ha, kResFileNotOpened);
    home("null-after-close", NULL, kResFileNotOpened);
    printf("HOME_RES_I386_PASS checks=%u failures=0\n", checks);
    return 0;
}
