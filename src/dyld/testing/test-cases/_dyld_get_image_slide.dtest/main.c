
// BUILD:  $CC main.c            -o $BUILD_DIR/_dyld_get_image_slide-test.exe

// RUN:  ./_dyld_get_image_slide-test.exe


#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_priv.h>


int main()
{
    printf("[BEGIN] _dyld_get_image_slide-test\n");

    int count = _dyld_image_count();
    for (int i=0; i < count; ++i) {
        const struct mach_header* mh    = _dyld_get_image_header(i);
        const char*               name  = _dyld_get_image_name(i);
        intptr_t                  slide = _dyld_get_image_slide(mh);
        intptr_t                  vmaddrSlide = _dyld_get_image_vmaddr_slide(i);
        if ( slide != vmaddrSlide ) {
            printf("[FAIL] _dyld_get_image_slide-test: %lld != %lld in %s\n",
                   (uint64_t)slide, (uint64_t)vmaddrSlide, name);
            return 0;
        }
    }

    // Check that garbage values return 0
    uintptr_t notMagic = 0;
    intptr_t slide = _dyld_get_image_slide((const struct mach_header*)&notMagic);
    if (slide != 0) {
        printf("[FAIL] _dyld_get_image_slide-test: slide value %lld for bad magic\n",
               (uint64_t)slide);
    }

    intptr_t vmaddrSlide = _dyld_get_image_vmaddr_slide(count + 1);
    if (vmaddrSlide != 0) {
        printf("[FAIL] _dyld_get_image_slide-test: vmaddr slide value %lld for index %d\n",
               (uint64_t)vmaddrSlide, count + 1);
    }

    printf("[PASS] _dyld_get_image_slide-test\n");
	return 0;
}

