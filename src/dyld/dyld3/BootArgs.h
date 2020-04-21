//
//  BootArgs.hpp
//  dyld
//
//  Created by Louis Gerbarg on 11/14/18.
//

#ifndef __DYLD_BOOTARGS_H__
#define __DYLD_BOOTARGS_H__

#include <cstdint>

#define VIS_HIDDEN __attribute__((visibility("hidden")))

namespace dyld3 {
#if BUILDING_DYLD
    struct VIS_HIDDEN BootArgs {
        static bool contains(const char* arg);
        static bool forceCustomerCache();
        static bool forceDyld2();
        static bool forceDyld3();
        static bool enableDyldTestMode();
        static bool enableCompactImageInfo();
        static void setFlags(uint64_t flags);
    private:
        static const uint64_t kForceCustomerCacheMask = 1<<0;
        static const uint64_t kDyldTestModeMask = 1<<1;
        static const uint64_t kForceDyld2CacheMask = 1<<15;
        static const uint64_t kForceDyld3CacheMask = 1<<16;
        static const uint64_t kEnableCompactImageInfoMask = 1<<17;
        //FIXME: Move this into __DATA_CONST once it is enabled for dyld
        static uint64_t _flags;
    };
#endif

} // namespace dyld3

#endif /* __DYLD_BOOTARGS_H__ */
