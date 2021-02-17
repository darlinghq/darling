project(JavaRuntimeSupport)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(JavaRuntimeSupport
	FAT
	CURRENT_VERSION
	PARENT "JavaVM"
	VERSION "A"

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
