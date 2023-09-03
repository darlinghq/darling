#ifndef XTRACE_STRING
#define XTRACE_STRING

#include "base.h"
#include "memory.h"

#include <stdio.h>


XTRACE_DECLARATIONS_C_BEGIN

typedef struct xtrace_string{
    void* _unused;
} *xtrace_string_t;

xtrace_string_t xtrace_string_construct();
void xtrace_string_destruct(xtrace_string_t obj);

void xtrace_string_append_c_string(xtrace_string_t obj, const char* other);
void xtrace_string_append_xtrace_string(xtrace_string_t obj, xtrace_string_t other);
void xtrace_string_append_format_valist(xtrace_string_t obj, const char* format, va_list args);

__attribute__((format(printf, 2, 3)))
void xtrace_string_append_format(xtrace_string_t obj, const char* format, ...);

void xtrace_string_clear(xtrace_string_t obj);

const char* xtrace_string_c_str(xtrace_string_t obj);
size_t xtrace_string_size(xtrace_string_t obj);
size_t xtrace_string_capacity(xtrace_string_t obj);

XTRACE_DECLARATIONS_C_END


#ifdef XTRACE_CPP
#include <string>

namespace xtrace {
    class String {
    private:
        char* buffer;
        size_t buffer_size;
        size_t buffer_capacity;

    public:
        // Static methods
        static String* new_ptr();
        static void free_ptr(String* obj);
        static String* to_cxx_ptr(xtrace_string_t obj);
        static xtrace_string_t to_c_ptr(String* obj);

        // Constructor and destructor
        String();
        String(const char* other_buffer);
        String(const String* other_obj);
        ~String();
    
        // Methods
        void clear();
        void append(const char* other);
        void append(const String* other);
        void append_format(const char* format, va_list args);

        __attribute__((format(printf, 2, 3)))
        void append_format(const char* format, ...);

        // Getters
        const char* c_str() const;
        size_t size();
        size_t capacity();

    private:
        // Methods
        void constructor_internal();
        void destructor_internal();
        size_t increase_capacity_internal(size_t other_buffer_size);
    };
}
#endif

#endif // XTRACE_STRING