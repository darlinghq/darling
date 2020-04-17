
// BUILD:  $CC foo.c -bundle     -o $BUILD_DIR/test.bundle
// BUILD:  $CC main.c            -o $BUILD_DIR/image-list-uuid.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./image-list-uuid.exe


#include <stdio.h>
#include <dlfcn.h>
#include <mach-o/loader.h>
#include <mach-o/dyld_images.h>
#include <uuid/uuid.h>

extern const struct mach_header __dso_handle;

static void printUUIDs(const struct dyld_all_image_infos* infos)
{
    if ( infos->uuidArray != NULL ) {
        for (int i=0; i < infos->uuidArrayCount; ++i) {
            const struct dyld_uuid_info* nonCacheInfo = &infos->uuidArray[i];
            uuid_string_t uuidStr;
            uuid_unparse_upper(nonCacheInfo->imageUUID, uuidStr);
            printf("%p %s\n", nonCacheInfo->imageLoadAddress, uuidStr);
          }
    }
}

int main()
{
    printf("[BEGIN] image_infos-uuids\n");

    // NOTE: dyld_all_image_infos is private, but currently looked at by kernel during stackshots
    // This test is to validate that the data available to the kernel is correct

    task_dyld_info_data_t task_dyld_info;
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    if ( task_info(mach_task_self(), TASK_DYLD_INFO, (task_info_t)&task_dyld_info, &count) ) {
        printf("[FAIL]  image_infos-uuids: task_info() failed\n");
        return 0;
    }
    const struct dyld_all_image_infos* infos = (struct dyld_all_image_infos*)(uintptr_t)task_dyld_info.all_image_info_addr;


    if ( infos->uuidArray == NULL ) {
        printf("[FAIL]  infos->uuidArray == NULL\n");
        return 0;
    }
    if ( infos->uuidArrayCount < 2 ) {
        // expect to contain main executable and dyld
        printf("[FAIL]  infos->uuidArrayCount != 2 (is %lu)\n", infos->uuidArrayCount);
        return 0;
    }
    printUUIDs(infos);
    uint32_t initialCount = infos->uuidArrayCount;

    bool foundMain = false;
    bool foundDyld = false;
    for (int i=0; i < infos->uuidArrayCount; ++i) {
        const struct dyld_uuid_info* nonCacheInfo = &infos->uuidArray[i];
        if ( nonCacheInfo->imageLoadAddress == &__dso_handle )
            foundMain = true;
        if ( nonCacheInfo->imageLoadAddress->filetype == MH_DYLINKER )
            foundDyld = true;
    }
    if ( !foundMain ) {
        printf("[FAIL]  image_infos-uuids uuid array does not contain main program\n");
        return 0;
    }
    if ( !foundDyld ) {
        printf("[FAIL]  image_infos-uuids uuid array does not contain dyld\n");
        return 0;
    }

    void* handle = dlopen(RUN_DIR "/test.bundle", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("[FAIL]  image_infos-uuids %s\n", dlerror());
        return 0;
    }
    printf("loaded test.bundle\n");

    // now expect UUID list to be three
    if ( infos->uuidArrayCount != initialCount+1 ) {
        // expect to contain main executable and dyld
        printf("[FAIL]  infos->uuidArrayCount was not incremented (is %lu)\n", infos->uuidArrayCount);
        return 0;
    }
    printUUIDs(infos);

    printf("[PASS]  image_infos-uuids\n");
	return 0;
}

