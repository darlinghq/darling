project(LDAP)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "2.4.0")

add_framework(LDAP
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/LDAP.c

    DEPENDENCIES
        system
)
