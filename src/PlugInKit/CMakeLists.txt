project(PlugInKit)

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

add_framework(PlugInKit
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION "A"

	SOURCES
		${PK_C}
		${PK_OBJC}

	DEPENDENCIES
		system
		objc
		Foundation
)
