//
//  BootArgs.cpp
//  dyld
//
//  Created by Louis Gerbarg on 11/14/18.
//

#include <sys/types.h>
#include <sys/sysctl.h>
#include <TargetConditionals.h>

#include "Loading.h" // For internalInstall()
#include "BootArgs.h"

namespace dyld3 {
/*
* Checks to see if there are any args that impact dyld. These args
* can be set sevaral ways. These will only be honored on development
* and Apple Internal builds.
*/
bool BootArgs::contains(const char* arg)
{
    //FIXME: Use strnstr(). Unfortunately we are missing an imp in libc.a
#if TARGET_OS_SIMULATOR
    return false;
#else
    // don't check for boot-args on customer installs
    if ( !internalInstall() )
        return false;

    // get length of full boot-args string
    size_t len;
    if ( sysctlbyname("kern.bootargs", NULL, &len, NULL, 0) != 0 )
        return false;

    // get copy of boot-args string
    char bootArgsBuffer[len];
    if ( sysctlbyname("kern.bootargs", bootArgsBuffer, &len, NULL, 0) != 0 )
        return false;

    // return true if 'arg' is a sub-string of boot-args
    return (strstr(bootArgsBuffer, arg) != nullptr);
#endif
}

uint64_t BootArgs::_flags = 0;

bool BootArgs::forceCustomerCache() {
    return (_flags & kForceCustomerCacheMask);
}

bool BootArgs::forceDyld2() {
    // If both force dyld2 and dyld3 are set then use dyld3
    if (_flags & kForceDyld3CacheMask) { return false; }
    if (_flags & kForceDyld2CacheMask) { return true; }
    if (contains("force_dyld2=1")) { return true; }
    return false;
}

bool BootArgs::forceDyld3() {
    return ((_flags & kForceDyld3CacheMask) || contains("force_dyld3=1"));
}

bool BootArgs::enableDyldTestMode() {
    return (_flags & kDyldTestModeMask);
}

bool BootArgs::enableCompactImageInfo() {
    return (_flags & kEnableCompactImageInfoMask);
}

void BootArgs::setFlags(uint64_t flags) {
#if TARGET_IPHONE_SIMULATOR
    return;
#else
    // don't check for boot-args on customer installs
    if ( !internalInstall() )
        return;
    _flags = flags;
#endif
}
};
