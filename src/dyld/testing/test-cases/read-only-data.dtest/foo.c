#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <mach/vm_region.h>
#include <dlfcn.h>

const void* const funcs[] = { &malloc, &free, &strcmp, &printf };

bool isReadOnly(const void* addr)
{
    mach_port_t                     objName;
    vm_address_t                    testAddr = (vm_address_t)addr;
    vm_size_t                       regionSize;
    vm_region_basic_info_data_64_t  info;
    mach_msg_type_number_t          len = VM_REGION_BASIC_INFO_COUNT_64;
    kern_return_t err = vm_region_64(mach_task_self(), &testAddr, &regionSize, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&info, &len, &objName);

    if ( err == 0 ) {
        if ( info.protection & VM_PROT_WRITE )
            return false;
        return true;
    }
    return false;
}



#if BUNDLE
bool testBundle()
{
    return isReadOnly(&funcs[1]);
}
#else
bool testLib()
{
    return isReadOnly(&funcs[1]);
}
#endif

