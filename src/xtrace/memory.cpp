#include <elfcalls.h>

extern struct elf_calls* _elfcalls;

extern "C"
void* xtrace_malloc(size_t size) {
    return _elfcalls->malloc(size);
}

extern "C"
void xtrace_free(void *ptr) {
    _elfcalls->free(ptr);
}

extern "C"
void* xtrace_realloc(void *ptr, size_t size) {
    return _elfcalls->realloc(ptr, size);
}
