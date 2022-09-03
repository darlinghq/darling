project(PlugInKit)

remove_sdk_framework(PlugInKit
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(PK_C src/functions.c src/globals.c)
set(PK_OBJC
    src/PKBundle.m
    src/PKDaemonClient.m
    src/PKDiscoveryDriver.m
    src/PKDiscoveryLSWatcher.m
    src/PKHostDefaults.m
    src/PKHost.m
    src/PKHostPlugIn.m
    src/PKManager.m
    src/PKPlugInCore.m
    src/PKSandboxExtension.m
    src/PKServiceDefaults.m
    src/PKService.m
    src/PKServicePersonality.m
    src/PKXPCObject.m
)

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(PlugInKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/PlugInKit"
	PRIVATE
)

add_framework(PlugInKit
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		${PK_C}
		${PK_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
