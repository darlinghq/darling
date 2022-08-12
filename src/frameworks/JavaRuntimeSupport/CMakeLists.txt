project(JavaRuntimeSupport)

remove_sdk_framework(JavaRuntimeSupport)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(JavaRuntimeSupport
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/JavaRuntimeSupport"
)

add_framework(JavaRuntimeSupport
	FAT
	CURRENT_VERSION
	PARENT "JavaVM"
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/JavaRuntimeSupport.m
		src/JRSDrag.m
		src/JRSAppKitAWT.m
		src/JRSMenu.m
		src/_JRSUIRenderer.m
		src/JRSInputMethodController.m
		src/MIGThread.m
		src/JRSRenderServer.m
		src/JRSSymbolicator.m
		src/JRSSymbolicatorImpl.m
		src/JRSSecurity.m

	DEPENDENCIES
		system
		objc
		Foundation
)
