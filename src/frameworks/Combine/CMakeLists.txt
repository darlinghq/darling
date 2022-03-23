project(Combine)

set(DYLIB_COMPAT_VERSION "0.0.0")
set(DYLIB_CURRENT_VERSION "134.102.0")

add_framework(Combine
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/Combine.m
        src/_TtC7Combine14AnyCancellable.m
        src/_TtC7Combine25ObservableObjectPublisher.m
        src/_TtCC7Combine25ObservableObjectPublisherP33_40FA7804DFFDD9097A0F4AE4A57029327Conduit.m
        src/_TtC7Combine9DebugHook.m

    DEPENDENCIES
        system
        objc
        Foundation
)
