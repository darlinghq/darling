
# This scripts looks for libobjc2 header files as distributed by the GNUstep Project
# GCC's ancient libobjc will NOT do.

find_path(LIBOBJC2_INCLUDE_DIR objc/objc-arc.h
	PATHS
	/usr/include
	/usr/local/include
)

find_library(LIBOBJC2_LIBRARY
	NAMES libobjc.so.4
	PATHS /usr/lib /usr/local/lib
)

if (LIBOBJC2_INCLUDE_DIR AND LIBOBJC2_LIBRARY)
	# try_compile(LIBOBJC2_RECENT # TODO
	set(LIBOBJC2_FOUND TRUE)
	set(LIBOBJC2_LIBRARIES ${LIBOBJC2_LIBRARY})
else (LIBOBJC2_INCLUDE_DIR AND LIBOBJC2_LIBRARY)
	set(LIBOBJC2_FOUND FALSE)
endif (LIBOBJC2_INCLUDE_DIR AND LIBOBJC2_LIBRARY)

