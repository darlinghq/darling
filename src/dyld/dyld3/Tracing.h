/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */


#ifndef Tracing_h
#define Tracing_h

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <uuid/uuid.h>
#include <mach-o/loader.h>
#include <System/sys/kdebug.h>
#include <System/sys/reason.h>

#ifdef DARLING
#define kdebug_trace_string(...) ((uint64_t)-1)
#endif


#define DBG_DYLD_INTERNAL_SUBCLASS              (7)
#define DBG_DYLD_API_SUBCLASS                   (8)
#define DBG_DYLD_DEBUGGING_SUBCLASS             (9)

#define DBG_DYLD_TIMING_STATIC_INITIALIZER      (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 0))
#define DBG_DYLD_TIMING_LAUNCH_EXECUTABLE       (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 1))
#define DBG_DYLD_TIMING_MAP_IMAGE               (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 2))
#define DBG_DYLD_TIMING_APPLY_FIXUPS            (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 3))
#define DBG_DYLD_TIMING_ATTACH_CODESIGNATURE    (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 4))
#define DBG_DYLD_TIMING_BUILD_CLOSURE           (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 5))
#define DBG_DYLD_TIMING_FUNC_FOR_ADD_IMAGE      (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 6))
#define DBG_DYLD_TIMING_FUNC_FOR_REMOVE_IMAGE   (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 7))
#define DBG_DYLD_TIMING_OBJC_INIT               (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 8))
#define DBG_DYLD_TIMING_OBJC_MAP                (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 9))
#define DBG_DYLD_TIMING_APPLY_INTERPOSING       (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 10))
#define DBG_DYLD_GDB_IMAGE_NOTIFIER             (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 11))
#define DBG_DYLD_REMOTE_IMAGE_NOTIFIER          (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 12))
#define DBG_DYLD_TIMING_BOOTSTRAP_START         (KDBG_CODE(DBG_DYLD, DBG_DYLD_INTERNAL_SUBCLASS, 13))

#define DBG_DYLD_TIMING_DLOPEN                  (KDBG_CODE(DBG_DYLD, DBG_DYLD_API_SUBCLASS, 0))
#define DBG_DYLD_TIMING_DLOPEN_PREFLIGHT        (KDBG_CODE(DBG_DYLD, DBG_DYLD_API_SUBCLASS, 1))
#define DBG_DYLD_TIMING_DLCLOSE                 (KDBG_CODE(DBG_DYLD, DBG_DYLD_API_SUBCLASS, 2))
#define DBG_DYLD_TIMING_DLSYM                   (KDBG_CODE(DBG_DYLD, DBG_DYLD_API_SUBCLASS, 3))
#define DBG_DYLD_TIMING_DLADDR                  (KDBG_CODE(DBG_DYLD, DBG_DYLD_API_SUBCLASS, 4))

#define DBG_DYLD_DEBUGGING_VM_REMAP             (KDBG_CODE(DBG_DYLD, DBG_DYLD_DEBUGGING_SUBCLASS, 0))
#define DBG_DYLD_DEBUGGING_VM_UNMAP             (KDBG_CODE(DBG_DYLD, DBG_DYLD_DEBUGGING_SUBCLASS, 1))
#define DBG_DYLD_DEBUGGING_MAP_LOOP             (KDBG_CODE(DBG_DYLD, DBG_DYLD_DEBUGGING_SUBCLASS, 2))
#define DBG_DYLD_DEBUGGING_MARK                 (KDBG_CODE(DBG_DYLD, DBG_DYLD_DEBUGGING_SUBCLASS, 3))


#define VIS_HIDDEN __attribute__((visibility("hidden")))

namespace dyld3 {

enum class DyldTimingBuildClosure : uint64_t {
    ClosureBuildFailure                     = 0,
    LaunchClosure_Built                     = 1,
    DlopenClosure_UsedSharedCacheDylib      = 2,
    DlopenClosure_UsedSharedCacheOther      = 3,
    DlopenClosure_NoLoad                    = 4,
    DlopenClosure_Built                     = 5
};

struct VIS_HIDDEN kt_arg {
    kt_arg(int value) : _value(value), _str(nullptr) {}
    kt_arg(uint64_t value) : _value(value), _str(nullptr) {}
    kt_arg(DyldTimingBuildClosure value) : _value((uint64_t)value), _str(nullptr) {}
    kt_arg(const char *value) : _value(0), _str(value) {}
    kt_arg(void *value) : _value((uint64_t)value), _str(nullptr) {}
    uint64_t value() const { return _value; }
private:
    void prepare(uint32_t code) {
        if (_str) {
            _value = kdebug_trace_string(code, 0, _str);
            if (_value == (uint64_t)-1) _value = 0;
        }
    }
    void destroy(uint32_t code) {
        if (_str && _value) {
            kdebug_trace_string(code, _value, nullptr);
        }
    }
    friend class ScopedTimer;
    friend uint64_t kdebug_trace_dyld_duration_start(uint32_t code, kt_arg data1, kt_arg data2, kt_arg data3);
    friend void kdebug_trace_dyld_duration_end(uint64_t pair_id, uint32_t code, kt_arg data4, kt_arg data5, kt_arg data6);
    friend void kdebug_trace_dyld_marker(uint32_t code, kt_arg data1, kt_arg data2, kt_arg data3, kt_arg data4);
    uint64_t _value;
    const char* _str;
};

class VIS_HIDDEN ScopedTimer {
public:
    ScopedTimer(uint32_t code, kt_arg data1, kt_arg data2, kt_arg data3)
        : code(code), data1(data1), data2(data2), data3(data3), data4(0), data5(0), data6(0) {
#if BUILDING_LIBDYLD || BUILDING_DYLD
        startTimer();
#endif
    }

    ~ScopedTimer() {
#if BUILDING_LIBDYLD || BUILDING_DYLD
        endTimer();
#endif
    }

    void setData4(kt_arg data) { data4 = data; }
    void setData5(kt_arg data) { data5 = data; }
    void setData6(kt_arg data) { data6 = data; }
private:
#if BUILDING_LIBDYLD || BUILDING_DYLD
    void startTimer();
    void endTimer();
#endif

    uint32_t code;
    kt_arg data1;
    kt_arg data2;
    kt_arg data3;
    kt_arg data4;
    kt_arg data5;
    kt_arg data6;
    uint64_t current_trace_id = 0;
};

VIS_HIDDEN
void kdebug_trace_dyld_image(const uint32_t code,
                       const char* path,
                       const uuid_t* uuid_bytes,
                       const fsobj_id_t fsobjid,
                       const fsid_t fsid,
                       const mach_header* load_addr);

VIS_HIDDEN
bool kdebug_trace_dyld_enabled(uint32_t code);

VIS_HIDDEN
void kdebug_trace_dyld_marker(uint32_t code, kt_arg data1, kt_arg data2, kt_arg data3, kt_arg data4);

VIS_HIDDEN
uint64_t kdebug_trace_dyld_duration_start(uint32_t code, kt_arg data1, kt_arg data2, kt_arg data3);

VIS_HIDDEN
void kdebug_trace_dyld_duration_end(uint64_t trace_id, uint32_t code, kt_arg data4, kt_arg data5, kt_arg data6);

VIS_HIDDEN
void syntheticBacktrace(const char *reason, bool enableExternally=false);

};
#endif /* Tracing_h */
