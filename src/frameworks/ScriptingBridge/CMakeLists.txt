project(ScriptingBridge)

remove_sdk_framework(ScriptingBridge)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ScriptingBridge
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ScriptingBridge"
)

add_framework(ScriptingBridge
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/SBApplication.m
		src/SBElementArray.m
		src/SBObject.m

	DEPENDENCIES
		system
        objc
		Foundation
)
