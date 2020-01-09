project(ConfigurationProfiles)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(ConfigurationProfiles
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION "A"

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
