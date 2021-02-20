project(JavaNativeFoundation)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(JavaNativeFoundation
	FAT
	CURRENT_VERSION
	PARENT "JavaVM"
	VERSION "A"

	SOURCES
		src/JavaNativeFoundation.m
		src/JNFException.m
		src/JNFTypeCoercer.m
		src/JNFInternalJavaClassToCoersionHolder.m
		src/JNFStringCoercion.m
		src/JNFNumberCoercion.m
		src/JNFDateCoercion.m
		src/JNFMapCoercion.m
		src/JNFListCoercion.m
		src/JNFSetCoercion.m
		src/JNFDefaultCoercions.m
		src/JNFJObjectWrapper.m
		src/JNFWeakJObjectWrapper.m
		src/JNFRunLoop.m
		src/JNFRunnableWrapper.m
		src/JNFRunnable.m

	DEPENDENCIES
		system
		objc
		Foundation
)
