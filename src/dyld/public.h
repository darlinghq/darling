#ifndef DYLD_PUBLIC_H
#define DYLD_PUBLIC_H
#include <stdint.h>
#include <mach-o/dyld_priv.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t _dyld_image_count(void);

const struct mach_header* _dyld_get_image_header(uint32_t image_index);

intptr_t _dyld_get_image_vmaddr_slide(uint32_t image_index);

const char* _dyld_get_image_name(uint32_t image_index);

void _dyld_register_func_for_add_image(void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide));

void _dyld_register_func_for_remove_image(void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide));

int32_t NSVersionOfRunTimeLibrary(const char* libraryName);

int32_t NSVersionOfLinkTimeLibrary(const char* libraryName);

int _NSGetExecutablePath(char* buf, uint32_t* bufsize);

void __assert_rtn(const char* func, const char* file, int line, const char* failedexpr);


const char* dyld_image_path_containing_address(const void* addr);
bool _dyld_find_unwind_sections(void* addr, struct dyld_unwind_sections* info);

#ifdef __cplusplus
}
#endif

#endif
