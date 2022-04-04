project(Collaboration)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "81.0.0")

add_framework(Collaboration
    FAT
    CURRENT_VERSION
    VERSION "A"

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
