#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/cdefs.h>
#include <sys/mman.h>
#include <mach/vm_page_size.h>
#include <os/overflow.h>

#ifdef __LP64__
#define GMALLOC_MAGIC 0xdeadbeefdeadbeef
#else
#define GMALLOC_MAGIC 0xdeadbeef
#endif

struct gmalloc_header {
    size_t magic;
    size_t size;
    void *mmap_base;
    size_t mmap_size;
    size_t magic_again;
};

static int gmalloc_protect_before;
static int gmalloc_fill_space;
static int gmalloc_allow_reads;
static int gmalloc_strict_size;
static int gmalloc_check_header = 1;

void __malloc_init(const char *apple[]) {
    (void) apple;

    if (getenv("MALLOC_PROTECT_BEFORE")) gmalloc_protect_before = 1;
    if (getenv("MALLOC_FILL_SPACE")) gmalloc_fill_space = 1;
    if (getenv("MALLOC_ALLOW_READS")) gmalloc_allow_reads = 1;
    if (getenv("MALLOC_STRICT_SIZE")) gmalloc_strict_size = 1;

    const char *check_header = getenv("MALLOC_CHECK_HEADER");
    if (check_header && (!strcmp(check_header, "0") || !strcmp(check_header, "NO"))) {
        gmalloc_check_header = 0;
    }
}

static inline size_t round_up(size_t size, size_t increment) {
    if ((size & (increment - 1)) == 0) {
        return size;
    }
    return (size | (increment - 1)) + 1;
}

static inline void *round_down(void *address, size_t align) {
    uintptr_t v = (uintptr_t) address;
    return (void *) (v & ~(align - 1));
}

static struct gmalloc_header *header_for_ptr(void *ptr) {
    return (struct gmalloc_header *) ((char *) ptr - sizeof(struct gmalloc_header));
}

static void *do_alloc(size_t payload_size, size_t align) {
    // Decide how much memory we're going to allocate,
    // not counting the guard page.
    size_t accessible_size = round_up(payload_size, align);
    if (gmalloc_protect_before) {
        // In protect-before mode, the header will reside
        // on the guard page, so need to add it here.
    } else {
        accessible_size += sizeof(struct gmalloc_header);
    }
    accessible_size = round_up(accessible_size, vm_page_size);

    void *mmap_base = mmap(
        NULL, accessible_size + vm_page_size,
        PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON,
        -1, 0
    );
    if (mmap_base == MAP_FAILED) {
        return NULL;
    }

    void *guard = gmalloc_protect_before ? mmap_base : mmap_base + accessible_size;
    void *accessible = gmalloc_protect_before ? mmap_base + vm_page_size : mmap_base;
    void *payload = gmalloc_protect_before ? accessible : accessible + accessible_size - payload_size;
    payload = round_down(payload, align);

    if (gmalloc_fill_space) {
        memset(accessible, 0x55, accessible_size);
    }

    struct gmalloc_header *header = header_for_ptr(payload);
    header->magic = header->magic_again = GMALLOC_MAGIC;
    header->size = payload_size;
    header->mmap_base = mmap_base;
    header->mmap_size = accessible_size + vm_page_size;

    int guard_page_prot = gmalloc_allow_reads ? PROT_READ : PROT_NONE;
    int rc = mprotect(guard, vm_page_size, guard_page_prot);
    if (rc < 0) {
        int saved_errno = errno;
        munmap(mmap_base, accessible_size + vm_page_size);
        errno = saved_errno;
        return NULL;
    }

    return payload;
}

void *aligned_alloc(size_t align, size_t size) {
    void *ptr = do_alloc(size, align);

    if (ptr == NULL) {
        int saved_errno = errno;
        const char msg[] = "Failed to alloc: ";
        write(2, msg, sizeof(msg) - 1);
        // Do not use strerror(), because it allocates.
        extern const char *const sys_errlist[];
        write(2, sys_errlist[saved_errno], strlen(sys_errlist[saved_errno]));
        write(2, "\n", 1);

        errno = saved_errno;
        return NULL;
    }

    return ptr;
}

void *malloc(size_t size) {
    return aligned_alloc(gmalloc_strict_size ? 1 : 16, size);
}

int posix_memalign(void **ptr, size_t align, size_t size) {
    *ptr = aligned_alloc(align, size);
    if (*ptr == NULL) {
        return ENOMEM;
    }
    return 0;
}

void *calloc(size_t num, size_t size) {
    void *ptr = malloc(num * size);
    memset(ptr, 0, num * size);
    return ptr;
}

static void unlock_and_check_header(const struct gmalloc_header *header) {
    if (gmalloc_protect_before && !gmalloc_allow_reads) {
        mprotect(round_down((void *) header, vm_page_size), vm_page_size, PROT_READ);
    }

    if (!gmalloc_check_header) {
        return;
    }

    if (header->magic != GMALLOC_MAGIC || header->magic_again != GMALLOC_MAGIC) {
        abort();
    }
}

void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    struct gmalloc_header *header = header_for_ptr(ptr);
    unlock_and_check_header(header);

    munmap(header->mmap_base, header->mmap_size);
}

void *realloc(void *old_ptr, size_t new_size) {
    size_t size_to_copy;
    if (old_ptr == NULL) {
        size_to_copy = 0;
    } else {
        struct gmalloc_header *header = header_for_ptr(old_ptr);
        unlock_and_check_header(header);
        size_to_copy = header->size;
    }
    if (size_to_copy > new_size) {
        size_to_copy = new_size;
    }

    void *new_ptr = malloc(new_size);
    if (new_ptr == NULL) {
        return NULL;
    }
    memcpy(new_ptr, old_ptr, size_to_copy);
    free(old_ptr);

    return new_ptr;
}

void *reallocarray(void *old_ptr, size_t num, size_t size) __DARWIN_EXTSN(reallocarray);
void *reallocarray(void *old_ptr, size_t num, size_t size) {
    size_t new_size;
    if (os_mul_overflow(num, size, &new_size)) {
        errno = ENOMEM;
        return NULL;
    }
    return realloc(old_ptr, new_size);
}

void *reallocarrayf(void *old_ptr, size_t num, size_t size) __DARWIN_EXTSN(reallocarrayf);
void *reallocarrayf(void *old_ptr, size_t num, size_t size) {
    size_t new_size;
    if (os_mul_overflow(num, size, &new_size)) {
        errno = ENOMEM;
        return NULL;
    }
    void *new_ptr = realloc(old_ptr, new_size);
    if (new_ptr == NULL) {
        free(old_ptr);
    }
    return new_ptr;
}

// Zone versions
typedef void malloc_zone_t;
void *malloc_zone_malloc(malloc_zone_t *zone, size_t size) {
    (void) zone;
    return malloc(size);
}

void *malloc_zone_calloc(malloc_zone_t *zone, size_t num, size_t size) {
    (void) zone;
    return calloc(num, size);
}

void *malloc_zone_realloc(malloc_zone_t *zone, void *ptr, size_t size) {
    (void) zone;
    return realloc(ptr, size);
}

void *malloc_zone_memalign(malloc_zone_t *zone, size_t align, size_t size) {
    (void) zone;
    void *ptr;
    posix_memalign(&ptr, align, size);
    return ptr;
}

void malloc_zone_free(malloc_zone_t *zone, void *ptr) {
    (void) zone;
    free(ptr);
}
