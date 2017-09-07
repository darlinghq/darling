project(ScriptingBridge)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(ScriptingBridge
	FAT
	CURRENT_VERSION
	VERSION "A"

	SOURCES
		src/SBApplication.m
		src/SBElementArray.m
		src/SBObject.m

	DEPENDENCIES
		system
        objc
		Foundation
)
