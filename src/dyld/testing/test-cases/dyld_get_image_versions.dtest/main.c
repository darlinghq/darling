
// BUILD:  $CC main.c  -o $BUILD_DIR/dyld_get_image_versions.exe

// RUN:  ./dyld_get_image_versions.exe

#include <stdio.h>
#include <string.h>
#include <mach-o/dyld_priv.h>

extern struct mach_header __dso_handle;

int main()
{
    printf("[BEGIN] dyld_get_image_versions\n");

    // should succeed
    dyld_get_image_versions(&__dso_handle, ^(dyld_platform_t platform, uint32_t sdkVersion, uint32_t minOS) {
        printf("main binary: platform=%d, sdk=0x%08X, minOS-0x%08X\n", platform, sdkVersion, minOS);
    });

    uint8_t badFile[4096];
    struct mach_header* mh = (struct mach_header*)badFile;
    mh->magic       = MH_MAGIC_64;
    mh->ncmds       = 1;
    mh->filetype    = MH_DYLIB;
    mh->sizeofcmds  = 40;
    mh->flags       = 0;
    struct load_command* lc = (struct load_command*)&badFile[32];
    lc->cmd         = 1;
    lc->cmdsize     = 4000; // bad load command size

    // should detect the mh is bad and not crash or call the callback
    dyld_get_image_versions(mh, ^(dyld_platform_t platform, uint32_t sdkVersion, uint32_t minOS) {
        printf("bad binary: platform=%d, sdk=0x%08X, minOS-0x%08X\n", platform, sdkVersion, minOS);
    });


    printf("[PASS] dyld_get_image_versions\n");
	return 0;
}

