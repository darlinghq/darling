project(ConfigurationProfiles)

remove_sdk_framework(ConfigurationProfiles
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ConfigurationProfiles
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ConfigurationProfiles"
	PRIVATE
)

add_framework(ConfigurationProfiles
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ConfigurationProfiles.m
        src/CPProfile.m
        src/MCXPlugInKitManagementCache.m
        src/MCXPlugInKitManagement.m
        src/CPProfileManager.m
        src/CPProfileMissingFieldHelper.m
        src/CPProfileAdditionalWarningsHelper.m
        src/CPMCXTools.m
        src/CPProvisioningProfile.m
        src/CPProvisioningManager.m
        src/CPProfileDomainServiceConnectionHandler.m
        src/CPProfileDomainService.m
        src/CPIdentificationPayloads.m
        src/CPCompositorWatcher.m
        src/MCTeslaConfiguration.m
	constants.m

    DEPENDENCIES
        system
        objc
        Foundation
)
