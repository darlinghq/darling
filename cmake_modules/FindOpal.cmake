
find_path(LIBOPAL_INCLUDE_DIR CoreGraphics/CoreGraphics.h
	PATHS
	/usr/include
	/usr/local/include
	/usr/include/GNUstep
	/usr/local/include/GNUstep
)

find_library(LIBOPAL_LIBRARY
	NAMES libopal.so
	PATHS /usr/lib /usr/local/lib
)

if (LIBOPAL_INCLUDE_DIR AND LIBOPAL_LIBRARY)
	set(LIBOPAL_FOUND TRUE)
	set(LIBOPAL_LIBRARIES ${LIBOPAL_LIBRARY})
else (LIBOPAL_INCLUDE_DIR AND LIBOPAL_LIBRARY)
	set(LIBOPAL_FOUND FALSE)
endif (LIBOPAL_INCLUDE_DIR AND LIBOPAL_LIBRARY)

