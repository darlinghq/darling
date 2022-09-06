project(Collaboration)

remove_sdk_framework(Collaboration)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "81.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(Collaboration
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Collaboration"
)

add_framework(Collaboration
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/Collaboration.m
        src/CBIdentityAuthority.m
        src/CBIdentityPicker.m
        src/CBIdentity.m
        src/CBUserIdentity.m
        src/CBGroupIdentity.m
        src/SidebarDataSource.m
        src/IdentityPickerInternal.m
        src/ListObject.m
        src/IdentityDataSource.m
        src/CBIdentityQuery.m

    DEPENDENCIES
        system
        objc
        Foundation
)
