#include <mach/mach.h>

#include "darwintest.h"

T_GLOBAL_META(T_META_NAMESPACE("dyld"));

extern char** environ;

#if __x86_64__
cpu_type_t otherArch[] = { CPU_TYPE_I386 };
#elif __i386__
cpu_type_t otherArch[] = { CPU_TYPE_X86_64 };
#elif __arm64__
cpu_type_t otherArch[] = { CPU_TYPE_ARM };
#elif __arm__
cpu_type_t otherArch[] = { CPU_TYPE_ARM64 };
#endif

#define T_DECL_DYLD(name, description, ...)                                                           \
    static void dyld_test_ ## name(void);                                                           \
    T_DECL(name, description, ## __VA_ARGS__) {                                                     \
        dyld_test_ ## name();                                                                       \
    }                                                                                               \
    static void dyld_test_ ## name(void)

/* Since the test runner manually invokes us in both dyld2 and dyld3 mode we do not need this yet
 
 T_DECL(name ## _dyld3, description, T_META_ENVVAR("DYLD_USE_CLOSURES=1"), ## __VA_ARGS__) {     \
 dyld_test_ ## name();                                                                       \
 }                                                                                               \
 
 */


#define T_DLOPEN_EXPECT_NOTNULL(path, mode)                                                             \
({                                                                                                      \
    void* handle = dlopen(path, mode);                                                            \
    T_QUIET; T_ASSERT_NOTNULL(handle, "Image \"%s\" failed to load with error: %s", path, dlerror());   \
    T_QUIET; T_ASSERT_NULL(dlerror(), "dlerror() should be null after successfull dloepn()");           \
    handle;                                                                                             \
})

#define T_DLSYM_EXPECT_NOTNULL(handle, symbol)                              \
({                                                                          \
    const void* sym = dlsym((void *)handle, symbol);                        \
    T_QUIET; T_ASSERT_NOTNULL(sym, "dlsym(%s) should not be null", symbol); \
    sym;                                                                    \
})

#define T_DLCLOSE_EXPECT_NULL(handle)                                                   \
({                                                                                      \
    int result = dlclose((void *)handle);                                               \
    T_QUIET; T_ASSERT_EQ_INT(result, 0, "dlclose() failed with error: %s", dlerror());  \
    result;                                                                             \
})

#define T_POSIXSPAWN_ASSERT(launchSuspended, launchOtherArch, program)                                                  \
({                                                                                                                      \
    pid_t result = 0;                                                                                                   \
    posix_spawnattr_t attr = 0;                                                                                         \
    T_QUIET; T_ASSERT_EQ_INT(posix_spawnattr_init(&attr), 0, "dyld_process_info posix_spawnattr_init() failed");        \
    if ( launchSuspended ) {                                                                                            \
        int result = posix_spawnattr_setflags(&attr, POSIX_SPAWN_START_SUSPENDED);                                      \
        T_QUIET; T_ASSERT_EQ_INT(result, 0, "posix_spawnattr_setflags() failed");                                       \
    }                                                                                                                   \
    if ( launchOtherArch ) {                                                                                            \
        size_t copied;                                                                                                  \
        int result = posix_spawnattr_setbinpref_np(&attr, 1, otherArch, &copied);                                       \
        T_QUIET; T_ASSERT_EQ_INT(result, 0, "posix_spawnattr_setbinpref_np(), &copied) failed");                        \
    }                                                                                                                   \
    const char* argv[] = { program, NULL };                                                                             \
    int psResult = posix_spawn(&result, program, NULL, &attr, (char**)argv, environ);                                   \
    T_QUIET; T_ASSERT_EQ_INT(psResult, 0, "dyld_process_info posix_spawn(%s) failed, err=%d\n", program, psResult);     \
    T_QUIET; T_ASSERT_EQ_INT(posix_spawnattr_destroy(&attr), KERN_SUCCESS, "posix_spawnattr_destroy() failed");         \
    result;                                                                                                             \
})

#define T_TASK_FOR_PID_ASSERT(pid)                                                                          \
({                                                                                                          \
    task_t result;                                                                                          \
    T_QUIET; T_ASSERT_MACH_SUCCESS(task_for_pid(mach_task_self(), pid, &result), "task_for_pid() failed");  \
    result;                                                                                                 \
})

#pragma pack(4)
typedef struct exception_data {
    mach_msg_header_t Head;
    mach_msg_body_t msgh_body;
    mach_msg_port_descriptor_t thread;
    mach_msg_port_descriptor_t task;
    NDR_record_t NDR;
    exception_type_t exception;
    mach_msg_type_number_t codeCnt;
    __int64_t code[2];
} exception_data;
#pragma pack()

typedef bool(^exceptionValidator)(task_t task);

#define T_POSIXSPAWN_CRASH(program, validatorFunc, ...)                                                                 \
({                                                                                                                      \
    pid_t pid = 0;                                                                                                      \
    posix_spawnattr_t attr = 0;                                                                                         \
    mach_port_t exceptionPort = MACH_PORT_NULL;                                                                         \
    T_QUIET; T_ASSERT_EQ_INT(posix_spawnattr_init(&attr), 0, "dyld_process_info posix_spawnattr_init() failed");        \
    mach_port_options_t options = { .flags = MPO_CONTEXT_AS_GUARD | MPO_STRICT | MPO_INSERT_SEND_RIGHT, .mpl = { 1 }};  \
    T_QUIET; T_ASSERT_MACH_SUCCESS(mach_port_construct(mach_task_self(), &options, (mach_port_context_t)exceptionPort,  \
        &exceptionPort), "mach_port_construct() failed");                                                               \
    int epResult = posix_spawnattr_setexceptionports_np(&attr, EXC_MASK_CORPSE_NOTIFY, exceptionPort,                   \
        EXCEPTION_DEFAULT | MACH_EXCEPTION_CODES, 0);                                                                   \
    T_QUIET; T_ASSERT_EQ_INT(epResult, 0, "posix_spawnattr_setexceptionports_np() failed");                             \
    const char* argv[] = { program, NULL };                                                                             \
    int psResult = posix_spawn(&pid, program, NULL, &attr, (char**)argv, environ);                                      \
    T_QUIET; T_ASSERT_EQ_INT(psResult, 0, "dyld_process_info posix_spawn(%s) failed, err=%d\n", program, psResult);     \
    T_QUIET; T_ASSERT_EQ_INT(posix_spawnattr_destroy(&attr), KERN_SUCCESS, "posix_spawnattr_destroy() failed");         \
    uint8_t data[MACH_MSG_SIZE_RELIABLE];                                                                               \
    exception_data* request = (exception_data*)&data[0];                                                                \
    T_QUIET; T_ASSERT_MACH_SUCCESS(mach_msg(&request->Head, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0,                         \
        MACH_MSG_SIZE_RELIABLE, exceptionPort, 10000, MACH_PORT_NULL), "mach_msg() failed");                            \
    validatorFunc((task_t)request->task.name);                                                                          \
})
