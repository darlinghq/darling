project(Kerberos)

set(DYLIB_COMPAT_VERSION "5.0.0")
set(DYLIB_CURRENT_VERSION "6.0.0")

add_framework(Kerberos
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/Kerberos.c

    DEPENDENCIES
        system
)
