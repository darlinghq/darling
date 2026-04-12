project(PubSub)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
set(FRAMEWORK_VERSION "A")

# When generating the SDK headers we specifiy PubSub as public,
# as we want the headers to be available. But the framework will
# actually be private, with a public symlink to it.

remove_sdk_framework(PubSub
)

generate_sdk_framework(PubSub
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/PubSub"
)

add_framework(PubSub
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/PubSub.m
        src/PSEnclosureInternal.m
        src/PSEnclosure.m
        src/PSFeedSettingsInternal.m
        src/PSFeedSettings.m
        src/PSClientInternal.m
        src/PSClient.m
        src/PSContentInternal.m
        src/PSContent.m
        src/PSFeedInternal.m
        src/PSFeed.m
        src/PSEntryInternal.m
        src/PSEntry.m
        src/PSAuthorInternal.m
        src/PSAuthor.m
        src/PSUtils.m
        src/PSLink.m
        src/PSRegularExpression.m
        src/PSInternal.m
        src/PSSCGIProtocol.m
        src/PSFeedURLUtils.m
        src/PSSyncManager.m
        src/PSMessage.m
        src/PSMessageQueue.m
        src/PSTempDelegate.m
        src/PSDotMacOperation.m
        src/PSDotMacGetOperation.m
        src/PSDotMacPutOperation.m
        src/PSOperation.m
        src/PSGetOperation.m
        src/PSPutOperation.m
        src/PSAggregate.m

    DEPENDENCIES
        system
        objc
        Foundation
)
