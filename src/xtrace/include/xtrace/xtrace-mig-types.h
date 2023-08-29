#ifndef XTRACE_XTRACE_MIG_TYPES
#define XTRACE_XTRACE_MIG_TYPES

#include <mach/message.h>

#include "../base.h"

XTRACE_DECLARATIONS_C_BEGIN

struct xtrace_mig_callbacks {
    void (*add_raw_arg)(const char *format, ...) __attribute__((format(printf, 1, 2)));
    void (*add_num_arg)(unsigned long long n);
    void (*add_ptr_arg)(void *ptr);
    void (*add_string_arg)(const char *s);
    void (*add_bytes_arg)(const void *bytes, unsigned long cnt);
    void (*add_return_code_arg)(kern_return_t code);
    void (*add_port_arg)(mach_port_name_t port_name, mach_msg_type_name_t disposition);
    void (*add_ool_mem_arg)(const void *ptr, unsigned long size);
    void (*add_ool_ports_arg)(const void *ptr, unsigned long cnt, mach_msg_type_name_t disposition);
    void (*add_struct_arg)(const void *ptr, unsigned long cnt, unsigned long item_size);
    void (*add_array_arg)(const void *ptr, unsigned long cnt, unsigned long item_size);

    void (*set_return_code)(kern_return_t code);
};

typedef void xtrace_mig_routine(
    const mach_msg_header_t *Mess,
    boolean_t is_reply,
    const struct xtrace_mig_callbacks *callbacks
);

struct xtrace_mig_routine_desc {
    int present;
    int reply_present;
    const char *name;
    xtrace_mig_routine *routine;
};

struct xtrace_mig_subsystem {
    const char *name;
    mach_msg_id_t base;
    unsigned long routine_cnt;
    const struct xtrace_mig_routine_desc *routines;
};

XTRACE_DECLARATIONS_C_END

#endif // XTRACE_XTRACE_MIG_TYPES
